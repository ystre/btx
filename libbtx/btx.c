#include <libbtx/btx.h>

#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Lexer
 * ---------------------------------------------------------------------- */

typedef enum {
    TOK_HEX,   /* \xNN  */
    TOK_BITS,  /* \bBBBBBBBB */
    TOK_EOF,
} tok_type_t;

typedef struct {
    tok_type_t type;
    union {
        uint8_t byte;       /* TOK_HEX: decoded byte value */
        uint8_t bits[8];    /* TOK_BITS: 0/1/2 per position; 2 = unowned (_) */
    };
} token_t;

typedef struct {
    const char *p;
    const char *end;
    int line;   /* 1-based */
    int col;    /* 1-based */
} lexer_t;

static void lexer_init(lexer_t *l, const char *text, size_t len) {
    l->p    = text;
    l->end  = text + len;
    l->line = 1;
    l->col  = 1;
}

static void skip_whitespace_and_comments(lexer_t *l) {
    while (l->p < l->end) {
        /* whitespace */
        if (*l->p == ' ' || *l->p == '\t' || *l->p == '\r' || *l->p == '\n') {
            if (*l->p == '\n') {
                l->line++;
                l->col = 1;
            } else {
                l->col++;
            }
            l->p++;
            continue;
        }

        /* comment */
        if (l->p + 1 < l->end && l->p[0] == '/' && l->p[1] == '/') {
            while (l->p < l->end && *l->p != '\n') {
                l->p++;
                l->col++;
            }
            continue;
        }

        break;
    }
}

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static btx_result_t next_token(lexer_t *l, token_t *tok) {
    if (l->p >= l->end) {
        tok->type = TOK_EOF;
        return BTX_OK;
    }

    /* \xNN */
    if (l->p + 3 < l->end && l->p[0] == '\\' && l->p[1] == 'x') {
        int hi = hex_digit(l->p[2]);
        int lo = hex_digit(l->p[3]);
        if (hi < 0 || lo < 0) return BTX_ERR_INVALID_HEX_SYNTAX;
        tok->type = TOK_HEX;
        tok->byte = (uint8_t)((hi << 4) | lo);
        l->p += 4;
        l->col += 4;
        return BTX_OK;
    }

    /* \bBBBBBBBB or \bBBBB'BBBB */
    if (l->p + 1 < l->end && l->p[0] == '\\' && l->p[1] == 'b') {
        l->p += 2;
        l->col += 2;
        int pos = 0;
        while (pos < 8 && l->p < l->end) {
            char c = *l->p;
            if (c == '\'' && pos == 4) { l->p++; l->col++; continue; }
            if (c == '0')      { tok->bits[pos++] = 0; l->p++; l->col++; }
            else if (c == '1') { tok->bits[pos++] = 1; l->p++; l->col++; }
            else if (c == '_') { tok->bits[pos++] = 2; l->p++; l->col++; }
            else break;
        }
        if (pos != 8) return BTX_ERR_INVALID_BIT_SYNTAX;
        if (l->p < l->end && *l->p == '\'') return BTX_ERR_INVALID_BIT_SYNTAX;
        tok->type = TOK_BITS;
        return BTX_OK;
    }

    return BTX_ERR_INVALID_TOKEN;
}

/* -------------------------------------------------------------------------
 * Bit-pack validation helpers
 * ---------------------------------------------------------------------- */

/* Returns the index of the first owned bit, or 8 if none. */
static int first_owned(const uint8_t bits[8]) {
    for (int i = 0; i < 8; i++) if (bits[i] != 2) return i;
    return 8;
}

/* Returns the index one past the last owned bit, or 0 if none. */
static int last_owned_end(const uint8_t bits[8]) {
    for (int i = 7; i >= 0; i--) if (bits[i] != 2) return i + 1;
    return 0;
}

/* Check that owned bits form a contiguous run (no gaps). */
static btx_result_t check_contiguous(const uint8_t bits[8]) {
    int first = first_owned(bits);
    int end   = last_owned_end(bits);
    for (int i = first; i < end; i++)
        if (bits[i] == 2) return BTX_ERR_BIT_NONCONTIGUOUS;
    return BTX_OK;
}

/* -------------------------------------------------------------------------
 * Dynamic output buffer
 * ---------------------------------------------------------------------- */

typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   cap;
} buf_t;

static btx_result_t buf_push(buf_t *b, uint8_t byte) {
    if (b->len == b->cap) {
        if (b->cap > SIZE_MAX / 2) return BTX_ERR_OOM;
        size_t ncap = b->cap ? b->cap * 2 : 64;
        uint8_t *nd = realloc(b->data, ncap);
        if (!nd) return BTX_ERR_OOM;
        b->data = nd;
        b->cap  = ncap;
    }
    b->data[b->len++] = byte;
    return BTX_OK;
}

/* -------------------------------------------------------------------------
 * Core parse loop (shared by decode and validate)
 * ---------------------------------------------------------------------- */

/*
 * bit_acc:      accumulated byte value for current group
 * next_bit:     next expected bit position (0-7 within current byte,
 *               but tracked globally mod 8 across the group)
 * owned_mask:   bitmask of positions already owned in current byte
 */
typedef struct {
    uint8_t  bit_acc;
    int      next_bit;   /* 0..7; 0 means start of a fresh byte */
    uint8_t  owned_mask;
} bitstate_t;

static btx_result_t process_bits(bitstate_t *bs, const token_t *tok, buf_t *out) {
    btx_result_t r = check_contiguous(tok->bits);
    if (r != BTX_OK) return r;

    int first = first_owned(tok->bits);
    int end   = last_owned_end(tok->bits);

    /* A fully unowned token (all _) is a no-op but valid */
    if (first == 8) return BTX_OK;

    /* Overlap check must come before order check: two tokens owning the same
     * bit are an overlap error, not an order error. */
    for (int i = first; i < end; i++) {
        uint8_t mask = (uint8_t)(1u << (7 - i));
        if (bs->owned_mask & mask) return BTX_ERR_BIT_OVERLAP;
    }

    /* Order check: owned run must start exactly at next_bit */
    if (first != bs->next_bit) return BTX_ERR_BIT_ORDER;

    /* Apply bits */
    for (int i = first; i < end; i++) {
        uint8_t mask = (uint8_t)(1u << (7 - i));
        bs->owned_mask |= mask;
        if (tok->bits[i] == 1) bs->bit_acc |= mask;
    }

    bs->next_bit = end % 8;

    /* Flush completed byte */
    if (end == 8) {
        if (out) {
            r = buf_push(out, bs->bit_acc);
            if (r != BTX_OK) return r;
        }
        bs->bit_acc    = 0;
        bs->owned_mask = 0;
        bs->next_bit   = 0;
    }

    return BTX_OK;
}

static btx_result_t parse(const char *text, size_t len, buf_t *out, btx_error_t *err) {
    lexer_t    l;
    token_t    tok;
    bitstate_t bs = {0, 0, 0};
    btx_result_t r;

    lexer_init(&l, text, len);

    for (;;) {
        skip_whitespace_and_comments(&l);
        int tok_line = l.line;
        int tok_col  = l.col;
        r = next_token(&l, &tok);
        if (r != BTX_OK) {
            if (err) { err->line = tok_line; err->col = tok_col; }
            return r;
        }

        if (tok.type == TOK_EOF) break;

        if (tok.type == TOK_BITS) {
            r = process_bits(&bs, &tok, out);
            if (r != BTX_OK) {
                if (err) { err->line = tok_line; err->col = tok_col; }
                return r;
            }
            continue;
        }

        /* TOK_HEX: check no pending bit group */
        if (bs.next_bit != 0) {
            if (err) { err->line = tok_line; err->col = tok_col; }
            return BTX_ERR_BIT_INCOMPLETE;
        }

        if (out) {
            r = buf_push(out, tok.byte);
            if (r != BTX_OK) {
                if (err) { err->line = tok_line; err->col = tok_col; }
                return r;
            }
        }
    }

    if (bs.next_bit != 0) {
        if (err) { err->line = l.line; err->col = l.col; }
        return BTX_ERR_BIT_INCOMPLETE;
    }

    return BTX_OK;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

btx_result_t btx_decode(const char *text, size_t len, uint8_t **out, size_t *out_len, btx_error_t *err) {
    if ((!text && len > 0) || !out || !out_len) return BTX_ERR_INVALID_ARG;
    buf_t buf = {NULL, 0, 0};
    btx_result_t r = parse(text, len, &buf, err);
    if (r != BTX_OK) { free(buf.data); return r; }
    *out     = buf.data;
    *out_len = buf.len;
    return BTX_OK;
}

btx_result_t btx_encode(const uint8_t *data, size_t len, char **out, size_t *out_len) {
    /* Each byte becomes \xNN (4 chars) + NUL terminator */
    if ((!data && len > 0) || !out || !out_len) return BTX_ERR_INVALID_ARG;
    if (len > (SIZE_MAX - 1) / 4) return BTX_ERR_OOM;
    size_t sz = len * 4 + 1;
    char *buf = malloc(sz);
    if (!buf) return BTX_ERR_OOM;
    char *p = buf;
    for (size_t i = 0; i < len; i++) {
        static const char hex[] = "0123456789abcdef";
        *p++ = '\\';
        *p++ = 'x';
        *p++ = hex[data[i] >> 4];
        *p++ = hex[data[i] & 0xf];
    }
    *p = '\0';
    *out     = buf;
    *out_len = (size_t)(p - buf);
    return BTX_OK;
}

btx_result_t btx_validate(const char *text, size_t len, btx_error_t *err) {
    if (!text && len > 0) return BTX_ERR_INVALID_ARG;
    return parse(text, len, NULL, err);
}

const char* btx_strerror(btx_result_t result) {
    switch (result) {
        case BTX_OK:                        return "success";
        case BTX_ERR_INVALID_ARG:           return "invalid argument";
        case BTX_ERR_INVALID_TOKEN:         return "invalid token";
        case BTX_ERR_INVALID_HEX_SYNTAX:    return "invalid hex syntax";
        case BTX_ERR_INVALID_BIT_SYNTAX:    return "invalid bit syntax";
        case BTX_ERR_BIT_NONCONTIGUOUS:     return "non-contiguous bit ownership";
        case BTX_ERR_BIT_ORDER:             return "out of order bit ownership";
        case BTX_ERR_BIT_OVERLAP:           return "overlapping bit ownership";
        case BTX_ERR_BIT_INCOMPLETE:        return "incomplete byte";
        case BTX_ERR_OOM:                   return "out of memory";
        default:                            return "unknown error";
    }
}

void btx_free(void *ptr) {
    free(ptr);
}
