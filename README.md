# dec2base — Arbitrary-Precision Decimal to Hex & Binary Converter

A command-line utility that converts arbitrarily large decimal integers
to **hexadecimal and binary**. Unlike converters that rely on the
platform's native integer types, `dec2base` uses string arithmetic
(repeated division by 16), so there is no compiled-in upper limit on
input size — it handles big integers well beyond 2^512. Each hex digit
expands to a 4-bit group, so the binary output shares the same unbounded
range.

## Build

```
make          # debug build  (-O0, debug symbols)
make release  # release build (-O2, no debug symbols)
```

Requires gcc and a C99-capable toolchain.

## Usage

```
./dec2base <decimal_integer>
```

Output shows the value under a `Hex:` label and a `Bin:` label, with
leading-zero padding to the nearest 4-digit boundary and a space every
4 digits. Both bases wrap at 16 groups (79 columns) per line so large
values stay within an 80-column terminal or printout.

### Examples

```
$> ./dec2base 255

Hex:
00ff

Bin:
1111 1111

$> ./dec2base 65535

Hex:
ffff

Bin:
1111 1111 1111 1111

$> ./dec2base 4294967296

Hex:
0001 0000 0000

Bin:
0001 0000 0000 0000 0000 0000 0000 0000 0000
```

For a large value such as 2^512, the hex and binary blocks each wrap
across multiple 79-column lines, column-aligned with one another.

## Input rules

- Digits only — no leading `+`, no `-`, no decimal point
- Empty input is rejected with an error
- Leading zeros are accepted (`007` converts correctly)

## Algorithm

The conversion uses long division of the decimal string by 16, collecting
remainders to build the hex digits from least significant to most
significant, then reversing. The binary output is derived from the hex
string by expanding each hex digit to its 4-bit group via a lookup table.
Each division pass is O(n) in the number of decimal digits; the full
conversion is O(n²). For the input sizes this tool is intended for —
numbers up to and beyond 2^512 — this is fast enough to be imperceptible.

## Testing

A bash test harness is included:

```
./test-dec2base.sh              # 10,000 tests, random seed
./test-dec2base.sh -n 1000      # fewer tests
./test-dec2base.sh -s 42        # fixed seed (reproducible)
./test-dec2base.sh -v           # verbose output
```

The harness generates random integers in `[0, 2^512]`, runs them through
`dec2base`, and validates **both** outputs by converting the hex back to
decimal with Python's `int(hex, 16)` and the binary with `int(bin, 2)`.
Requires Python 3.

10,000 random values in `[0, 2^512]` have been verified: 10,000 passed,
0 failed. Correctness beyond 2^512 is expected from the algorithm but has
not been tested.

## Related projects

Big integer conversion is part of the same family of arbitrary-precision
tools in the BigFermat project:

- [bi-quad](https://github.com/scurvyhund/bi-quad) — exhaustive hunt for
  bi-quadratic emirps and prime palindromes on the curve 2n²+2n+1; the
  prime search that motivates this toolset.
- [bigint-mul](https://github.com/scurvyhund/bigint-mul) — arbitrary-
  precision big integer multiplication in C; schoolbook string arithmetic
  and 256-bit `__uint128_t` approaches.

---

## Author

Jim Adams
