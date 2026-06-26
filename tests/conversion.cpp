#include <cstddef>
#include <cstdint>
#include <vector>

import weqeqq.color;
import weqeqq.test;
import wqcolor.tests.reference;

namespace {
using namespace weqeqq::test;
namespace wc = weqeqq::color;

const std::vector<std::size_t> kPixelCounts = {
    1,  2,  3,  7,   8,   15,   16,   17,   31,   32,  33,
    63, 64, 65, 100, 255, 256,  257,  1000, 4096, 5000};

std::vector<std::uint8_t> MakeCmyk(std::size_t pixels, unsigned seed) {
  auto data = reference::RandomBytes(pixels * 4, seed);
  for (std::size_t i = 0; i < pixels && i < 8; i++) {
    data[i * 4 + 0] = data[i * 4 + 1] = data[i * 4 + 2] = 0;
  }
  return data;
}

template <class Builder, class Reference>
void CheckRoute(std::size_t input_channels, wc::Format input_format,
                wc::Format output_format, wc::Standard standard, int tolerance,
                const Builder& build_input, const Reference& reference_fn) {
  unsigned seed = 1;
  for (const std::size_t pixels : kPixelCounts) {
    const auto input = build_input(pixels, seed++);
    const auto actual =
        wc::Convert(input, input_format, output_format, standard);
    const auto expected = reference_fn(input);
    Require(actual.size(), Eq(expected.size())) << "pixels=" << pixels;
    Expect(reference::MaxDiff(actual, expected), Le(tolerance))
        << "pixels=" << pixels;
  }
}

auto RandomInput(std::size_t channels) {
  return [channels](std::size_t pixels, unsigned seed) {
    return reference::RandomBytes(pixels * channels, seed);
  };
}

const Suite kConversion("conversion", [] {
  Test("rgb -> rgb", [] {
    CheckRoute(3, wc::Format::kRgb, wc::Format::kRgb, wc::Standard::kBt709, 0,
               RandomInput(3), reference::RgbToRgb);
  });
  Test("rgb -> rgba", [] {
    CheckRoute(3, wc::Format::kRgb, wc::Format::kRgba, wc::Standard::kBt709, 0,
               RandomInput(3), reference::RgbToRgba);
  });
  Test("rgb -> grayscale (bt601)", [] {
    CheckRoute(3, wc::Format::kRgb, wc::Format::kGrayscale,
               wc::Standard::kBt601, 0, RandomInput(3),
               reference::ToGray<76, 150, 29, 3>);
  });
  Test("rgb -> grayscale (bt709)", [] {
    CheckRoute(3, wc::Format::kRgb, wc::Format::kGrayscale,
               wc::Standard::kBt709, 0, RandomInput(3),
               reference::ToGray<54, 183, 18, 3>);
  });
  Test("rgb -> cmyk", [] {
    CheckRoute(3, wc::Format::kRgb, wc::Format::kCmyk, wc::Standard::kBt709, 1,
               RandomInput(3), reference::ToCmyk<3>);
  });

  Test("rgba -> rgb", [] {
    CheckRoute(4, wc::Format::kRgba, wc::Format::kRgb, wc::Standard::kBt709, 0,
               RandomInput(4), reference::RgbaToRgb);
  });
  Test("rgba -> rgba", [] {
    CheckRoute(4, wc::Format::kRgba, wc::Format::kRgba, wc::Standard::kBt709, 0,
               RandomInput(4), reference::RgbaToRgba);
  });
  Test("rgba -> grayscale (bt601)", [] {
    CheckRoute(4, wc::Format::kRgba, wc::Format::kGrayscale,
               wc::Standard::kBt601, 0, RandomInput(4),
               reference::ToGray<76, 150, 29, 4>);
  });
  Test("rgba -> grayscale (bt709)", [] {
    CheckRoute(4, wc::Format::kRgba, wc::Format::kGrayscale,
               wc::Standard::kBt709, 0, RandomInput(4),
               reference::ToGray<54, 183, 18, 4>);
  });
  Test("rgba -> cmyk", [] {
    CheckRoute(4, wc::Format::kRgba, wc::Format::kCmyk, wc::Standard::kBt709, 1,
               RandomInput(4), reference::ToCmyk<4>);
  });

  Test("grayscale -> rgb", [] {
    CheckRoute(1, wc::Format::kGrayscale, wc::Format::kRgb,
               wc::Standard::kBt709, 0, RandomInput(1), reference::GrayToRgb);
  });
  Test("grayscale -> rgba", [] {
    CheckRoute(1, wc::Format::kGrayscale, wc::Format::kRgba,
               wc::Standard::kBt709, 0, RandomInput(1), reference::GrayToRgba);
  });
  Test("grayscale -> grayscale", [] {
    CheckRoute(1, wc::Format::kGrayscale, wc::Format::kGrayscale,
               wc::Standard::kBt709, 0, RandomInput(1), reference::GrayToGray);
  });
  Test("grayscale -> cmyk", [] {
    CheckRoute(1, wc::Format::kGrayscale, wc::Format::kCmyk,
               wc::Standard::kBt709, 0, RandomInput(1), reference::GrayToCmyk);
  });

  Test("cmyk -> rgb", [] {
    CheckRoute(4, wc::Format::kCmyk, wc::Format::kRgb, wc::Standard::kBt709, 0,
               MakeCmyk, reference::CmykToRgb);
  });
  Test("cmyk -> rgba", [] {
    CheckRoute(4, wc::Format::kCmyk, wc::Format::kRgba, wc::Standard::kBt709, 0,
               MakeCmyk, reference::CmykToRgba);
  });
  Test("cmyk -> grayscale", [] {
    CheckRoute(4, wc::Format::kCmyk, wc::Format::kGrayscale,
               wc::Standard::kBt709, 1, MakeCmyk, reference::CmykToGray);
  });
  Test("cmyk -> cmyk", [] {
    CheckRoute(4, wc::Format::kCmyk, wc::Format::kCmyk, wc::Standard::kBt709, 0,
               MakeCmyk, reference::CmykToCmyk);
  });
});
}  // namespace
