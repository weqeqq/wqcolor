#include "test_support.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace weqeqq::color {
namespace {

inline constexpr std::array<std::size_t, 12> kOddPixelCounts = {
    0, 1, 2, 3, 7, 15, 17, 31, 33, 63, 65, 127,
};

inline constexpr std::array<testsupport::RouteCase, 9> kStandardInvariantRoutes = {{
    {Format::kRgb, Format::kRgba, "RgbToRgba"},
    {Format::kRgba, Format::kRgb, "RgbaToRgb"},
    {Format::kGrayscale, Format::kRgb, "GrayscaleToRgb"},
    {Format::kGrayscale, Format::kRgba, "GrayscaleToRgba"},
    {Format::kRgb, Format::kCmyk, "RgbToCmyk"},
    {Format::kCmyk, Format::kRgb, "CmykToRgb"},
    {Format::kCmyk, Format::kRgba, "CmykToRgba"},
    {Format::kRgba, Format::kCmyk, "RgbaToCmyk"},
    {Format::kGrayscale, Format::kCmyk, "GrayscaleToCmyk"},
}};

inline constexpr std::array<testsupport::RouteCase, 3> kStandardSensitiveRoutes = {{
    {Format::kRgb, Format::kGrayscale, "RgbToGrayscale"},
    {Format::kRgba, Format::kGrayscale, "RgbaToGrayscale"},
    {Format::kCmyk, Format::kGrayscale, "CmykToGrayscale"},
}};

}  // namespace

TEST(ColorRegressionTest, TailProcessingMatchesReferenceAcrossOddSizes) {
  constexpr std::uint8_t kSentinel = 0xD3;
  const std::array<Standard, 2> standards = {
      Standard::kBt601,
      Standard::kBt709,
  };

  for (const auto route : testsupport::kTailRoutes) {
    for (const auto pixel_count : kOddPixelCounts) {
      for (const auto standard : standards) {
        auto input = testsupport::MakePattern(route.input, pixel_count);
        const auto output_size =
            RequiredOutputSize(input, route.input, route.output);

        std::vector<std::uint8_t> actual(output_size + 5, kSentinel);
        auto expected = actual;

        Convert(input, actual, route.input, route.output, standard);
        testsupport::ConvertReference(input, expected, route.input, route.output,
                                      standard);

        SCOPED_TRACE(::testing::Message()
                     << route.name << " pixels=" << pixel_count
                     << " standard="
                     << (standard == Standard::kBt601 ? "Bt601" : "Bt709"));
        EXPECT_EQ(actual, expected);
      }
    }
  }
}

TEST(ColorRegressionTest, TruncatedInputBytesMatchReferenceModel) {
  constexpr std::uint8_t kSentinel = 0xB1;
  const std::array<testsupport::RouteCase, 4> routes = {{
      {Format::kRgb, Format::kGrayscale, "RgbToGrayscale"},
      {Format::kRgba, Format::kRgb, "RgbaToRgb"},
      {Format::kRgb, Format::kRgba, "RgbToRgba"},
      {Format::kCmyk, Format::kRgb, "CmykToRgb"},
  }};

  for (const auto route : routes) {
    auto input = testsupport::MakePattern(route.input, 17);
    const auto input_channels = ChannelCount(route.input);
    for (std::size_t i = 1; i < input_channels; ++i) {
      input.push_back(static_cast<std::uint8_t>(11u * i));
    }

    const auto output_size = RequiredOutputSize(input, route.input, route.output);
    std::vector<std::uint8_t> actual(output_size + 4, kSentinel);
    auto expected = actual;

    Convert(input, actual, route.input, route.output, Standard::kBt709);
    testsupport::ConvertReference(input, expected, route.input, route.output,
                                  Standard::kBt709);

    SCOPED_TRACE(route.name);
    EXPECT_EQ(actual, expected);
  }
}

TEST(ColorRegressionTest, StandardChoiceAffectsOnlyGrayscaleProjection) {
  const std::vector<std::uint8_t> rgb_input = {
      100, 150, 50, 200, 20, 220, 33, 77, 201, 91, 12, 171};
  const std::vector<std::uint8_t> rgba_input = {
      100, 150, 50, 0, 200, 20, 220, 64, 33, 77, 201, 128, 91, 12, 171, 255};

  for (const auto route : kStandardSensitiveRoutes) {
    const auto& input = route.input == Format::kRgb ? rgb_input : rgba_input;
    auto bt601 = Convert(input, route.input, route.output, Standard::kBt601);
    auto bt709 = Convert(input, route.input, route.output, Standard::kBt709);

    SCOPED_TRACE(route.name);
    EXPECT_NE(bt601, bt709);
  }

  for (const auto route : kStandardInvariantRoutes) {
    auto input = testsupport::MakePattern(route.input, 37);
    auto bt601 = Convert(input, route.input, route.output, Standard::kBt601);
    auto bt709 = Convert(input, route.input, route.output, Standard::kBt709);

    SCOPED_TRACE(route.name);
    EXPECT_EQ(bt601, bt709);
  }
}

}  // namespace weqeqq::color
