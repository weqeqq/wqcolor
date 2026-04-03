#pragma once

#include <weqeqq/color.h>
#include <weqeqq/parallel.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace weqeqq::color::testsupport {

struct RouteCase {
  Format input;
  Format output;
  const char* name;
};

inline constexpr std::array<RouteCase, 12> kParallelRoutes = {{
    {Format::kRgb, Format::kGrayscale, "RgbToGrayscale"},
    {Format::kRgba, Format::kGrayscale, "RgbaToGrayscale"},
    {Format::kRgb, Format::kRgba, "RgbToRgba"},
    {Format::kRgba, Format::kRgb, "RgbaToRgb"},
    {Format::kGrayscale, Format::kRgb, "GrayscaleToRgb"},
    {Format::kGrayscale, Format::kRgba, "GrayscaleToRgba"},
    {Format::kRgb, Format::kCmyk, "RgbToCmyk"},
    {Format::kRgba, Format::kCmyk, "RgbaToCmyk"},
    {Format::kGrayscale, Format::kCmyk, "GrayscaleToCmyk"},
    {Format::kCmyk, Format::kRgb, "CmykToRgb"},
    {Format::kCmyk, Format::kRgba, "CmykToRgba"},
    {Format::kCmyk, Format::kGrayscale, "CmykToGrayscale"},
}};

inline constexpr std::array<RouteCase, 12> kTailRoutes = {{
    {Format::kRgb, Format::kGrayscale, "RgbToGrayscale"},
    {Format::kRgba, Format::kGrayscale, "RgbaToGrayscale"},
    {Format::kRgb, Format::kRgba, "RgbToRgba"},
    {Format::kRgba, Format::kRgb, "RgbaToRgb"},
    {Format::kGrayscale, Format::kRgb, "GrayscaleToRgb"},
    {Format::kGrayscale, Format::kRgba, "GrayscaleToRgba"},
    {Format::kRgb, Format::kCmyk, "RgbToCmyk"},
    {Format::kRgba, Format::kCmyk, "RgbaToCmyk"},
    {Format::kGrayscale, Format::kCmyk, "GrayscaleToCmyk"},
    {Format::kCmyk, Format::kRgb, "CmykToRgb"},
    {Format::kCmyk, Format::kRgba, "CmykToRgba"},
    {Format::kCmyk, Format::kGrayscale, "CmykToGrayscale"},
}};

inline constexpr std::array<RouteCase, 12> kReferenceRoutes = {{
    {Format::kRgb, Format::kGrayscale, "RgbToGrayscale"},
    {Format::kRgba, Format::kGrayscale, "RgbaToGrayscale"},
    {Format::kRgb, Format::kRgba, "RgbToRgba"},
    {Format::kRgba, Format::kRgb, "RgbaToRgb"},
    {Format::kGrayscale, Format::kRgb, "GrayscaleToRgb"},
    {Format::kGrayscale, Format::kRgba, "GrayscaleToRgba"},
    {Format::kRgb, Format::kCmyk, "RgbToCmyk"},
    {Format::kRgba, Format::kCmyk, "RgbaToCmyk"},
    {Format::kGrayscale, Format::kCmyk, "GrayscaleToCmyk"},
    {Format::kCmyk, Format::kRgb, "CmykToRgb"},
    {Format::kCmyk, Format::kRgba, "CmykToRgba"},
    {Format::kCmyk, Format::kGrayscale, "CmykToGrayscale"},
}};

std::vector<std::uint8_t> MakePattern(Format format, std::size_t pixel_count);

void ConvertReference(std::span<const std::uint8_t> input,
                      std::span<std::uint8_t> output, Format input_format,
                      Format output_format, Standard standard);

std::vector<std::uint8_t> ConvertWithExecution(
    std::span<const std::uint8_t> input, Format input_format,
    Format output_format, Standard standard, parallel::Execution execution);

}  // namespace weqeqq::color::testsupport
