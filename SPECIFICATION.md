# BTX Format Specification

## Overview

BTX (Binary TeXt) is a textual representation of binary data. It allows binary bytes to be written in a human-readable format, with support for comments and bit-level composition.

## Tokens

### Hex byte token

Syntax: `\xNN` where `NN` is exactly two hexadecimal digits (case-insensitive).

Represents a single byte with the given value.

Examples:
```
\x00  // byte 0x00
\xff  // byte 0xff
\x1A  // byte 0x1a
```

### Bit-pack token

Syntax: `0bBBBBBBBB` or `0bBBBB'BBBB` where each `B` is `0`, `1`, or `_`.

- Exactly 8 bit positions must follow the `0b` prefix.
- An optional `'` separator may appear after the 4th bit position for readability.
- `0` and `1` mean the token **owns** that bit position and sets it to the respective value.
- `_` means the token does **not** own that bit position.

Examples:
```
0b1111'0000   // owns all 8 bits, value 0xf0
0b1111'____   // owns upper nibble only
0b____'0000   // owns lower nibble only
```

#### Bit-pack groups

Consecutive bit-pack tokens form a group that produces one or more complete bytes. The following rules apply:

1. **Contiguous ownership**: within a single token, owned bits (`0` or `1`) must form a contiguous run. A token may not have gaps between owned bits (e.g., `0b1__1'____` is invalid).

2. **Ordered ownership**: across tokens in a group, owned runs must proceed strictly from MSB to LSB. Each token's owned run must start exactly where the previous token's run ended.

3. **No overlap**: each bit position must be owned by exactly one token. If two tokens both own the same bit position (regardless of the values they set), it is an error.

4. **Complete bytes**: after each token, if the total number of owned bits is a multiple of 8, the completed byte(s) are flushed. A group implicitly ends at each byte boundary. If a non-`0b` token is encountered while a group is mid-byte (owned bit count not a multiple of 8), it is an error.

5. **Multiple consecutive groups**: the entire file may consist of bit-pack groups. Groups are separated implicitly by byte boundaries — no explicit delimiter is required.

Example — two partials forming one byte:
```
0b1111'____ 0b____'0011  // valid: 0xf3
```

Example — three partials forming one byte:
```
0b11______ 0b__1100__ 0b______11  // valid: 0b11110011 = 0xf3
```

Example — invalid (gap within a token):
```
0b1__1'____  // error: non-contiguous ownership within token
```

Example — invalid (wrong order):
```
0b1111'____ 0b______11  // error: skips bits 5-2
```

Example — invalid (incomplete byte):
```
0b1111'____ \xff  // error: group is mid-byte when \xff is encountered
```

## Whitespace and separators

Whitespace (spaces, tabs, newlines, carriage returns) between tokens is ignored and may be used freely for formatting.

## Comments

`//` begins a single-line comment. Everything from `//` to the end of the line is ignored.

Comments may appear:
- On a line by themselves
- At the end of a line containing tokens

Example:
```
// this is a full-line comment
\x12 \xac  // inline comment
```

## Error conditions

| Error | Description |
|---|---|
| `BTX_ERR_INVALID_TOKEN` | Unrecognized token in input |
| `BTX_ERR_INVALID_HEX` | Malformed `\xNN` token |
| `BTX_ERR_INVALID_BIT_SYNTAX` | Malformed `0bBBBBBBBB` token |
| `BTX_ERR_BIT_NONCONTIGUOUS` | Owned bits within a single token are not contiguous |
| `BTX_ERR_BIT_ORDER` | Token's owned run does not start where the previous run ended |
| `BTX_ERR_BIT_OVERLAP` | Two tokens own the same bit position |
| `BTX_ERR_BIT_INCOMPLETE` | Non-`0b` token encountered while group is mid-byte |
| `BTX_ERR_OOM` | Memory allocation failure |

## Library API

```c
#include <btx/btx.h>

// Decode BTX text to binary bytes.
btx_result_t btx_decode(const char *text, size_t len, uint8_t **out, size_t *out_len);

// Encode binary bytes to BTX text (\xNN per byte).
btx_result_t btx_encode(const uint8_t *data, size_t len, char **out, size_t *out_len);

// Validate BTX text without producing output.
btx_result_t btx_validate(const char *text, size_t len);

// Return a human-readable string for a result code.
const char* btx_strerror(btx_result_t result);

// Free memory allocated by btx_decode or btx_encode.
void btx_free(void *ptr);
```

The caller is responsible for freeing output buffers via `btx_free`.

## CLI

```
btx encode <file>     # read binary, write BTX text to stdout
btx decode <file>     # read BTX text, write binary to stdout
btx validate <file>   # validate BTX text; exit 1 on error
btx --help            # show usage
btx --version         # print version
```

Use `-` as `<file>` to read from stdin. Errors are written to stderr with a non-zero exit code.

## Future extensions

The format is designed to allow additional token types beyond `\xNN` and `0bBBBBBBBB` without breaking existing files. New token prefixes can be introduced in later revisions.
