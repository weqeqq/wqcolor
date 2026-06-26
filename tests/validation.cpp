#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

import weqeqq.color;
import weqeqq.test;
import wqcolor.tests.reference;

namespace {
using namespace weqeqq::test;
namespace wc = weqeqq::color;

const Suite kValidation("validation", [] {
  Test("empty input yields empty output", [] {
    const auto result = wc::Convert(std::span<const std::uint8_t>{},
                                    wc::Format::kRgb, wc::Format::kRgba,
                                    wc::Standard::kBt709);
    Expect(result.empty(), IsTrue());
  });

  Test("invalid input format leaves output untouched", [] {
    const auto rgb = reference::RandomBytes(30, 1);
    std::vector<std::uint8_t> output(40, 0xAB);
    const auto original = output;
    wc::Convert(rgb, output, static_cast<wc::Format>(99), wc::Format::kRgba);
    Expect(output, Eq(original));
  });

  Test("invalid output format leaves output untouched", [] {
    const auto rgb = reference::RandomBytes(30, 2);
    std::vector<std::uint8_t> output(40, 0xAB);
    const auto original = output;
    wc::Convert(rgb, output, wc::Format::kRgb, static_cast<wc::Format>(99));
    Expect(output, Eq(original));
  });

  Test("invalid standard leaves output untouched", [] {
    const auto rgb = reference::RandomBytes(30, 3);
    std::vector<std::uint8_t> output(10, 0xAB);
    const auto original = output;
    wc::Convert(rgb, output, wc::Format::kRgb, wc::Format::kGrayscale,
                static_cast<wc::Standard>(99));
    Expect(output, Eq(original));
  });

  Test("overlapping spans leave output untouched", [] {
    auto buffer = reference::RandomBytes(64 * 3, 4);
    const auto original = buffer;
    const std::span<const std::uint8_t> input(buffer);
    const std::span<std::uint8_t> output(buffer);
    wc::Convert(input, output, wc::Format::kRgb, wc::Format::kRgb);
    Expect(buffer, Eq(original));
  });

  Test("undersized output converts only what fits", [] {
    const auto rgb = reference::RandomBytes(10 * 3, 5);
    std::vector<std::uint8_t> output(4 * 4, 0xAB);
    wc::Convert(rgb, output, wc::Format::kRgb, wc::Format::kRgba);

    std::vector<std::uint8_t> prefix(rgb.begin(), rgb.begin() + 4 * 3);
    Expect(output, Eq(reference::RgbToRgba(prefix)));
  });

  Test("error carries its message", [] {
    const wc::Error error("boom");
    const std::string message = error.what();
    Expect(message.find("boom") != std::string::npos, IsTrue());
  });

  Test("error is throwable and catchable", [] {
    Expect([] { throw wc::Error("bad"); }, Throws<wc::Error>());
  });

  Test("cmyk to grayscale ignores standard argument", [] {
    auto cmyk = reference::RandomBytes(2000 * 4, 6);
    for (std::size_t i = 0; i < 8; i++) {
      cmyk[i * 4 + 0] = cmyk[i * 4 + 1] = cmyk[i * 4 + 2] = 0;
    }
    const auto with_bt601 = wc::Convert(cmyk, wc::Format::kCmyk,
                                        wc::Format::kGrayscale,
                                        wc::Standard::kBt601);
    const auto with_bt709 = wc::Convert(cmyk, wc::Format::kCmyk,
                                        wc::Format::kGrayscale,
                                        wc::Standard::kBt709);
    Expect(with_bt709, Eq(with_bt601));
  });
});
}  // namespace
