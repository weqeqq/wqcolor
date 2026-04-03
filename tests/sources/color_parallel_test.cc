#include "test_support.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace weqeqq::color {
namespace {

class ColorParallelTest
    : public ::testing::TestWithParam<testsupport::RouteCase> {};

}  // namespace

TEST_P(ColorParallelTest, SequentialAndParallelStayBitExactOnLargeBuffers) {
  constexpr std::uint8_t kSentinel = 0xC4;
  const auto route = GetParam();
  const std::array<std::size_t, 2> pixel_counts = {257, 1021};
  const std::array<Standard, 2> standards = {Standard::kBt601, Standard::kBt709};

  for (const auto pixel_count : pixel_counts) {
    for (const auto standard : standards) {
      auto input = testsupport::MakePattern(route.input, pixel_count);
      const auto output_size =
          RequiredOutputSize(input, route.input, route.output);

      std::vector<std::uint8_t> sequential(output_size + 3, kSentinel);
      std::vector<std::uint8_t> parallel(output_size + 3, kSentinel);

      Convert(input, sequential, route.input, route.output, standard,
              parallel::Execution::kSequential);
      Convert(input, parallel, route.input, route.output, standard,
              parallel::Execution::kParallel);

      SCOPED_TRACE(::testing::Message()
                   << route.name << " pixels=" << pixel_count
                   << " standard="
                   << (standard == Standard::kBt601 ? "Bt601" : "Bt709"));
      EXPECT_EQ(sequential, parallel);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    HighVolumeRoutes, ColorParallelTest,
    ::testing::ValuesIn(testsupport::kParallelRoutes),
    [](const ::testing::TestParamInfo<ColorParallelTest::ParamType>& info) {
      return info.param.name;
    });

}  // namespace weqeqq::color
