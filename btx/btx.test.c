#include "btx/btx.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond) do { \
    if (cond) { g_pass++; } \
    else { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_fail++; } \
} while (0)

#define ASSERT_EQ_INT(a, b)  ASSERT((a) == (b))
#define ASSERT_EQ_MEM(a, b, n) ASSERT(memcmp((a), (b), (n)) == 0)

/* -------------------------------------------------------------------------
 * btx_validate
 * ---------------------------------------------------------------------- */

static void test_validate_ok(void) {
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\x00", 4));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xff\\x12\\xac", 12));
    ASSERT_EQ_INT(BTX_OK, btx_validate("  \\x01  \\x02  ", 14));
    ASSERT_EQ_INT(BTX_OK, btx_validate("// comment\n\\xab", 15));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xab // inline\n\\xcd", 19));
    ASSERT_EQ_INT(BTX_OK, btx_validate("", 0));
    /* case insensitivity */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xAB", 4));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xAb", 4));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xaB", 4));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xFF\\xAC", 8));
    /* tab whitespace */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\xab\t\\xcd", 9));
    /* \r\n line endings in comment */
    ASSERT_EQ_INT(BTX_OK, btx_validate("// comment\r\n\\xab", 16));
    /* comment-only and whitespace-only */
    ASSERT_EQ_INT(BTX_OK, btx_validate("// only a comment\n", 18));
    ASSERT_EQ_INT(BTX_OK, btx_validate("   \t  \n  ", 9));
}

static void test_validate_hex_errors(void) {
    ASSERT_EQ_INT(BTX_ERR_INVALID_HEX_SYNTAX, btx_validate("\\xGG", 4));
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN, btx_validate("hello", 5));
    /* truncated hex tokens */
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN, btx_validate("\\x", 2));
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN, btx_validate("\\x1", 3));
}

static void test_validate_bits_ok(void) {
    /* single complete byte */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b11110011", 10));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b1111'0011", 11));
    /* two partials forming one byte */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b1111'____ \\b____'0011", 23));
    /* three partials */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b11______ \\b__1100__ \\b______11", 32));
    /* multiple consecutive groups (each token is a complete byte) */
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b11110000 \\b00001111", 21));
    ASSERT_EQ_INT(BTX_OK, btx_validate("\\b1111'____ \\b____'1111 \\b00__'____ \\b__11'00__ \\b____'__11", 59));
}

static void test_validate_bits_errors(void) {
    /* non-contiguous within token */
    ASSERT_EQ_INT(BTX_ERR_BIT_NONCONTIGUOUS, btx_validate("\\b1__1'____", 11));
    ASSERT_EQ_INT(BTX_ERR_BIT_NONCONTIGUOUS, btx_validate("\\b____'1__1", 11));
    /* wrong order: skips bits */
    ASSERT_EQ_INT(BTX_ERR_BIT_ORDER, btx_validate("\\b1111'____ \\b______11", 22));
    /* overlap: both set bit 7 */
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP, btx_validate("\\b1_______ \\b1_______", 21));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP, btx_validate("\\b1_______ \\b0_______", 21));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP, btx_validate("\\b0_______ \\b1_______", 21));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP, btx_validate("\\b0_______ \\b0_______", 21));
    /* incomplete: hex token mid-group */
    ASSERT_EQ_INT(BTX_ERR_BIT_INCOMPLETE, btx_validate("\\b1111'____ \\xff", 16));
    /* incomplete at EOF */
    ASSERT_EQ_INT(BTX_ERR_BIT_INCOMPLETE, btx_validate("\\b1111'____", 11));
    /* malformed bit token */
    ASSERT_EQ_INT(BTX_ERR_INVALID_BIT_SYNTAX, btx_validate("\\b1111", 6));
    /* trailing apostrophe */
    ASSERT_EQ_INT(BTX_ERR_INVALID_BIT_SYNTAX, btx_validate("\\b11110000'", 11));
}

/* -------------------------------------------------------------------------
 * btx_decode
 * ---------------------------------------------------------------------- */

static void test_decode_hex(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\x12\\xac", 8, &out, &len));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0x12, out[0]);
    ASSERT_EQ_INT(0xac, out[1]);
    btx_free(out);
    /* boundary values */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\x00", 4, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0x00, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\xff", 4, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xff, out[0]);
    btx_free(out);
    /* case insensitivity */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\xAB", 4, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\xAb\\xFF", 8, &out, &len));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    ASSERT_EQ_INT(0xff, out[1]);
    btx_free(out);
}

static void test_decode_bits(void) {
    uint8_t *out = NULL; size_t len = 0;
    /* single complete token */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\b11110000", 10, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* single token with separator */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\b1111'0000", 11, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* two partials */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\b1111'____ \\b____'0000", 23, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* three partials */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\b11______ \\b__1100__ \\b______11", 32, &out, &len));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf3, out[0]);
    btx_free(out);
    /* two consecutive complete bytes from bit tokens */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\b11110000 \\b00001111", 21, &out, &len));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    ASSERT_EQ_INT(0x0f, out[1]);
    btx_free(out);
}

static void test_decode_errors(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN,      btx_decode("hello", 5, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_HEX_SYNTAX, btx_decode("\\xGG", 4, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_BIT_NONCONTIGUOUS,  btx_decode("\\b1__1'____", 11, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_BIT_ORDER,          btx_decode("\\b1111'____ \\b______11", 22, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP,        btx_decode("\\b1_______ \\b1_______", 21, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_BIT_INCOMPLETE,     btx_decode("\\b1111'____", 11, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_BIT_SYNTAX, btx_decode("\\b1111", 6, &out, &len));
}

static void test_decode_mixed(void) {
    uint8_t *out = NULL; size_t len = 0;
    /* \xab, then two partials forming 0xff, then \xcd */
    ASSERT_EQ_INT(BTX_OK, btx_decode("\\xab \\b1111'____ \\b____'1111 \\xcd", 33, &out, &len));
    ASSERT_EQ_INT(3, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    ASSERT_EQ_INT(0xff, out[1]);
    ASSERT_EQ_INT(0xcd, out[2]);
    btx_free(out);
}

static void test_decode_empty(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_decode("", 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_decode(NULL, 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
}

/* -------------------------------------------------------------------------
 * btx_encode
 * ---------------------------------------------------------------------- */

static void test_encode(void) {
    char *out = NULL; size_t len = 0;
    /* boundary values */
    uint8_t zero[] = {0x00};
    ASSERT_EQ_INT(BTX_OK, btx_encode(zero, 1, &out, &len));
    ASSERT_EQ_INT(4, (int)len);
    ASSERT_EQ_MEM("\\x00", out, 4);
    btx_free(out);
    uint8_t ff[] = {0xff};
    ASSERT_EQ_INT(BTX_OK, btx_encode(ff, 1, &out, &len));
    ASSERT_EQ_INT(4, (int)len);
    ASSERT_EQ_MEM("\\xff", out, 4);
    btx_free(out);
    /* multi-byte */
    uint8_t data[] = {0x12, 0xac};
    ASSERT_EQ_INT(BTX_OK, btx_encode(data, 2, &out, &len));
    ASSERT_EQ_INT(8, (int)len);
    ASSERT_EQ_MEM("\\x12\\xac", out, 8);
    btx_free(out);
}

static void test_encode_empty(void) {
    char *out = NULL; size_t len = 0;
    uint8_t empty[] = {0};
    ASSERT_EQ_INT(BTX_OK, btx_encode(empty, 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_encode(NULL, 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
}

static void test_validate_empty(void) {
    ASSERT_EQ_INT(BTX_OK, btx_validate("", 0));
    ASSERT_EQ_INT(BTX_OK, btx_validate(NULL, 0));
}

/* -------------------------------------------------------------------------
 * btx_strerror
 * ---------------------------------------------------------------------- */

static void test_strerror(void) {
    for (int i = 0; i < BTX_RESULT_MAX; i++) {
        const char *s = btx_strerror((btx_result_t)i);
        ASSERT(s != NULL);
        ASSERT(s[0] != '\0');
    }
    /* out-of-range returns non-NULL */
    ASSERT(btx_strerror(BTX_RESULT_MAX) != NULL);
}

/* -------------------------------------------------------------------------
 * Invalid argument guards
 * ---------------------------------------------------------------------- */

static void test_invalid_args(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_decode(NULL, 1, &out, &len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_decode("\\xab", 4, NULL, &len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_decode("\\xab", 4, &out, NULL));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_validate(NULL, 1));
    char *enc = NULL; size_t enc_len = 0;
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_encode(NULL, 1, &enc, &enc_len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_encode((const uint8_t *)"", 0, NULL, &enc_len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_encode((const uint8_t *)"", 0, &enc, NULL));
}

/* -------------------------------------------------------------------------
 * btx_free(NULL)
 * ---------------------------------------------------------------------- */

static void test_free_null(void) {
    btx_free(NULL); /* must not crash */
    ASSERT(1);
}

/* -------------------------------------------------------------------------
 * Round-trip
 * ---------------------------------------------------------------------- */

static void test_roundtrip(void) {
    uint8_t orig[] = {0x00, 0x01, 0x7f, 0x80, 0xff};
    char *enc = NULL; size_t enc_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_encode(orig, 5, &enc, &enc_len));

    uint8_t *dec = NULL; size_t dec_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_decode(enc, enc_len, &dec, &dec_len));
    ASSERT_EQ_INT(5, (int)dec_len);
    ASSERT_EQ_MEM(orig, dec, 5);

    btx_free(enc);
    btx_free(dec);
}

static void test_roundtrip_all_bytes(void) {
    uint8_t orig[256];
    for (int i = 0; i < 256; i++) orig[i] = (uint8_t)i;

    char *enc = NULL; size_t enc_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_encode(orig, 256, &enc, &enc_len));
    ASSERT_EQ_INT(1024, (int)enc_len);

    uint8_t *dec = NULL; size_t dec_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_decode(enc, enc_len, &dec, &dec_len));
    ASSERT_EQ_INT(256, (int)dec_len);
    ASSERT_EQ_MEM(orig, dec, 256);

    btx_free(enc);
    btx_free(dec);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void) {
    test_validate_ok();
    test_validate_hex_errors();
    test_validate_bits_ok();
    test_validate_bits_errors();
    test_decode_hex();
    test_decode_bits();
    test_decode_errors();
    test_decode_mixed();
    test_decode_empty();
    test_encode();
    test_encode_empty();
    test_validate_empty();
    test_strerror();
    test_invalid_args();
    test_free_null();
    test_roundtrip();
    test_roundtrip_all_bytes();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
