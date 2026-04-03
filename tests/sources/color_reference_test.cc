#include "test_support.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace weqeqq::color {
namespace {

class ColorReferenceTest
    : public ::testing::TestWithParam<testsupport::RouteCase> {};

}  // namespace

TEST_P(ColorReferenceTest, LibraryMatchesIndependentReferenceAcrossVariedSizes) {
  constexpr std::uint8_t kSentinel = 0x91;
  const auto route = GetParam();
  const std::array<std::size_t, 10> pixel_counts = {1, 2, 3, 7, 15,
                                                     17, 33, 65, 127, 257};
  const std::array<Standard, 2> standards = {Standard::kBt601, Standard::kBt709};

  for (const auto pixel_count : pixel_counts) {
    for (const auto standard : standards) {
      auto input = testsupport::MakePattern(route.input, pixel_count);
      const auto output_size =
          RequiredOutputSize(input, route.input, route.output);

      std::vector<std::uint8_t> actual(output_size + 2, kSentinel);
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

TEST_P(ColorReferenceTest, LibraryMatchesReferenceWhenOutputIsTruncated) {
  constexpr std::uint8_t kSentinel = 0xE2;
  const auto route = GetParam();
  auto input = testsupport::MakePattern(route.input, 41);
  const auto full_output_size =
      RequiredOutputSize(input, route.input, route.output);
  ASSERT_GT(full_output_size, 0u);

  const auto truncated_output_size = full_output_size - 1;
  std::vector<std::uint8_t> actual(truncated_output_size + 5, kSentinel);
  auto expected = actual;

  Convert(input, actual, route.input, route.output, Standard::kBt709);
  testsupport::ConvertReference(input, expected, route.input, route.output,
                                Standard::kBt709);

  EXPECT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(
    OracleRoutes, ColorReferenceTest,
    ::testing::ValuesIn(testsupport::kReferenceRoutes),
    [](const ::testing::TestParamInfo<ColorReferenceTest::ParamType>& info) {
      return info.param.name;
    });

}  // namespace weqeqq::color
