#pragma once

#include "internal/color_chunking.h"
#include "internal/color_coefficients.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>

namespace weqeqq::color::internal {

namespace scalar_fallback {

template <std::unsigned_integral U>
inline std::uint8_t Div255(U value) {
  return static_cast<std::uint8_t>(value * 257u >> 16);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kRgba)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  output_ptr[0] = input_ptr[0];
  output_ptr[1] = input_ptr[1];
  output_ptr[2] = input_ptr[2];
  output_ptr[3] = 0xff;
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kGrayscale)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  constexpr auto coeff = Coeff<Std>{};

  const auto gray = static_cast<std::uint32_t>(input_ptr[0]) * coeff.kR +
                    static_cast<std::uint32_t>(input_ptr[1]) * coeff.kG +
                    static_cast<std::uint32_t>(input_ptr[2]) * coeff.kB;

  output_ptr[0] = Div255(gray);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kCmyk)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  const std::uint32_t r = input_ptr[0];
  const std::uint32_t g = input_ptr[1];
  const std::uint32_t b = input_ptr[2];
  const auto max_ch = std::max({r, g, b});

  if (max_ch == 0) {
    output_ptr[0] = output_ptr[1] = output_ptr[2] = 0;
    output_ptr[3] = 0xff;
    return;
  }

  output_ptr[0] = static_cast<std::uint8_t>(((max_ch - r) * 255u) / max_ch);
  output_ptr[1] = static_cast<std::uint8_t>(((max_ch - g) * 255u) / max_ch);
  output_ptr[2] = static_cast<std::uint8_t>(((max_ch - b) * 255u) / max_ch);
  output_ptr[3] = static_cast<std::uint8_t>(255u - max_ch);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgba)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  if constexpr (Out == Format::kRgb) {
    output_ptr[0] = input_ptr[0];
    output_ptr[1] = input_ptr[1];
    output_ptr[2] = input_ptr[2];
  } else {
    ConvertPixelColor<Format::kRgb, Out, Std>(input_ptr, output_ptr);
  }
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kGrayscale &&
           (Out == Format::kRgb || Out == Format::kRgba))
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  output_ptr[0] = output_ptr[1] = output_ptr[2] = input_ptr[0];
  if constexpr (Out == Format::kRgba) {
    output_ptr[3] = 0xff;
  }
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kGrayscale && Out == Format::kCmyk)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  output_ptr[0] = output_ptr[1] = output_ptr[2] = 0;
  output_ptr[3] = 255u - input_ptr[0];
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kCmyk && (Out == Format::kRgb || Out == Format::kRgba))
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  const auto invk = 255u - input_ptr[3];

  output_ptr[0] = Div255((255u - input_ptr[0]) * invk);
  output_ptr[1] = Div255((255u - input_ptr[1]) * invk);
  output_ptr[2] = Div255((255u - input_ptr[2]) * invk);
  if constexpr (Out == Format::kRgba) {
    output_ptr[3] = 0xff;
  }
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kCmyk && Out == Format::kGrayscale)
void ConvertPixelColor(const std::uint8_t* input_ptr,
                       std::uint8_t* output_ptr) {
  if (input_ptr[0] == 0 && input_ptr[1] == 0 && input_ptr[2] == 0) {
    output_ptr[0] = static_cast<std::uint8_t>(255u - input_ptr[3]);
    return;
  }

  constexpr auto coeff = Coeff<Std>{};
  std::array<std::uint8_t, 3> rgb{};

  ConvertPixelColor<Format::kCmyk, Format::kRgb, Std>(input_ptr, rgb.data());

  const auto gray = static_cast<std::uint32_t>(rgb[0]) * coeff.kR +
                    static_cast<std::uint32_t>(rgb[1]) * coeff.kG +
                    static_cast<std::uint32_t>(rgb[2]) * coeff.kB;
  output_ptr[0] = Div255(gray);
}

}  // namespace scalar_fallback

}  // namespace weqeqq::color::internal
