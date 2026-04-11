# Future ideas

## `btx inspect`

Human-readable breakdown of a BTX file showing each token, its position, and the byte value it produces.

```
offset  token               value
0       \x12                0x12 (18)
1       \xac                0xac (172)
2       \b1111'____ +
        \b____'0000         0xf0 (240)
```

## `btx fmt`

Formatter/normalizer for BTX text. Produces a canonical version of a BTX file:

- normalize hex digits to lowercase
- strip or standardize comments
- consistent spacing and indentation
- optional column width wrapping

## CLI args

- `encode --width <n>` — wrap output at N bytes per line
- `encode --format <hex|bits>` — choose output token style
- `decode --strict` — fail on unknown tokens instead of skipping
- `validate --quiet` — suppress error messages, exit code only
