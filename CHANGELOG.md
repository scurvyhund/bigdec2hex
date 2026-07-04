# Changelog — dec2base

---

## 2026-07-03 — Binary output added; renamed final-d2h → dec2base

### New feature — binary output

- The tool now prints every value in **both hex and binary**. Each hex
  digit expands to its 4-bit group via a static lookup table, so binary
  inherits the same arbitrary-precision range as hex (verified to 2^512).
- New `format_bin_with_padding()` mirrors `format_hex_with_padding()`.

### Renamed to dec2base

- `final-d2h` no longer describes a tool that emits two bases. Renamed:
  - `final-d2h.c` → `dec2base.c`
  - `test-final-d2h.sh` → `test-dec2base.sh`
  - Makefile `TARGET`/`SRC`, README, source header, and `.gitignore`
    build-output/log entries updated to match. (The old `final-d2h`
    binary stays in `.gitignore` until the stale build is removed.)

### Output format changes

- Removed the `( )` wrapping from hex output.
- Each base prints under its own label on its own line (`Hex:` / `Bin:`),
  so the two blocks are column-aligned.
- Both bases wrap at 16 four-char groups (79 columns) per line, keeping
  large values within an 80-column terminal or printout. Previously the
  single-line hex output ran off the page for values past ~2^128.

### Testing

- The harness (`test-dec2base.sh`) now validates **both** outputs each
  run — hex via `int(hex, 16)` and binary via `int(bin, 2)`. Re-verified
  with 10,000 random values in `[0, 2^512]` — 10,000 passed, 0 failed.

---

## 2026-06-10 — Public release prep and code audit

### Bug fixes

**Empty string input not rejected**
- `decimal_to_hex_string_bigint()` accepted `""` silently and returned
  `"Hex: ()"` with no error. Added an explicit check for
  `decimal_str[0] == '\0'` before the digit-validation loop, which cannot
  catch an empty string because
  it has no characters to iterate over.

**`isdigit` argument not cast to `unsigned char`**
- The C standard requires that arguments to `<ctype.h>` functions be
  representable as `unsigned char` or equal to `EOF`. Passing a plain `char`
  is undefined behavior on platforms where `char` is signed and the value is
  negative (i.e. non-ASCII input). Changed to
  `isdigit((unsigned char)decimal_str[i])`.

### Type correctness — signed `int` indices replaced with `size_t`

All loop counters and buffer indices that walk or size strings are now `size_t`,
matching the type returned by `strlen` and used by `malloc`.

| Variable | Function | Risk |
|---|---|---|
| `hex_pos` | `decimal_to_hex_string_bigint` | **Medium** — signed overflow would produce a negative buffer index and an out-of-bounds write. Most consequential change. |
| `i` (reversal loop) | `decimal_to_hex_string_bigint` | Changed to `size_t` together with `hex_pos` to avoid a signed/unsigned comparison. |
| `i` (digit-validation loop) | `decimal_to_hex_string_bigint` | Signed overflow UB if input > `INT_MAX` bytes. |
| `i` | `strip_leading_zeros` | Same root cause. |
| `i` | `divide_string_by_16` | Same root cause. |

Variables that remain `int` because their values are proven
bounded by arithmetic:
- `remainder` in `divide_string_by_16`: always 0–15 (`value % 16`)
- `digit`: always 0–9 (digit character minus `'0'`)
- `value`: max 159 (`15 * 10 + 9`), fits any integer type

`format_hex_with_padding` already used `size_t` throughout and
required no changes.
The `size_t` subtraction `(total_hex_chars >> 2) - 1` at first appears to risk
underflow to `SIZE_MAX`, but is safe because the `input_len == 0` early-return
guard guarantees `total_hex_chars >= 4` before that line is reached.

### Verified correct to 2^512

The test harness (`test-final-d2h.sh`) was updated from `[0, 2^394]` to
`[0, 2^512]` and run for 10,000 random values — 10,000 passed, 0 failed.
Results validated against Python's `hex()` reference. Correctness beyond
2^512 is expected from the algorithm but has not been tested.

### Repository hygiene

- **Makefile** added to the repo (it was excluded by `.gitignore`); without it
  there were no build instructions for anyone cloning the repo.
- **Compiled binaries removed** from git tracking (`final-d2h`, `d2h-final`).
  Binaries are platform-specific and should be built from source.
- **`d2h-final` and `d2h-final.c`** (the older `strtol`-based version, limited
  to 64-bit `long`) removed — superseded by the
  string-arithmetic implementation.
- **`.gitignore` rewritten** to cover: build output (`final-d2h`, `final-test`,
  `test-loop`, `wc`), generated test logs (`test-d2h-*.log`), personal/local
  files (`scott-email.docx`, `qq`, `code_journey/`, `test-loop.c`).
- **Makefile** given a `release` target (`-O2`, no debug symbols) alongside
  the existing `debug` target (`-gdwarf-5 -O0`). Default target is `debug`.
