#include "test_support.h"

#include <gtest/gtest.h>

#include <cmath>
#include <array>
#include <cstdint>
#include <vector>

namespace weqeqq::color {

TEST(ColorCorrectnessTest, RgbToGrayscaleBt601MatchesCanonicalPrimaries) {
  struct Sample {
    std::array<std::uint8_t, 3> rgb;
    std::uint8_t expected;
  };

  const std::array<Sample, 4> samples{{
      {{255, 0, 0}, 76},
      {{0, 255, 0}, 150},
      {{0, 0, 255}, 29},
      {{255, 255, 255}, 255},
  }};

  for (const auto& sample : samples) {
    std::vector<std::uint8_t> output(1, 0);
    Convert(sample.rgb, output, Format::kRgb, Format::kGrayscale,
            Standard::kBt601);
    EXPECT_LE(std::abs(static_cast<int>(output[0]) -
                       static_cast<int>(sample.expected)),
              1);
  }
}

TEST(ColorCorrectnessTest, RgbToGrayscaleBt709MatchesCanonicalPrimaries) {
  struct Sample {
    std::array<std::uint8_t, 3> rgb;
    std::uint8_t expected;
  };

  const std::array<Sample, 4> samples{{
      {{255, 0, 0}, 54},
      {{0, 255, 0}, 182},
      {{0, 0, 255}, 18},
      {{255, 255, 255}, 255},
  }};

  for (const auto& sample : samples) {
    std::vector<std::uint8_t> output(1, 0);
    Convert(sample.rgb, output, Format::kRgb, Format::kGrayscale,
            Standard::kBt709);
    EXPECT_LE(std::abs(static_cast<int>(output[0]) -
                       static_cast<int>(sample.expected)),
              1);
  }
}

TEST(ColorCorrectnessTest, RgbaToGrayscaleIgnoresAlphaChannel) {
  std::vector<std::uint8_t> low_alpha = {100, 150, 50, 0};
  std::vector<std::uint8_t> high_alpha = {100, 150, 50, 255};
  std::vector<std::uint8_t> low_output(1, 0);
  std::vector<std::uint8_t> high_output(1, 0);

  Convert(low_alpha, low_output, Format::kRgba, Format::kGrayscale,
          Standard::kBt709);
  Convert(high_alpha, high_output, Format::kRgba, Format::kGrayscale,
          Standard::kBt709);

  EXPECT_EQ(low_output, high_output);
}

TEST(ColorCorrectnessTest, RgbToRgbaInjectsOpaqueAlpha) {
  std::vector<std::uint8_t> input = {10, 20, 30, 40, 50, 60};
  std::vector<std::uint8_t> output(8, 0);

  Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601);

  EXPECT_EQ(output[0], 10);
  EXPECT_EQ(output[1], 20);
  EXPECT_EQ(output[2], 30);
  EXPECT_EQ(output[3], 255);
  EXPECT_EQ(output[4], 40);
  EXPECT_EQ(output[5], 50);
  EXPECT_EQ(output[6], 60);
  EXPECT_EQ(output[7], 255);
}

TEST(ColorCorrectnessTest, RgbaToRgbDropsAlpha) {
  std::vector<std::uint8_t> input = {10, 20, 30, 200, 40, 50, 60, 7};
  std::vector<std::uint8_t> output(6, 0);

  Convert(input, output, Format::kRgba, Format::kRgb, Standard::kBt601);

  EXPECT_EQ(output[0], 10);
  EXPECT_EQ(output[1], 20);
  EXPECT_EQ(output[2], 30);
  EXPECT_EQ(output[3], 40);
  EXPECT_EQ(output[4], 50);
  EXPECT_EQ(output[5], 60);
}

TEST(ColorCorrectnessTest, RgbToCmykEncodesPrimaryColors) {
  {
    std::vector<std::uint8_t> input = {255, 0, 0};
    std::vector<std::uint8_t> output(4, 0);
    Convert(input, output, Format::kRgb, Format::kCmyk, Standard::kBt601);
    EXPECT_EQ(output[0], 0);
    EXPECT_EQ(output[1], 255);
    EXPECT_EQ(output[2], 255);
    EXPECT_EQ(output[3], 0);
  }

  {
    std::vector<std::uint8_t> input = {0, 255, 0};
    std::vector<std::uint8_t> output(4, 0);
    Convert(input, output, Format::kRgb, Format::kCmyk, Standard::kBt601);
    EXPECT_EQ(output[0], 255);
    EXPECT_EQ(output[1], 0);
    EXPECT_EQ(output[2], 255);
    EXPECT_EQ(output[3], 0);
  }
}

TEST(ColorCorrectnessTest, RgbToCmykEncodesBlackAsFullK) {
  std::vector<std::uint8_t> input = {0, 0, 0};
  std::vector<std::uint8_t> output(4, 0);

  Convert(input, output, Format::kRgb, Format::kCmyk, Standard::kBt601);

  EXPECT_EQ(output[0], 0);
  EXPECT_EQ(output[1], 0);
  EXPECT_EQ(output[2], 0);
  EXPECT_EQ(output[3], 255);
}

TEST(ColorCorrectnessTest, CmykToRgbDecodesPrimaryColors) {
  {
    std::vector<std::uint8_t> input = {0, 255, 255, 0};
    std::vector<std::uint8_t> output(3, 0);
    Convert(input, output, Format::kCmyk, Format::kRgb, Standard::kBt601);
    EXPECT_EQ(output[0], 254);
    EXPECT_EQ(output[1], 0);
    EXPECT_EQ(output[2], 0);
  }

  {
    std::vector<std::uint8_t> input = {255, 255, 0, 0};
    std::vector<std::uint8_t> output(3, 0);
    Convert(input, output, Format::kCmyk, Format::kRgb, Standard::kBt601);
    EXPECT_EQ(output[0], 0);
    EXPECT_EQ(output[1], 0);
    EXPECT_EQ(output[2], 254);
  }
}

TEST(ColorCorrectnessTest, GrayscaleToCmykUsesNeutralBlackOnlyEncoding) {
  std::vector<std::uint8_t> input = {255, 128, 0};
  std::vector<std::uint8_t> output(12, 0);

  Convert(input, output, Format::kGrayscale, Format::kCmyk, Standard::kBt709);

  EXPECT_EQ(output[0], 0);
  EXPECT_EQ(output[1], 0);
  EXPECT_EQ(output[2], 0);
  EXPECT_EQ(output[3], 0);

  EXPECT_EQ(output[4], 0);
  EXPECT_EQ(output[5], 0);
  EXPECT_EQ(output[6], 0);
  EXPECT_EQ(output[7], 127);

  EXPECT_EQ(output[8], 0);
  EXPECT_EQ(output[9], 0);
  EXPECT_EQ(output[10], 0);
  EXPECT_EQ(output[11], 255);
}

TEST(ColorCorrectnessTest, CmykToGrayscaleAccountsForChromaticChannels) {
  std::vector<std::uint8_t> input = {0, 255, 255, 0};
  std::vector<std::uint8_t> output(1, 0);

  Convert(input, output, Format::kCmyk, Format::kGrayscale, Standard::kBt601);

  EXPECT_EQ(output[0], 75);
}

TEST(ColorCorrectnessTest, IdentityConversionPreservesEachFormat) {
  const std::array<Format, 4> formats = {
      Format::kRgb,
      Format::kRgba,
      Format::kGrayscale,
      Format::kCmyk,
  };

  for (const auto format : formats) {
    auto input = testsupport::MakePattern(format, 23);
    std::vector<std::uint8_t> output(input.size(), 0);
    Convert(input, output, format, format, Standard::kBt709);
    EXPECT_EQ(output, input);
  }
}

TEST(ColorCorrectnessTest, RoundTripRgbToRgbaToRgbPreservesColors) {
  const auto input = testsupport::MakePattern(Format::kRgb, 91);

  const auto rgba =
      Convert(input, Format::kRgb, Format::kRgba, Standard::kBt601);
  const auto rgb =
      Convert(rgba, Format::kRgba, Format::kRgb, Standard::kBt601);

  EXPECT_EQ(rgb, input);
}

TEST(ColorCorrectnessTest, RoundTripRgbBlackToCmykToRgbPreservesBlack) {
  const std::vector<std::uint8_t> input = {0, 0, 0};

  const auto cmyk =
      Convert(input, Format::kRgb, Format::kCmyk, Standard::kBt601);
  const auto rgb =
      Convert(cmyk, Format::kCmyk, Format::kRgb, Standard::kBt601);

  EXPECT_EQ(rgb, input);
}

TEST(ColorCorrectnessTest, RoundTripGrayToRgbToGrayPreservesSamples) {
  const auto input = testsupport::MakePattern(Format::kGrayscale, 91);

  const auto rgb =
      Convert(input, Format::kGrayscale, Format::kRgb, Standard::kBt709);
  const auto gray =
      Convert(rgb, Format::kRgb, Format::kGrayscale, Standard::kBt709);

  ASSERT_EQ(gray.size(), input.size());
  for (std::size_t i = 0; i < gray.size(); ++i) {
    EXPECT_LE(std::abs(static_cast<int>(gray[i]) - static_cast<int>(input[i])),
              1);
  }
}

TEST(ColorCorrectnessTest, RoundTripGrayToCmykToGrayPreservesSamples) {
  const auto input = testsupport::MakePattern(Format::kGrayscale, 91);

  const auto cmyk =
      Convert(input, Format::kGrayscale, Format::kCmyk, Standard::kBt709);
  const auto gray =
      Convert(cmyk, Format::kCmyk, Format::kGrayscale, Standard::kBt709);

  EXPECT_EQ(gray, input);
}

TEST(ColorCorrectnessTest, RoundTripGrayToRgbaToGrayPreservesSamples) {
  const auto input = testsupport::MakePattern(Format::kGrayscale, 91);

  const auto rgba =
      Convert(input, Format::kGrayscale, Format::kRgba, Standard::kBt709);
  const auto gray =
      Convert(rgba, Format::kRgba, Format::kGrayscale, Standard::kBt709);

  ASSERT_EQ(gray.size(), input.size());
  for (std::size_t i = 0; i < gray.size(); ++i) {
    EXPECT_LE(std::abs(static_cast<int>(gray[i]) - static_cast<int>(input[i])),
              1);
  }
}

TEST(ColorCorrectnessTest, RoundTripRgbaToRgbToRgbaPreservesRgbAndNormalizesAlpha) {
  const auto input = testsupport::MakePattern(Format::kRgba, 91);

  const auto rgb =
      Convert(input, Format::kRgba, Format::kRgb, Standard::kBt601);
  const auto rgba =
      Convert(rgb, Format::kRgb, Format::kRgba, Standard::kBt601);

  ASSERT_EQ(rgba.size(), input.size());
  for (std::size_t i = 0; i < 91; ++i) {
    EXPECT_EQ(rgba[i * 4], input[i * 4]);
    EXPECT_EQ(rgba[i * 4 + 1], input[i * 4 + 1]);
    EXPECT_EQ(rgba[i * 4 + 2], input[i * 4 + 2]);
    EXPECT_EQ(rgba[i * 4 + 3], 255);
  }
}

}  // namespace weqeqq::color
