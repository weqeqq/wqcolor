# wqcolor

`wqcolor` is a C++23 color conversion library for byte-based pixel buffers.
It provides deterministic format conversions with optional SIMD acceleration and explicit execution policy control.

## What it supports

- Formats: `Rgb`, `Rgba`, `Grayscale`, `Cmyk`
- Standards: `Bt601`, `Bt709`
- Execution modes: `Sequential`, `Parallel`

The public API lives in `headers/weqeqq/color.h` (`#include <weqeqq/color.h>`).

## Key features

- Fast conversions between common 8-bit formats.
- Explicit runtime execution policy (`Sequential` vs `Parallel`).
- Optional SIMD path (`-Dsimd=enabled` / `-Dsimd=disabled`).
- Meson build with gtest-based test coverage.

## Build

### Prerequisites

- C++23 compiler
- Meson + Ninja
- `wqparallel` (found via the system or downloaded through the Meson wrap fallback)

### Configure and compile

```bash
meson setup build --buildtype=release
meson compile -C build
```

### Install

```bash
meson setup build-install --buildtype=release -Dinstall=enabled
meson compile -C build-install
meson install -C build-install
```

## API contract

- `Grayscale` represents visual luma, not the raw `K` channel from CMYK.
- `Convert(input, output, ...)` requires `input` and `output` to be non-overlapping spans.
- Invalid formats, invalid standards, and overlapping spans are contract violations.
- Debug builds throw `weqeqq::color::Error`; release builds return without writing output bytes.

## API quick start

### Basic conversion

```cpp
#include <weqeqq/color.h>

#include <cstdint>
#include <vector>

int main() {
  using namespace weqeqq::color;

  std::vector<std::uint8_t> rgb = {
      255, 0, 0,   // red
      0, 255, 0,   // green
      0, 0, 255,   // blue
  };

  std::vector<std::uint8_t> gray =
      Convert(rgb, Format::kRgb, Format::kGrayscale, Standard::kBt709);
}
```

### Explicit parallel policy

```cpp
#include <weqeqq/color.h>

#include <cstdint>
#include <vector>

int main() {
  using namespace weqeqq::color;
  using weqeqq::parallel::Execution;

  std::vector<std::uint8_t> input = {255, 0, 0, 128, 64, 32};
  std::vector<std::uint8_t> output(8, 0);

  Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601,
          Execution::kParallel);
}
```

### Channel counts

| Format | Channels |
| --- | ---: |
| `Rgb` | 3 |
| `Rgba` | 4 |
| `Grayscale` | 1 |
| `Cmyk` | 4 |

## Meson options

Current options from `meson_options.txt`:

- `-Dtests=<auto|enabled|disabled>`
- `-Dinstall=<auto|enabled|disabled>`
- `-Dsimd=<enabled|disabled>`

## Tests

```bash
meson setup build-test -Dtests=enabled
meson compile -C build-test
meson test -C build-test --print-errorlogs
```

The test executables are registered with `protocol: 'gtest'` in Meson.

## Project layout

- `headers/` public headers
- `sources/` library implementation
- `tests/` gtest suite
- `meson.build`, `meson_options.txt` build configuration
