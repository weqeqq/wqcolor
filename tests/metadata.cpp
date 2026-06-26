#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <vector>

import weqeqq.color;
import weqeqq.test;

namespace {
using namespace weqeqq::test;
namespace wc = weqeqq::color;

const Suite kMetadata("metadata", [] {
  Test("channel count", [] {
    Expect(wc::ChannelCount(wc::Format::kRgb), Eq(std::size_t{3}));
    Expect(wc::ChannelCount(wc::Format::kRgba), Eq(std::size_t{4}));
    Expect(wc::ChannelCount(wc::Format::kGrayscale), Eq(std::size_t{1}));
    Expect(wc::ChannelCount(wc::Format::kCmyk), Eq(std::size_t{4}));
    Expect(wc::ChannelCount(wc::Format::kCount), Eq(std::size_t{0}));
  });

  Test("valid format", [] {
    Expect(wc::IsValidFormat(wc::Format::kRgb), IsTrue());
    Expect(wc::IsValidFormat(wc::Format::kRgba), IsTrue());
    Expect(wc::IsValidFormat(wc::Format::kGrayscale), IsTrue());
    Expect(wc::IsValidFormat(wc::Format::kCmyk), IsTrue());
    Expect(wc::IsValidFormat(wc::Format::kCount), IsFalse());
    Expect(wc::IsValidFormat(static_cast<wc::Format>(-1)), IsFalse());
    Expect(wc::IsValidFormat(static_cast<wc::Format>(99)), IsFalse());
  });

  Test("valid standard", [] {
    Expect(wc::IsValidStandard(wc::Standard::kBt601), IsTrue());
    Expect(wc::IsValidStandard(wc::Standard::kBt709), IsTrue());
    Expect(wc::IsValidStandard(wc::Standard::kCount), IsFalse());
    Expect(wc::IsValidStandard(static_cast<wc::Standard>(-1)), IsFalse());
  });

  Test("required output size", [] {
    const std::vector<std::uint8_t> rgb(30);
    Expect(wc::RequiredOutputSize(rgb, wc::Format::kRgb, wc::Format::kRgba),
           Eq(std::size_t{40}));
    Expect(wc::RequiredOutputSize(rgb, wc::Format::kRgb, wc::Format::kGrayscale),
           Eq(std::size_t{10}));
    const std::vector<std::uint8_t> gray(10);
    Expect(wc::RequiredOutputSize(gray, wc::Format::kGrayscale,
                                  wc::Format::kCmyk),
           Eq(std::size_t{40}));
  });

  Test("required output size rejects invalid input", [] {
    const std::vector<std::uint8_t> rgb(30);
    Expect(wc::RequiredOutputSize(rgb, static_cast<wc::Format>(99),
                                  wc::Format::kRgba),
           Eq(std::size_t{0}));
    Expect(wc::RequiredOutputSize(rgb, wc::Format::kRgb,
                                  static_cast<wc::Format>(99)),
           Eq(std::size_t{0}));
    Expect(wc::RequiredOutputSize(std::span<const std::uint8_t>{},
                                  wc::Format::kRgb, wc::Format::kRgba),
           Eq(std::size_t{0}));
  });

  Test("format formatter", [] {
    Expect(std::format("{}", wc::Format::kRgb), Eq(std::string("Rgb")));
    Expect(std::format("{}", wc::Format::kRgba), Eq(std::string("Rgba")));
    Expect(std::format("{}", wc::Format::kGrayscale),
           Eq(std::string("Grayscale")));
    Expect(std::format("{}", wc::Format::kCmyk), Eq(std::string("Cmyk")));
  });

  Test("standard formatter", [] {
    Expect(std::format("{}", wc::Standard::kBt601), Eq(std::string("Bt601")));
    Expect(std::format("{}", wc::Standard::kBt709), Eq(std::string("Bt709")));
  });
});
}  // namespace
