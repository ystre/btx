#include <libbtx/btx.h>

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
 * btx_to_bin
 * ---------------------------------------------------------------------- */

static void test_to_bin_hex(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\x12\\xac", 8, &out, &len, NULL));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0x12, out[0]);
    ASSERT_EQ_INT(0xac, out[1]);
    btx_free(out);
    /* boundary values */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\x00", 4, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0x00, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xff", 4, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xff, out[0]);
    btx_free(out);
    /* case insensitivity */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xAB", 4, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xAb\\xFF", 8, &out, &len, NULL));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    ASSERT_EQ_INT(0xff, out[1]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xaB", 4, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xFF\\xAC", 8, &out, &len, NULL));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0xff, out[0]);
    ASSERT_EQ_INT(0xac, out[1]);
    btx_free(out);
    /* tab whitespace */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xab\t\\xcd", 9, &out, &len, NULL));
    ASSERT_EQ_INT(2, (int)len);
    btx_free(out);
    /* \r\n line endings in comment */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("// comment\r\n\\xab", 16, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    btx_free(out);
    /* comment-only and whitespace-only */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("// only a comment\n", 18, &out, &len, NULL));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("   \t  \n  ", 9, &out, &len, NULL));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
}

static void test_to_bin_bits(void) {
    uint8_t *out = NULL; size_t len = 0;
    /* single complete token */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b11110000", 10, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* single token with separator */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b1111'0000", 11, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* two partials */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b1111'____ \\b____'0000", 23, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    btx_free(out);
    /* three partials */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b11______ \\b__1100__ \\b______11", 32, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf3, out[0]);
    btx_free(out);
    /* two consecutive complete bytes from bit tokens */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b11110000 \\b00001111", 21, &out, &len, NULL));
    ASSERT_EQ_INT(2, (int)len);
    ASSERT_EQ_INT(0xf0, out[0]);
    ASSERT_EQ_INT(0x0f, out[1]);
    btx_free(out);
    /* different bit pattern */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b11110011", 10, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf3, out[0]);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b1111'____ \\b____'0011", 23, &out, &len, NULL));
    ASSERT_EQ_INT(1, (int)len);
    ASSERT_EQ_INT(0xf3, out[0]);
    btx_free(out);
    /* 5-token multi-group */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\b1111'____ \\b____'1111 \\b00__'____ \\b__11'00__ \\b____'__11", 59, &out, &len, NULL));
    btx_free(out);
}

static void test_to_bin_errors(void) {
    uint8_t *out = NULL; size_t len = 0;
    btx_error_t err = { 0, 0 };
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN,      btx_to_bin("hello", 5, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    ASSERT_EQ_INT(BTX_ERR_INVALID_HEX_SYNTAX, btx_to_bin("\\xGG", 4, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    /* truncated hex tokens */
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN,      btx_to_bin("\\x", 2, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    ASSERT_EQ_INT(BTX_ERR_INVALID_TOKEN,      btx_to_bin("\\x1", 3, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    /* multiline: error on second line */
    ASSERT_EQ_INT(BTX_ERR_INVALID_HEX_SYNTAX, btx_to_bin("\\x00\n\\xGG", 9, &out, &len, &err));
    ASSERT_EQ_INT(2, err.line); ASSERT_EQ_INT(1, err.col);
    /* non-contiguous within token */
    ASSERT_EQ_INT(BTX_ERR_BIT_NONCONTIGUOUS,  btx_to_bin("\\b1__1'____", 11, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    ASSERT_EQ_INT(BTX_ERR_BIT_NONCONTIGUOUS,  btx_to_bin("\\b____'1__1", 11, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    /* wrong order */
    ASSERT_EQ_INT(BTX_ERR_BIT_ORDER,          btx_to_bin("\\b1111'____ \\b______11", 22, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(13, err.col);
    /* overlap */
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP,        btx_to_bin("\\b1_______ \\b1_______", 21, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(12, err.col);
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP,        btx_to_bin("\\b1_______ \\b0_______", 21, &out, &len, NULL));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP,        btx_to_bin("\\b0_______ \\b1_______", 21, &out, &len, NULL));
    ASSERT_EQ_INT(BTX_ERR_BIT_OVERLAP,        btx_to_bin("\\b0_______ \\b0_______", 21, &out, &len, NULL));
    /* incomplete: hex token mid-group */
    ASSERT_EQ_INT(BTX_ERR_BIT_INCOMPLETE,     btx_to_bin("\\b1111'____ \\xff", 16, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(13, err.col);
    /* incomplete at EOF */
    ASSERT_EQ_INT(BTX_ERR_BIT_INCOMPLETE,     btx_to_bin("\\b1111'____", 11, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(12, err.col);
    /* malformed bit token */
    ASSERT_EQ_INT(BTX_ERR_INVALID_BIT_SYNTAX, btx_to_bin("\\b1111", 6, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
    /* trailing apostrophe */
    ASSERT_EQ_INT(BTX_ERR_INVALID_BIT_SYNTAX, btx_to_bin("\\b11110000'", 11, &out, &len, &err));
    ASSERT_EQ_INT(1, err.line); ASSERT_EQ_INT(1, err.col);
}

static void test_to_bin_mixed(void) {
    uint8_t *out = NULL; size_t len = 0;
    /* \xab, then two partials forming 0xff, then \xcd */
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("\\xab \\b1111'____ \\b____'1111 \\xcd", 33, &out, &len, NULL));
    ASSERT_EQ_INT(3, (int)len);
    ASSERT_EQ_INT(0xab, out[0]);
    ASSERT_EQ_INT(0xff, out[1]);
    ASSERT_EQ_INT(0xcd, out[2]);
    btx_free(out);
}

static void test_to_bin_empty(void) {
    uint8_t *out = NULL; size_t len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_to_bin("", 0, &out, &len, NULL));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_to_bin(NULL, 0, &out, &len, NULL));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
}

/* -------------------------------------------------------------------------
 * btx_from_bin
 * ---------------------------------------------------------------------- */

static void test_from_bin(void) {
    char *out = NULL; size_t len = 0;
    /* boundary values */
    uint8_t zero[] = {0x00};
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(zero, 1, &out, &len));
    ASSERT_EQ_INT(4, (int)len);
    ASSERT_EQ_MEM("\\x00", out, 4);
    btx_free(out);
    uint8_t ff[] = {0xff};
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(ff, 1, &out, &len));
    ASSERT_EQ_INT(4, (int)len);
    ASSERT_EQ_MEM("\\xff", out, 4);
    btx_free(out);
    /* multi-byte */
    uint8_t data[] = {0x12, 0xac};
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(data, 2, &out, &len));
    ASSERT_EQ_INT(8, (int)len);
    ASSERT_EQ_MEM("\\x12\\xac", out, 8);
    btx_free(out);
}

static void test_from_bin_empty(void) {
    char *out = NULL; size_t len = 0;
    uint8_t empty[] = {0};
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(empty, 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(NULL, 0, &out, &len));
    ASSERT_EQ_INT(0, (int)len);
    btx_free(out);
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
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_to_bin(NULL, 1, &out, &len, NULL));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_to_bin("\\xab", 4, NULL, &len, NULL));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_to_bin("\\xab", 4, &out, NULL, NULL));
    char *enc = NULL; size_t enc_len = 0;
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_from_bin(NULL, 1, &enc, &enc_len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_from_bin((const uint8_t *)"", 0, NULL, &enc_len));
    ASSERT_EQ_INT(BTX_ERR_INVALID_ARG, btx_from_bin((const uint8_t *)"", 0, &enc, NULL));
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
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(orig, 5, &enc, &enc_len));

    uint8_t *dec = NULL; size_t dec_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_to_bin(enc, enc_len, &dec, &dec_len, NULL));
    ASSERT_EQ_INT(5, (int)dec_len);
    ASSERT_EQ_MEM(orig, dec, 5);

    btx_free(enc);
    btx_free(dec);
}

static void test_roundtrip_all_bytes(void) {
    uint8_t orig[256];
    for (int i = 0; i < 256; i++) orig[i] = (uint8_t)i;

    char *enc = NULL; size_t enc_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_from_bin(orig, 256, &enc, &enc_len));
    ASSERT_EQ_INT(1024, (int)enc_len);

    uint8_t *dec = NULL; size_t dec_len = 0;
    ASSERT_EQ_INT(BTX_OK, btx_to_bin(enc, enc_len, &dec, &dec_len, NULL));
    ASSERT_EQ_INT(256, (int)dec_len);
    ASSERT_EQ_MEM(orig, dec, 256);

    btx_free(enc);
    btx_free(dec);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void) {
    test_to_bin_hex();
    test_to_bin_bits();
    test_to_bin_errors();
    test_to_bin_mixed();
    test_to_bin_empty();
    test_from_bin();
    test_from_bin_empty();
    test_strerror();
    test_invalid_args();
    test_free_null();
    test_roundtrip();
    test_roundtrip_all_bytes();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
