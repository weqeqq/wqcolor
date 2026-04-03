#include "test_support.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <span>
#include <vector>

namespace weqeqq::color {

TEST(ColorContractTest, ChannelCountReportsExpectedComponentWidth) {
  EXPECT_EQ(ChannelCount(Format::kRgb), 3);
  EXPECT_EQ(ChannelCount(Format::kRgba), 4);
  EXPECT_EQ(ChannelCount(Format::kGrayscale), 1);
  EXPECT_EQ(ChannelCount(Format::kCmyk), 4);
}

TEST(ColorContractTest, RequiredOutputSizeReturnsZeroForEmptyInput) {
  std::vector<std::uint8_t> empty;
  EXPECT_EQ(RequiredOutputSize(empty, Format::kRgb, Format::kGrayscale), 0);
  EXPECT_EQ(RequiredOutputSize(empty, Format::kRgba, Format::kRgb), 0);
  EXPECT_EQ(RequiredOutputSize(empty, Format::kCmyk, Format::kRgba), 0);
}

TEST(ColorContractTest, RequiredOutputSizeDropsIncompleteInputPixels) {
  std::vector<std::uint8_t> rgb7(7);
  EXPECT_EQ(RequiredOutputSize(rgb7, Format::kRgb, Format::kGrayscale), 2);
  EXPECT_EQ(RequiredOutputSize(rgb7, Format::kRgb, Format::kRgba), 8);

  std::vector<std::uint8_t> rgba9(9);
  EXPECT_EQ(RequiredOutputSize(rgba9, Format::kRgba, Format::kRgb), 6);
}

TEST(ColorContractTest, ConvertStopsAtShortestCompletePixelRange) {
  constexpr std::uint8_t kSentinel = 0xA5;

  const auto input = testsupport::MakePattern(Format::kRgb, 3);
  std::vector<std::uint8_t> output(10, kSentinel);
  auto expected = output;

  Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601);
  testsupport::ConvertReference(input, expected, Format::kRgb, Format::kRgba,
                                Standard::kBt601);

  EXPECT_EQ(output, expected);
  EXPECT_EQ(output[8], kSentinel);
  EXPECT_EQ(output[9], kSentinel);
}

TEST(ColorContractTest, ConvertTruncatesIncompleteInputPixels) {
  constexpr std::uint8_t kSentinel = 0x7B;

  std::vector<std::uint8_t> input = {11, 22, 33, 44, 55, 66, 77};
  std::vector<std::uint8_t> output(12, kSentinel);
  auto expected = output;

  Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt709);
  testsupport::ConvertReference(input, expected, Format::kRgb, Format::kRgba,
                                Standard::kBt709);

  EXPECT_EQ(output, expected);
  EXPECT_EQ(output[8], kSentinel);
  EXPECT_EQ(output[9], kSentinel);
  EXPECT_EQ(output[10], kSentinel);
  EXPECT_EQ(output[11], kSentinel);
}

TEST(ColorContractTest, ConvertDoesNotWriteWhenOutputCannotFitPixel) {
  constexpr std::uint8_t kSentinel = 0x3C;

  std::vector<std::uint8_t> input = {1, 2, 3};
  std::vector<std::uint8_t> output(3, kSentinel);

  Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601);

  EXPECT_EQ(output[0], kSentinel);
  EXPECT_EQ(output[1], kSentinel);
  EXPECT_EQ(output[2], kSentinel);
}

TEST(ColorContractTest, ConvertLeavesOutputUntouchedForEmptyInput) {
  constexpr std::uint8_t kSentinel = 0x6E;

  std::vector<std::uint8_t> input;
  std::vector<std::uint8_t> output(5, kSentinel);

  Convert(input, output, Format::kRgb, Format::kGrayscale, Standard::kBt601);

  for (auto value : output) {
    EXPECT_EQ(value, kSentinel);
  }
}

TEST(ColorContractTest, ConvertHandlesEmptyOutputSafely) {
  const auto input = testsupport::MakePattern(Format::kRgb, 8);
  std::vector<std::uint8_t> output;

  Convert(input, output, Format::kRgb, Format::kGrayscale, Standard::kBt601);

  EXPECT_TRUE(output.empty());
}

TEST(ColorContractTest, VectorOverloadUsesPredictedOutputSize) {
  std::vector<std::uint8_t> input = {5, 10, 15, 20, 25};

  const auto predicted = RequiredOutputSize(input, Format::kRgb, Format::kRgba);
  auto actual = Convert(input, Format::kRgb, Format::kRgba, Standard::kBt601);

  ASSERT_EQ(actual.size(), predicted);

  std::vector<std::uint8_t> expected(predicted, 0);
  testsupport::ConvertReference(input, expected, Format::kRgb, Format::kRgba,
                                Standard::kBt601);
  EXPECT_EQ(actual, expected);
}

TEST(ColorContractTest, RequiredOutputSizeRejectsInvalidFormats) {
  constexpr auto kInvalidFormat = static_cast<Format>(999);
  std::vector<std::uint8_t> input = {1, 2, 3};

  if constexpr (kDebug) {
    EXPECT_THROW(
        {
          (void)RequiredOutputSize(input, kInvalidFormat, Format::kRgb);
        },
        Error);
    EXPECT_THROW(
        {
          (void)RequiredOutputSize(input, Format::kRgb, kInvalidFormat);
        },
        Error);
  } else {
    EXPECT_EQ(RequiredOutputSize(input, kInvalidFormat, Format::kRgb), 0);
    EXPECT_EQ(RequiredOutputSize(input, Format::kRgb, kInvalidFormat), 0);
  }
}

TEST(ColorContractTest, ConvertRejectsInvalidStandard) {
  constexpr auto kInvalidStandard = static_cast<Standard>(999);
  constexpr std::uint8_t kSentinel = 0x4E;

  std::vector<std::uint8_t> input = {10, 20, 30};
  std::vector<std::uint8_t> output(4, kSentinel);
  const auto expected = output;

  if constexpr (kDebug) {
    EXPECT_THROW(
        {
          Convert(input, output, Format::kRgb, Format::kRgba, kInvalidStandard);
        },
        Error);
  } else {
    Convert(input, output, Format::kRgb, Format::kRgba, kInvalidStandard);
  }

  EXPECT_EQ(output, expected);
}

TEST(ColorContractTest, ConvertRejectsInvalidFormats) {
  constexpr auto kInvalidFormat = static_cast<Format>(999);
  constexpr std::uint8_t kSentinel = 0x5F;

  std::vector<std::uint8_t> input = {10, 20, 30};
  std::vector<std::uint8_t> output(4, kSentinel);
  const auto expected = output;

  if constexpr (kDebug) {
    EXPECT_THROW(
        {
          Convert(input, output, kInvalidFormat, Format::kRgba,
                  Standard::kBt601);
        },
        Error);
    EXPECT_THROW(
        {
          Convert(input, output, Format::kRgb, kInvalidFormat,
                  Standard::kBt601);
        },
        Error);
  } else {
    Convert(input, output, kInvalidFormat, Format::kRgba, Standard::kBt601);
    Convert(input, output, Format::kRgb, kInvalidFormat, Standard::kBt601);
  }

  EXPECT_EQ(output, expected);
}

TEST(ColorContractTest, ConvertRejectsOverlappingBuffers) {
  std::vector<std::uint8_t> buffer = {1, 2, 3, 4, 5, 6, 0, 0};
  const auto expected = buffer;
  const auto input = std::span<const std::uint8_t>(buffer.data(), 6);
  auto output = std::span<std::uint8_t>(buffer.data(), 8);

  if constexpr (kDebug) {
    EXPECT_THROW(
        {
          Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601);
        },
        Error);
  } else {
    Convert(input, output, Format::kRgb, Format::kRgba, Standard::kBt601);
  }

  EXPECT_EQ(buffer, expected);
}

}  // namespace weqeqq::color
