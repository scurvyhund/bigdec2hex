# final-d2h

A command-line utility that converts arbitrarily large decimal integers to
hexadecimal. Unlike converters that rely on the platform's native integer
types, `final-d2h` uses string arithmetic (repeated division by 16), so
there is no compiled-in upper limit on input size.

## Build

```
make          # debug build  (-O0, debug symbols)
make release  # release build (-O2, no debug symbols)
```

Requires gcc and a C99-capable toolchain.

## Usage

```
./final-d2h <decimal_integer>
```

Output is formatted with a `Hex:` label, leading-zero padding to the
nearest 4-digit boundary, and spaces every 4 digits for readability.

### Examples

```
$> ./final-d2h 255
Hex: (00ff)

$> ./final-d2h 65535
Hex: (ffff)

$> ./final-d2h 4294967296
Hex: (0001 0000 0000)

$> ./final-d2h 13407807929942597099574024998205846127479365820592393377723561443721764030073546976801874298166903427690031858186486050853753882811946569946433649006084096
Hex: (0001 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000)
```

That last value is 2^512.

## Input rules

- Digits only — no leading `+`, no `-`, no decimal point
- Empty input is rejected with an error
- Leading zeros are accepted (`007` converts correctly)

## Algorithm

The conversion uses long division of the decimal string by 16, collecting
remainders to build the hex digits from least significant to most
significant, then reversing. Each division pass is O(n) in the number of
decimal digits; the full conversion is O(n²). For the input sizes this tool
is intended for — numbers up to and beyond 2^512 — this is fast enough to
be imperceptible.

## Testing

A bash test harness is included:

```
./test-final-d2h.sh              # 10,000 tests, random seed
./test-final-d2h.sh -n 1000      # fewer tests
./test-final-d2h.sh -s 42        # fixed seed (reproducible)
./test-final-d2h.sh -v           # verbose output
```

The harness generates random integers in `[0, 2^512]`, runs them through
`final-d2h`, and validates each result by converting the hex output back to
decimal using Python's `int(hex, 16)`. Requires Python 3.

10,000 random values in `[0, 2^512]` have been verified: 10,000 passed,
0 failed. Correctness beyond 2^512 is expected from the algorithm but has
not been tested.

## Author

Jim Adams
