#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

import weqeqq.color;
import weqeqq.parallel;
import weqeqq.test;
import wqcolor.tests.reference;

namespace {
using namespace weqeqq::test;
namespace wc = weqeqq::color;
namespace wp = weqeqq::parallel;

constexpr std::size_t kLargePixels = 300000;

std::vector<std::uint8_t> ConvertWith(const std::vector<std::uint8_t>& input,
                                      wc::Format input_format,
                                      wc::Format output_format,
                                      wp::ExecutionPolicy execution) {
  return wc::Convert(input, input_format, output_format, wc::Standard::kBt709,
                     execution);
}

const Suite kExecution("execution", [] {
  Test("parallel matches sequential", [] {
    const auto rgb = reference::RandomBytes(kLargePixels * 3, 11);
    const auto sequential =
        ConvertWith(rgb, wc::Format::kRgb, wc::Format::kCmyk,
                    wp::Execution::kSequential);
    const auto parallel = ConvertWith(rgb, wc::Format::kRgb, wc::Format::kCmyk,
                                      wp::Execution::kParallel);
    Expect(parallel, Eq(sequential));
  });

  Test("thread pool matches sequential", [] {
    const auto rgb = reference::RandomBytes(kLargePixels * 3, 22);
    const auto sequential =
        ConvertWith(rgb, wc::Format::kRgb, wc::Format::kRgba,
                    wp::Execution::kSequential);
    wp::ThreadPool pool(4);
    const auto pooled = ConvertWith(rgb, wc::Format::kRgb, wc::Format::kRgba,
                                    std::ref(pool));
    Expect(pooled, Eq(sequential));
  });

  Test("parallel identity matches sequential", [] {
    const auto rgba = reference::RandomBytes(kLargePixels * 4, 33);
    const auto sequential =
        ConvertWith(rgba, wc::Format::kRgba, wc::Format::kRgba,
                    wp::Execution::kSequential);
    const auto parallel =
        ConvertWith(rgba, wc::Format::kRgba, wc::Format::kRgba,
                    wp::Execution::kParallel);
    Expect(parallel, Eq(rgba));
    Expect(sequential, Eq(rgba));
  });

  Test("span overload writes full output", [] {
    const auto rgb = reference::RandomBytes(kLargePixels * 3, 44);
    std::vector<std::uint8_t> output(kLargePixels * 4, 0xAB);
    wc::Convert(rgb, output, wc::Format::kRgb, wc::Format::kRgba,
                wc::Standard::kBt709, wp::Execution::kParallel);
    Expect(output, Eq(reference::RgbToRgba(rgb)));
  });

  Test("round trip rgb -> rgba -> rgb", [] {
    const auto rgb = reference::RandomBytes(kLargePixels * 3, 55);
    const auto rgba = ConvertWith(rgb, wc::Format::kRgb, wc::Format::kRgba,
                                  wp::Execution::kParallel);
    const auto back = ConvertWith(rgba, wc::Format::kRgba, wc::Format::kRgb,
                                  wp::Execution::kParallel);
    Expect(back, Eq(rgb));
  });
});
}  // namespace
