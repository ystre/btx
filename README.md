# btx

BTX (Binary TeXt) is a textual representation of binary data. This project provides a C library (`libbtx`) and a CLI tool (`btx`) for encoding and decoding BTX files.

See [SPECIFICATION.md](SPECIFICATION.md) for the full format specification.

## Format overview

```
\x12\xac              // hex byte tokens
\b1111'____ \b____'0000  // bit-pack tokens (two partials → one byte 0xf0)
// this is a comment
\xab  // inline comment
```

## Build

```sh
cmake -B build
cmake --build build
```

## Test

```sh
ctest --test-dir build
```

## Usage

```sh
btx encode <file>     # binary → BTX text (stdout)
btx decode <file>     # BTX text → binary (stdout)
btx validate <file>   # validate BTX text; exit 1 on error
btx --help            # show usage
btx --version         # print version
```

Use `-` as `<file>` to read from stdin.

## Library API

```c
#include <btx/btx.h>

btx_result_t btx_decode(const char *text, size_t len, uint8_t **out, size_t *out_len);
btx_result_t btx_encode(const uint8_t *data, size_t len, char **out, size_t *out_len);
btx_result_t btx_validate(const char *text, size_t len);
const char*  btx_strerror(btx_result_t result);
void         btx_free(void *ptr);
```

## RFC Draft

The formal specification draft is in `rfc/draft-viragh-btx-format-00.xml`
(RFC XML v3 format). To render it as text or HTML:

```sh
pip install xml2rfc
xml2rfc rfc/draft-viragh-btx-format-00.xml --text   # → draft-viragh-btx-format-00.txt
xml2rfc rfc/draft-viragh-btx-format-00.xml --html   # → draft-viragh-btx-format-00.html
```

## Install

```sh
cmake --install build
```
