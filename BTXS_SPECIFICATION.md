# BTXS Schema Specification

BTXS (BTX Schema) is a DSL for describing the structure of binary data decoded from a BTX file.
It is used by `btx inspect` to produce a human-readable, field-by-field breakdown.

Schema files use the `.btxs` extension.

## Syntax

```
schema     ::= "struct" name "{" field+ "}"
field      ::= name ":" type "[" size "]" annotation?
type       ::= "uint" | "int" | "str" | "bytes"
size       ::= number ("b" | "B") | "$" name
annotation ::= "@le" | "@be"
name       ::= [a-zA-Z_][a-zA-Z0-9_]*
number     ::= [0-9]+
```

Each field must be on its own line. No whitespace is permitted within `type[size]` or `$name`. Comments follow BTX convention: `//` to end of line.

## Fields

Each field has a name, a type, a size, and an optional endianness annotation.

```
magic:    uint[2B]       // 2-byte unsigned int, default endianness
flags:    uint[4b]       // 4-bit unsigned int
reserved: uint[4b]
length:   uint[4B]  @be  // override to big-endian
label:    str[$length]   // variable length, value taken from 'length' field
checksum: bytes[2B]
```

## Types

| Type    | Description                                      |
|---------|--------------------------------------------------|
| `uint`  | Unsigned integer, displayed as decimal and hex   |
| `int`   | Signed integer, sign shown only when negative    |
| `str`   | ASCII string                                     |
| `bytes` | Raw bytes, displayed as hex                      |

## Size

Sizes are suffixed with `b` (bits) or `B` (bytes):

- `4b` — 4 bits
- `2B` — 2 bytes (16 bits)

For variable-length fields, use `$<fieldname>` to reference the value of a previously parsed field:

- `str[$length]` — length in bytes taken from the `length` field

The referenced field must appear earlier in the struct and must be of type `uint` or `int`.
Variable-length sizes are always interpreted as bytes.

## Endianness

Default endianness is little-endian. Precedence (highest to lowest):

1. Field-level annotation: `@le` or `@be`
2. CLI flag: `--le` or `--be`
3. Default: little-endian

Endianness only applies to `uint` and `int` fields wider than 1 byte.

## CLI Usage

```sh
btx inspect --schema <file.btxs> [--be] <file>
```

- `--schema <file.btxs>` — schema to apply
- `--be` — override default endianness to big-endian for this invocation

## Inspect Output

```
field       offset  size    type    value
magic       0       2B      uint    0x12ac (4780)
flags       2       4b      uint    0xf (15)
reserved    2+4b    4b      uint    0x0 (0)
length      3       4B      uint    0x00000400 (1024)
label       7       1024B   str     "hello world..."
checksum    1031    2B      bytes   0xdead
```

## Example

```
// network_packet.btxs
struct NetworkPacket {
    magic:      uint[2B]   @be
    version:    uint[4b]
    flags:      uint[4b]
    length:     uint[4B]
    payload:    bytes[$length]
    checksum:   uint[2B]   @be
}
```
