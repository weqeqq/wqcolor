
#pragma once

#include <cstddef>
#include <cstdint>
namespace weqeqq::color::simd {

enum class Format {
  kRgb,
  kRgba,
  kGrayscale,
  kCmyk,
};

enum class ColorStandard {
  kRec601,
  kRec709,
};

inline constexpr std::size_t ChannelCount(Format color) noexcept {
  switch (color) {
    case Format::kRgb:
      return 3;
    case Format::kRgba:
      return 4;
    case Format::kGrayscale:
      return 1;
    case Format::kCmyk:
      return 4;
    default:
      return 0;
  }
}

/* clang-format off */

void ConvertRgbToRgba            (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbToGrayscaleRec601 (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbToGrayscaleRec709 (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbToCmyk            (const std::uint8_t* input, std::uint8_t* output, std::size_t count);

void ConvertRgbaToRgb             (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbaToGrayscaleRec601 (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbaToGrayscaleRec709 (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertRgbaToCmyk            (const std::uint8_t* input, std::uint8_t* output, std::size_t count);

void ConvertGrayscaleToRgb  (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertGrayscaleToRgba (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertGrayscaleToCmyk (const std::uint8_t* input, std::uint8_t* output, std::size_t count);

void ConvertCmykToRgb       (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertCmykToRgba      (const std::uint8_t* input, std::uint8_t* output, std::size_t count);
void ConvertCmykToGrayscale (const std::uint8_t* input, std::uint8_t* output, std::size_t count);

void Copy(const std::uint8_t *input, std::uint8_t *output, std::size_t count);

/* clang-format on */

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
void ConvertChunk(const std::uint8_t* input, std::uint8_t* output,
                  std::size_t count) noexcept {
  using Function = void (*)(const std::uint8_t*, std::uint8_t*, std::size_t);

  /* clang-format off */
    auto function = []() -> Function {
        if constexpr (InputFormat == Format::kRgb && OutputFormat == Format::kRgba) {
            return ConvertRgbToRgba;
        }
        else if constexpr (InputFormat == Format::kRgb && OutputFormat == Format::kGrayscale) {

            if constexpr (Standard == ColorStandard::kRec601) return ConvertRgbToGrayscaleRec601;
            if constexpr (Standard == ColorStandard::kRec709) return ConvertRgbToGrayscaleRec709;
        }
        else if constexpr (InputFormat == Format::kRgb  && OutputFormat == Format::kCmyk)      { return ConvertRgbToCmyk; }
        else if constexpr (InputFormat == Format::kRgba && OutputFormat == Format::kRgb)       { return ConvertRgbaToRgb; }
        else if constexpr (InputFormat == Format::kRgba && OutputFormat == Format::kGrayscale) {

            if constexpr (Standard == ColorStandard::kRec601) return ConvertRgbaToGrayscaleRec601;
            if constexpr (Standard == ColorStandard::kRec709) return ConvertRgbaToGrayscaleRec709;
        }
        else if constexpr (InputFormat == Format::kRgba      && OutputFormat == Format::kCmyk)       { return ConvertRgbaToCmyk;      }
        else if constexpr (InputFormat == Format::kGrayscale && OutputFormat == Format::kRgb)        { return ConvertGrayscaleToRgb;  }
        else if constexpr (InputFormat == Format::kGrayscale && OutputFormat == Format::kRgba)       { return ConvertGrayscaleToRgba; }
        else if constexpr (InputFormat == Format::kGrayscale && OutputFormat == Format::kCmyk)       { return ConvertGrayscaleToCmyk; }
        else if constexpr (InputFormat == Format::kCmyk      && OutputFormat == Format::kRgb)        { return ConvertCmykToRgb;       }
        else if constexpr (InputFormat == Format::kCmyk      && OutputFormat == Format::kRgba)       { return ConvertCmykToRgba;      }
        else if constexpr (InputFormat == Format::kCmyk      && OutputFormat == Format::kGrayscale)  { return ConvertCmykToGrayscale; }

        return Copy;
    };
  /* clang-format on */

  constexpr Function target = function();

  target(input, output, count);
}

}  // namespace weqeqq::color::simd
