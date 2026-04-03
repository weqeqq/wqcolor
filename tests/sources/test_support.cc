#include "test_support.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace weqeqq::color::testsupport {
namespace {

struct Coeff {
  std::uint16_t r;
  std::uint16_t g;
  std::uint16_t b;
};

constexpr Coeff kBt601Coeff{76, 150, 29};
constexpr Coeff kBt709Coeff{54, 183, 18};

inline std::uint8_t Div255(std::uint32_t value) {
  return static_cast<std::uint8_t>(value * 257u >> 16);
}

inline Coeff GetCoeff(Standard standard) {
  return standard == Standard::kBt601 ? kBt601Coeff : kBt709Coeff;
}

void ConvertRgbToGrayscale(std::span<const std::uint8_t> input,
                           std::span<std::uint8_t> output,
                           Standard standard) {
  const auto coeff = GetCoeff(standard);
  const auto gray = static_cast<std::uint32_t>(input[0]) * coeff.r +
                    static_cast<std::uint32_t>(input[1]) * coeff.g +
                    static_cast<std::uint32_t>(input[2]) * coeff.b;
  output[0] = Div255(gray);
}

void ConvertRgbToCmyk(std::span<const std::uint8_t> input,
                      std::span<std::uint8_t> output) {
  const std::uint32_t r = input[0];
  const std::uint32_t g = input[1];
  const std::uint32_t b = input[2];
  const auto max_channel = std::max({r, g, b});

  if (max_channel == 0) {
    output[0] = 0;
    output[1] = 0;
    output[2] = 0;
    output[3] = 255;
    return;
  }

  output[0] = static_cast<std::uint8_t>(((max_channel - r) * 255u) / max_channel);
  output[1] = static_cast<std::uint8_t>(((max_channel - g) * 255u) / max_channel);
  output[2] = static_cast<std::uint8_t>(((max_channel - b) * 255u) / max_channel);
  output[3] = static_cast<std::uint8_t>(255u - max_channel);
}

void ConvertCmykToRgb(std::span<const std::uint8_t> input,
                      std::span<std::uint8_t> output) {
  const auto invk = 255u - input[3];
  output[0] = Div255((255u - input[0]) * invk);
  output[1] = Div255((255u - input[1]) * invk);
  output[2] = Div255((255u - input[2]) * invk);
}

void ConvertGrayscaleToCmyk(std::span<const std::uint8_t> input,
                            std::span<std::uint8_t> output) {
  output[0] = 0;
  output[1] = 0;
  output[2] = 0;
  output[3] = static_cast<std::uint8_t>(255u - input[0]);
}

void ConvertCmykToGrayscale(std::span<const std::uint8_t> input,
                            std::span<std::uint8_t> output,
                            Standard standard) {
  if (input[0] == 0 && input[1] == 0 && input[2] == 0) {
    output[0] = static_cast<std::uint8_t>(255u - input[3]);
    return;
  }

  std::array<std::uint8_t, 3> rgb{};
  ConvertCmykToRgb(input, rgb);
  ConvertRgbToGrayscale(rgb, output, standard);
}

void ConvertPixel(std::span<const std::uint8_t> input,
                  std::span<std::uint8_t> output, Format input_format,
                  Format output_format, Standard standard) {
  if (input_format == output_format) {
    std::copy_n(input.data(), ChannelCount(input_format), output.data());
    return;
  }

  switch (input_format) {
    case Format::kRgb: {
      switch (output_format) {
        case Format::kGrayscale:
          ConvertRgbToGrayscale(input, output, standard);
          return;
        case Format::kRgba:
          output[0] = input[0];
          output[1] = input[1];
          output[2] = input[2];
          output[3] = 0xff;
          return;
        case Format::kCmyk:
          ConvertRgbToCmyk(input, output);
          return;
        case Format::kRgb:
        case Format::kCount:
          break;
      }
      break;
    }
    case Format::kRgba: {
      const auto rgb = input.subspan(0, 3);
      switch (output_format) {
        case Format::kRgb:
          output[0] = input[0];
          output[1] = input[1];
          output[2] = input[2];
          return;
        case Format::kGrayscale:
          ConvertRgbToGrayscale(rgb, output, standard);
          return;
        case Format::kRgba:
          output[0] = input[0];
          output[1] = input[1];
          output[2] = input[2];
          output[3] = input[3];
          return;
        case Format::kCmyk:
          ConvertRgbToCmyk(rgb, output);
          return;
        case Format::kCount:
          break;
      }
      break;
    }
    case Format::kGrayscale: {
      switch (output_format) {
        case Format::kRgb:
          output[0] = input[0];
          output[1] = input[0];
          output[2] = input[0];
          return;
        case Format::kRgba:
          output[0] = input[0];
          output[1] = input[0];
          output[2] = input[0];
          output[3] = 0xff;
          return;
        case Format::kCmyk:
          ConvertGrayscaleToCmyk(input, output);
          return;
        case Format::kGrayscale:
          output[0] = input[0];
          return;
        case Format::kCount:
          break;
      }
      break;
    }
    case Format::kCmyk: {
      switch (output_format) {
        case Format::kRgb:
          ConvertCmykToRgb(input, output);
          return;
        case Format::kRgba:
          ConvertCmykToRgb(input, output.subspan(0, 3));
          output[3] = 0xff;
          return;
        case Format::kGrayscale:
          ConvertCmykToGrayscale(input, output, standard);
          return;
        case Format::kCmyk:
          output[0] = input[0];
          output[1] = input[1];
          output[2] = input[2];
          output[3] = input[3];
          return;
        case Format::kCount:
          break;
      }
      break;
    }
    case Format::kCount:
      break;
  }

  std::unreachable();
}

}  // namespace

std::vector<std::uint8_t> MakePattern(Format format, std::size_t pixel_count) {
  std::vector<std::uint8_t> output(pixel_count * ChannelCount(format));
  for (std::size_t i = 0; i < pixel_count; ++i) {
    switch (format) {
      case Format::kRgb:
        output[i * 3 + 0] = static_cast<std::uint8_t>((i * 37u + 13u) % 256u);
        output[i * 3 + 1] = static_cast<std::uint8_t>((i * 73u + 91u) % 256u);
        output[i * 3 + 2] = static_cast<std::uint8_t>((i * 19u + 207u) % 256u);
        break;
      case Format::kRgba:
        output[i * 4 + 0] = static_cast<std::uint8_t>((i * 37u + 13u) % 256u);
        output[i * 4 + 1] = static_cast<std::uint8_t>((i * 73u + 91u) % 256u);
        output[i * 4 + 2] = static_cast<std::uint8_t>((i * 19u + 207u) % 256u);
        output[i * 4 + 3] = static_cast<std::uint8_t>((i * 53u + 17u) % 256u);
        break;
      case Format::kGrayscale:
        output[i] = static_cast<std::uint8_t>((i * 29u + 41u) % 256u);
        break;
      case Format::kCmyk:
        output[i * 4 + 0] = static_cast<std::uint8_t>((i * 31u + 11u) % 256u);
        output[i * 4 + 1] = static_cast<std::uint8_t>((i * 47u + 67u) % 256u);
        output[i * 4 + 2] = static_cast<std::uint8_t>((i * 59u + 101u) % 256u);
        output[i * 4 + 3] = static_cast<std::uint8_t>((i * 23u + 151u) % 256u);
        break;
      case Format::kCount:
        std::unreachable();
    }
  }

  return output;
}

void ConvertReference(std::span<const std::uint8_t> input,
                      std::span<std::uint8_t> output, Format input_format,
                      Format output_format, Standard standard) {
  if (input.empty() || output.empty()) {
    return;
  }

  const auto input_channels = ChannelCount(input_format);
  const auto output_channels = ChannelCount(output_format);
  const auto input_pixels = input.size() / input_channels;
  const auto output_pixels = output.size() / output_channels;
  const auto pixel_count = std::min(input_pixels, output_pixels);

  for (std::size_t i = 0; i < pixel_count; ++i) {
    ConvertPixel(input.subspan(i * input_channels, input_channels),
                 output.subspan(i * output_channels, output_channels),
                 input_format, output_format, standard);
  }
}

std::vector<std::uint8_t> ConvertWithExecution(
    std::span<const std::uint8_t> input, Format input_format,
    Format output_format, Standard standard, parallel::Execution execution) {
  std::vector<std::uint8_t> output(
      RequiredOutputSize(input, input_format, output_format));
  Convert(input, output, input_format, output_format, standard, execution);
  return output;
}

}  // namespace weqeqq::color::testsupport
