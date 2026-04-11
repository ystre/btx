# BTX Format Specification

BTX (Binary TeXt) is a human-readable representation of binary data, supporting
both hexadecimal and bit-level expressions. It is designed to be easy to write,
read, and process using standard command-line tools.

## Syntax Rules

### Hexadecimal Bytes

Expressed using the `\xHH` syntax, where `H` is a hexadecimal digit (0-9, a-f, A-F).
Each `\xHH` sequence represents exactly one byte.

```
\x00   // byte 0x00
\xff   // byte 0xff
\x1A   // byte 0x1a
```

### Binary Bits

Expressed using the `\bBBBBBBBB` syntax, where `B` is a bit position.

- Each `\b` token must define exactly 8 bit positions.
- `0` and `1` set the bit value and claim ownership of that position.
- `_` marks a position as unowned (placeholder).
- Bits are accumulated across consecutive `\b` tokens.
- Every complete sequence of 8 owned bits produces one byte.
- An optional `'` separator may appear after the 4th bit for readability.

```
\b11110000        // owns all 8 bits, value 0xf0
\b1111'____       // owns upper nibble only
\b____'0000       // owns lower nibble only
\b1111'____ \b____'0000  // two partials → one byte 0xf0
```

#### Ownership Rules

1. **Contiguous**: owned bits within a single token must form a contiguous run.
   Gaps between owned bits are invalid (e.g., `\b1__1'____`).

2. **Ordered**: across tokens, owned runs must proceed strictly from MSB to LSB.
   Each token's run must start exactly where the previous one ended.

3. **Non-overlapping**: each bit position must be owned by exactly one token.

4. **Complete bytes**: when the total owned bit count reaches a multiple of 8,
   the completed byte is emitted. A non-`\b` token encountered mid-byte is an error.

### Whitespace and Separators

Whitespace (spaces, tabs, newlines) between tokens is ignored and may be used
freely for formatting. The `'` separator within a `\b` token is also ignored.

### Comments

`//` begins a single-line comment extending to the end of the line.
Comments may appear on their own line or inline after tokens.

```
// full-line comment
\x12 \xac  // inline comment
```

## Examples

Mixed hex and bit-pack tokens:

```
\xab \xcd                          // two bytes: 0xab, 0xcd
\b11110000                         // one byte: 0xf0
\b1111'____ \b____'0000            // two partials → one byte: 0xf0
\b11______ \b__1100__ \b______11   // three partials → one byte: 0xf3
\xab \b1111'____ \b____'1111 \xcd  // 0xab, 0xff, 0xcd
```

Invalid inputs:

```
\b1__1'____   // error: non-contiguous ownership within token
\b1111'____ \b______11   // error: ordered ownership violated (skips bits 5-2)
\b1111'____ \b1111'____  // error: overlapping ownership
\b1111'____ \xff         // error: incomplete byte before non-\b token
```

## Extensibility

The format is designed to allow additional token types in future revisions.
New token prefixes may be introduced without breaking existing files.
