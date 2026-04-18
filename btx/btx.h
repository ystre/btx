#ifndef BTX_BTX_H
#define BTX_BTX_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    BTX_OK = 0,
    BTX_ERR_INVALID_ARG,        /* NULL or invalid argument */
    BTX_ERR_INVALID_TOKEN,
    BTX_ERR_INVALID_HEX_SYNTAX,
    BTX_ERR_INVALID_BIT_SYNTAX,
    BTX_ERR_BIT_NONCONTIGUOUS,
    BTX_ERR_BIT_ORDER,
    BTX_ERR_BIT_OVERLAP,
    BTX_ERR_BIT_INCOMPLETE,
    BTX_ERR_OOM,
    BTX_RESULT_MAX, /* sentinel — not a valid result code */
} btx_result_t;

/**
 * @brief Decode BTX text to binary.
 *
 * @param text    Input BTX text. Must not be NULL.
 * @param len     Length of @p text in bytes.
 * @param out     On success, set to a newly allocated buffer; free with btx_free(). Must not be NULL.
 * @param out_len On success, set to the number of decoded bytes. Must not be NULL.
 *
 * @return BTX_OK on success; error code otherwise. On error, @p out and @p out_len are unmodified.
 */
btx_result_t btx_decode(const char *text, size_t len, uint8_t **out, size_t *out_len);

/**
 * @brief Encode binary data to BTX text (\xNN per byte).
 *
 * @param data    Input bytes. Must not be NULL.
 * @param len     Number of bytes to encode.
 * @param out     On success, set to a NUL-terminated string; free with btx_free(). Must not be NULL.
 * @param out_len On success, set to the length of the encoded string (excluding NUL). Must not be NULL.
 *
 * @return BTX_OK on success; error code otherwise. On error, @p out and @p out_len are unmodified.
 */
btx_result_t btx_encode(const uint8_t *data, size_t len, char **out, size_t *out_len);

/**
 * @brief Validate BTX text without producing output.
 *
 * @param text  Input BTX text. Must not be NULL.
 * @param len   Length of @p text in bytes.
 *
 * @return BTX_OK if valid; error code otherwise.
 */
btx_result_t btx_validate(const char *text, size_t len);

/**
 * @brief Return a human-readable string for a result code.
 *
 * @param result  A btx_result_t value.
 *
 * @return A static string describing the result. Never NULL.
 */
const char* btx_strerror(btx_result_t result);

/**
 * @brief Free memory allocated by btx_decode() or btx_encode().
 *
 * @param ptr  Pointer to free. NULL is safe.
 */
void btx_free(void *ptr);

#endif /* BTX_BTX_H */
