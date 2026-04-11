#ifndef BTX_BTX_H
#define BTX_BTX_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    BTX_OK = 0,
    BTX_ERR_INVALID_TOKEN,
    BTX_ERR_INVALID_HEX,
    BTX_ERR_INVALID_BIT_SYNTAX,
    BTX_ERR_BIT_NONCONTIGUOUS,
    BTX_ERR_BIT_ORDER,
    BTX_ERR_BIT_OVERLAP,
    BTX_ERR_BIT_INCOMPLETE,
    BTX_ERR_OOM,
} btx_result_t;

/* Decode BTX text to binary. Caller frees *out with btx_free. */
btx_result_t btx_decode(const char *text, size_t len, uint8_t **out, size_t *out_len);

/* Encode binary to BTX text (\xNN per byte). Caller frees *out with btx_free. */
btx_result_t btx_encode(const uint8_t *data, size_t len, char **out, size_t *out_len);

/* Validate BTX text without producing output. */
btx_result_t btx_validate(const char *text, size_t len);

/* Return a human-readable string for a result code. */
const char* btx_strerror(btx_result_t result);

/* Free memory allocated by btx_decode or btx_encode. */
void btx_free(void *ptr);

#endif /* BTX_BTX_H */
