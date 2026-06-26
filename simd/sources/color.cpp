
#include <weqeqq/color/simd.h>

#include <algorithm>
#include <array>
#include <cstring>

#if WQCOLOR_SIMD

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "color.cpp"

#include <hwy/foreach_target.h>  // IWYU pragma: keep
#include <hwy/highway.h>

HWY_BEFORE_NAMESPACE();

#endif  // WQCOLOR_SIMD

namespace weqeqq::color::simd::internal {

#if WQCOLOR_SIMD

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

#endif  // WQCOLOR_SIMD

// ---------------------------------------------------------------------------
// Luma coefficients
// ---------------------------------------------------------------------------

template <ColorStandard Std>
struct Coeff;

template <>
struct Coeff<ColorStandard::kRec601> {
  static constexpr std::uint16_t kR = 76;
  static constexpr std::uint16_t kG = 150;
  static constexpr std::uint16_t kB = 29;
};

template <>
struct Coeff<ColorStandard::kRec709> {
  static constexpr std::uint16_t kR = 54;
  static constexpr std::uint16_t kG = 183;
  static constexpr std::uint16_t kB = 18;
};

namespace scalar {

template <std::unsigned_integral U>
inline std::uint8_t Div255(U value) {
  return static_cast<std::uint8_t>(value * 257u >> 16);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kRgba)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  output[0] = input[0];
  output[1] = input[1];
  output[2] = input[2];
  output[3] = 0xff;
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kGrayscale)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  constexpr auto coeff = Coeff<Standard>{};

  const auto gray = static_cast<std::uint32_t>(input[0]) * coeff.kR +
                    static_cast<std::uint32_t>(input[1]) * coeff.kG +
                    static_cast<std::uint32_t>(input[2]) * coeff.kB;

  output[0] = Div255(gray);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kCmyk)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  const std::uint32_t r = input[0];
  const std::uint32_t g = input[1];
  const std::uint32_t b = input[2];
  const auto max_ch = std::max({r, g, b});

  if (max_ch == 0) {
    output[0] = output[1] = output[2] = 0;
    output[3] = 0xff;
    return;
  }

  output[0] = static_cast<std::uint8_t>(((max_ch - r) * 255u) / max_ch);
  output[1] = static_cast<std::uint8_t>(((max_ch - g) * 255u) / max_ch);
  output[2] = static_cast<std::uint8_t>(((max_ch - b) * 255u) / max_ch);
  output[3] = static_cast<std::uint8_t>(255u - max_ch);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgba)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  if constexpr (OutputFormat == Format::kRgb) {
    output[0] = input[0];
    output[1] = input[1];
    output[2] = input[2];
  } else {
    ConvertPixelColor<Format::kRgb, OutputFormat, Standard>(input, output);
  }
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kGrayscale &&
           (OutputFormat == Format::kRgb || OutputFormat == Format::kRgba))
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  output[0] = output[1] = output[2] = input[0];
  if constexpr (OutputFormat == Format::kRgba) {
    output[3] = 0xff;
  }
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kGrayscale && OutputFormat == Format::kCmyk)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  output[0] = output[1] = output[2] = 0;
  output[3] = static_cast<std::uint8_t>(255u - input[0]);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kCmyk &&
           (OutputFormat == Format::kRgb || OutputFormat == Format::kRgba))
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  const auto invk = 255u - input[3];

  output[0] = Div255((255u - input[0]) * invk);
  output[1] = Div255((255u - input[1]) * invk);
  output[2] = Div255((255u - input[2]) * invk);
  if constexpr (OutputFormat == Format::kRgba) {
    output[3] = 0xff;
  }
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kCmyk && OutputFormat == Format::kGrayscale)
inline void ConvertPixelColor(const std::uint8_t* input, std::uint8_t* output) {
  if (input[0] == 0 && input[1] == 0 && input[2] == 0) {
    output[0] = static_cast<std::uint8_t>(255u - input[3]);
    return;
  }

  constexpr auto coeff = Coeff<Standard>{};
  std::array<std::uint8_t, 3> rgb{};

  ConvertPixelColor<Format::kCmyk, Format::kRgb, Standard>(input, rgb.data());

  const auto gray = static_cast<std::uint32_t>(rgb[0]) * coeff.kR +
                    static_cast<std::uint32_t>(rgb[1]) * coeff.kG +
                    static_cast<std::uint32_t>(rgb[2]) * coeff.kB;
  output[0] = Div255(gray);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
inline void ConvertColor(const std::uint8_t* input, std::uint8_t* output,
                         std::size_t count) {
  constexpr auto kInputChannels = ChannelCount(InputFormat);
  constexpr auto kOutputChannels = ChannelCount(OutputFormat);

  for (std::size_t index = 0; index < count; index++) {
    ConvertPixelColor<InputFormat, OutputFormat, Standard>(
        input + index * kInputChannels, output + index * kOutputChannels);
  }
}

}  // namespace scalar

#if WQCOLOR_SIMD

namespace simd {

template <typename Descriptor, typename Value = hn::VFromD<Descriptor>>
HWY_INLINE Value Div255(Descriptor descriptor, Value value) {
  return hn::MulHigh(value, hn::Set(descriptor, 257));
}

HWY_ALIGN inline constexpr auto kReciprocalTableQ0_16 = [] {
  std::array<std::uint32_t, 256> table{};
  table[0] = 0;
  for (std::size_t index = 1; index < table.size(); index++) {
    table[index] = static_cast<std::uint32_t>(65535u / index);
  }
  return table;
}();

template <typename Tag, typename Value = hn::VFromD<Tag>>
HWY_INLINE Value RecipQ0_16(Tag tag_u16, Value max_u16) {
  using T = hn::TFromD<Tag>;
  static_assert(std::is_unsigned_v<T> && sizeof(T) == 2);

  HWY_ALIGN T max_buffer[hn::MaxLanes(tag_u16)];
  HWY_ALIGN T inv_buffer[hn::MaxLanes(tag_u16)];

  const auto lane_count = hn::Lanes(tag_u16);
  hn::Store(max_u16, tag_u16, max_buffer);
  for (std::size_t lane = 0; lane < lane_count; lane++) {
    inv_buffer[lane] = static_cast<T>(kReciprocalTableQ0_16[max_buffer[lane]]);
  }
  return hn::Load(tag_u16, inv_buffer);
}

template <typename Tag, typename Value = hn::VFromD<Tag>>
HWY_INLINE Value DivByMaxFast(Tag tag_u16, Value diff_u16, Value inv_u16) {
  return hn::MulHigh(hn::Mul(diff_u16, hn::Set(tag_u16, 255u)), inv_u16);
}

template <typename Value>
struct CmyVectors {
  Value c;
  Value m;
  Value y;
};

template <typename Tag, typename Value = hn::VFromD<Tag>>
HWY_INLINE Value ConvertRgbToGrayscale(Tag tag, Value r, Value g, Value b,
                                       Value coeff_r, Value coeff_g,
                                       Value coeff_b) {
  const auto r_part = hn::Mul(r, coeff_r);
  const auto g_part = hn::Mul(g, coeff_g);
  const auto b_part = hn::Mul(b, coeff_b);
  return Div255(tag, hn::Add(hn::Add(r_part, g_part), b_part));
}

template <typename Tag, typename Value = hn::VFromD<Tag>>
HWY_INLINE CmyVectors<Value> ConvertRgbToCmyFromMax(Tag tag_u16, Value r,
                                                    Value g, Value b,
                                                    Value mx) {
  const auto inv = RecipQ0_16(tag_u16, mx);
  return {
      .c = DivByMaxFast(tag_u16, hn::Sub(mx, r), inv),
      .m = DivByMaxFast(tag_u16, hn::Sub(mx, g), inv),
      .y = DivByMaxFast(tag_u16, hn::Sub(mx, b), inv),
  };
}

template <typename TagU8, typename TagU16, typename V8 = hn::VFromD<TagU8>>
HWY_INLINE void ConvertRgbChannelsToGrayscale(TagU8 tag_u8, TagU16 tag_u16,
                                              V8 r_u8, V8 g_u8, V8 b_u8,
                                              hn::VFromD<TagU16> coeff_r,
                                              hn::VFromD<TagU16> coeff_g,
                                              hn::VFromD<TagU16> coeff_b,
                                              std::uint8_t* out) {
  const auto y16_lo = ConvertRgbToGrayscale(
      tag_u16, hn::PromoteLowerTo(tag_u16, r_u8),
      hn::PromoteLowerTo(tag_u16, g_u8), hn::PromoteLowerTo(tag_u16, b_u8),
      coeff_r, coeff_g, coeff_b);

  const auto y16_hi = ConvertRgbToGrayscale(
      tag_u16, hn::PromoteUpperTo(tag_u16, r_u8),
      hn::PromoteUpperTo(tag_u16, g_u8), hn::PromoteUpperTo(tag_u16, b_u8),
      coeff_r, coeff_g, coeff_b);

  hn::StoreU(hn::OrderedDemote2To(tag_u8, y16_lo, y16_hi), tag_u8, out);
}

template <typename TagU8, typename TagU16, typename V8 = hn::VFromD<TagU8>>
HWY_INLINE void ConvertRgbChannelsToCmyk(TagU8 tag_u8, TagU16 tag_u16, V8 r_u8,
                                         V8 g_u8, V8 b_u8, std::uint8_t* out) {
  const auto max_u8 = hn::Max(r_u8, hn::Max(g_u8, b_u8));
  const auto k_u8 = hn::Sub(hn::Set(tag_u8, 255u), max_u8);
  const auto max_lo_u16 = hn::PromoteLowerTo(tag_u16, max_u8);
  const auto max_hi_u16 = hn::PromoteUpperTo(tag_u16, max_u8);

  const auto lo =
      ConvertRgbToCmyFromMax(tag_u16, hn::PromoteLowerTo(tag_u16, r_u8),
                             hn::PromoteLowerTo(tag_u16, g_u8),
                             hn::PromoteLowerTo(tag_u16, b_u8), max_lo_u16);
  const auto hi =
      ConvertRgbToCmyFromMax(tag_u16, hn::PromoteUpperTo(tag_u16, r_u8),
                             hn::PromoteUpperTo(tag_u16, g_u8),
                             hn::PromoteUpperTo(tag_u16, b_u8), max_hi_u16);

  const auto c_u8 = hn::OrderedDemote2To(tag_u8, lo.c, hi.c);
  const auto m_u8 = hn::OrderedDemote2To(tag_u8, lo.m, hi.m);
  const auto y_u8 = hn::OrderedDemote2To(tag_u8, lo.y, hi.y);

  const auto is_zero = hn::Eq(max_u8, hn::Zero(tag_u8));
  const auto zero = hn::Zero(tag_u8);

  hn::StoreInterleaved4(hn::IfThenElse(is_zero, zero, c_u8),
                        hn::IfThenElse(is_zero, zero, m_u8),
                        hn::IfThenElse(is_zero, zero, y_u8), k_u8, tag_u8, out);
}

template <typename TagU8, typename TagU16, typename V8 = hn::VFromD<TagU8>>
HWY_INLINE void ConvertCmykChannelsToRgb(TagU8 tag_u8, TagU16 tag_u16, V8 c_u8,
                                         V8 m_u8, V8 y_u8, V8 k_u8, V8& r_u8,
                                         V8& g_u8, V8& b_u8) {
  const auto v255 = hn::Set(tag_u16, 255u);

  const auto c_lo = hn::PromoteLowerTo(tag_u16, c_u8);
  const auto m_lo = hn::PromoteLowerTo(tag_u16, m_u8);
  const auto y_lo = hn::PromoteLowerTo(tag_u16, y_u8);
  const auto k_lo = hn::PromoteLowerTo(tag_u16, k_u8);
  const auto invk_lo = hn::Sub(v255, k_lo);

  const auto c_hi = hn::PromoteUpperTo(tag_u16, c_u8);
  const auto m_hi = hn::PromoteUpperTo(tag_u16, m_u8);
  const auto y_hi = hn::PromoteUpperTo(tag_u16, y_u8);
  const auto k_hi = hn::PromoteUpperTo(tag_u16, k_u8);
  const auto invk_hi = hn::Sub(v255, k_hi);

  const auto r_lo = Div255(tag_u16, hn::Mul(hn::Sub(v255, c_lo), invk_lo));
  const auto g_lo = Div255(tag_u16, hn::Mul(hn::Sub(v255, m_lo), invk_lo));
  const auto b_lo = Div255(tag_u16, hn::Mul(hn::Sub(v255, y_lo), invk_lo));

  const auto r_hi = Div255(tag_u16, hn::Mul(hn::Sub(v255, c_hi), invk_hi));
  const auto g_hi = Div255(tag_u16, hn::Mul(hn::Sub(v255, m_hi), invk_hi));
  const auto b_hi = Div255(tag_u16, hn::Mul(hn::Sub(v255, y_hi), invk_hi));

  r_u8 = hn::OrderedDemote2To(tag_u8, r_lo, r_hi);
  g_u8 = hn::OrderedDemote2To(tag_u8, g_lo, g_hi);
  b_u8 = hn::OrderedDemote2To(tag_u8, b_lo, b_hi);
}

// --- per-vector pixel converters ------------------------------------------

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kRgba)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};

  auto r = hn::Undefined(tag);
  auto g = hn::Undefined(tag);
  auto b = hn::Undefined(tag);
  hn::LoadInterleaved3(tag, input, r, g, b);

  hn::StoreInterleaved4(r, g, b, hn::Set(tag, 0xff), tag, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kGrayscale)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Standard>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved3(tag_u8, input, r_u8, g_u8, b_u8);

  ConvertRgbChannelsToGrayscale(tag_u8, tag_u16, r_u8, g_u8, b_u8, coeff_r,
                                coeff_g, coeff_b, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgb && OutputFormat == Format::kCmyk)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved3(tag_u8, input, r_u8, g_u8, b_u8);

  ConvertRgbChannelsToCmyk(tag_u8, tag_u16, r_u8, g_u8, b_u8, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgba && OutputFormat == Format::kRgb)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};

  auto r = hn::Undefined(tag);
  auto g = hn::Undefined(tag);
  auto b = hn::Undefined(tag);
  auto a = hn::Undefined(tag);
  hn::LoadInterleaved4(tag, input, r, g, b, a);

  hn::StoreInterleaved3(r, g, b, tag, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgba && OutputFormat == Format::kGrayscale)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Standard>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  auto a_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, input, r_u8, g_u8, b_u8, a_u8);

  ConvertRgbChannelsToGrayscale(tag_u8, tag_u16, r_u8, g_u8, b_u8, coeff_r,
                                coeff_g, coeff_b, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kRgba && OutputFormat == Format::kCmyk)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  auto a_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, input, r_u8, g_u8, b_u8, a_u8);

  ConvertRgbChannelsToCmyk(tag_u8, tag_u16, r_u8, g_u8, b_u8, output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kGrayscale &&
           (OutputFormat == Format::kRgb || OutputFormat == Format::kRgba))
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto y = hn::LoadU(tag, input);

  if constexpr (OutputFormat == Format::kRgb) {
    hn::StoreInterleaved3(y, y, y, tag, output);
  } else {
    hn::StoreInterleaved4(y, y, y, hn::Set(tag, 0xff), tag, output);
  }
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kGrayscale && OutputFormat == Format::kCmyk)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto y = hn::LoadU(tag, input);
  const auto zero = hn::Zero(tag);

  hn::StoreInterleaved4(zero, zero, zero, hn::Sub(hn::Set(tag, 255u), y), tag,
                        output);
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kCmyk &&
           (OutputFormat == Format::kRgb || OutputFormat == Format::kRgba))
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto c = hn::Undefined(tag_u8);
  auto m = hn::Undefined(tag_u8);
  auto y = hn::Undefined(tag_u8);
  auto k = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, input, c, m, y, k);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  ConvertCmykChannelsToRgb(tag_u8, tag_u16, c, m, y, k, r_u8, g_u8, b_u8);

  if constexpr (OutputFormat == Format::kRgb) {
    hn::StoreInterleaved3(r_u8, g_u8, b_u8, tag_u8, output);
  } else {
    hn::StoreInterleaved4(r_u8, g_u8, b_u8, hn::Set(tag_u8, 0xff), tag_u8,
                          output);
  }
}

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
  requires(InputFormat == Format::kCmyk && OutputFormat == Format::kGrayscale)
HWY_INLINE void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT input,
                                  std::uint8_t* HWY_RESTRICT output) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Standard>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto c = hn::Undefined(tag_u8);
  auto m = hn::Undefined(tag_u8);
  auto y = hn::Undefined(tag_u8);
  auto k = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, input, c, m, y, k);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  ConvertCmykChannelsToRgb(tag_u8, tag_u16, c, m, y, k, r_u8, g_u8, b_u8);

  ConvertRgbChannelsToGrayscale(tag_u8, tag_u16, r_u8, g_u8, b_u8, coeff_r,
                                coeff_g, coeff_b, output);

  const auto neutral = hn::And(
      hn::Eq(c, hn::Zero(tag_u8)),
      hn::And(hn::Eq(m, hn::Zero(tag_u8)), hn::Eq(y, hn::Zero(tag_u8))));
  const auto gray_from_k = hn::Sub(hn::Set(tag_u8, 255u), k);
  const auto projected_gray = hn::LoadU(tag_u8, output);
  hn::StoreU(hn::IfThenElse(neutral, gray_from_k, projected_gray), tag_u8,
             output);
}

// --- driver ---------------------------------------------------------------

template <Format InputFormat, Format OutputFormat, ColorStandard Standard>
inline void ConvertColor(const std::uint8_t* input, std::uint8_t* output,
                         std::size_t count) {
  constexpr auto kInputChannels = ChannelCount(InputFormat);
  constexpr auto kOutputChannels = ChannelCount(OutputFormat);

  const auto tag = hn::ScalableTag<std::uint8_t>();
  const std::size_t lane_count = hn::Lanes(tag);
  const std::size_t unroll = lane_count * 4;

  const auto block = [&](std::size_t at) {
    ConvertPixelColor<InputFormat, OutputFormat, Standard>(
        input + at * kInputChannels, output + at * kOutputChannels);
  };

  std::size_t index = 0;
  for (; index + unroll <= count; index += unroll) {
    block(index);
    block(index + lane_count);
    block(index + lane_count * 2);
    block(index + lane_count * 3);
  }
  for (; index + lane_count <= count; index += lane_count) {
    block(index);
  }
  for (; index < count; index++) {
    scalar::ConvertPixelColor<InputFormat, OutputFormat, Standard>(
        input + index * kInputChannels, output + index * kOutputChannels);
  }
}

}  // namespace simd

#endif  // WQCOLOR_SIMD

#if WQCOLOR_SIMD
namespace impl = simd;
#else
namespace impl = scalar;
#endif  // WQCOLOR_SIMD

void ConvertRgbToRgba(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  impl::ConvertColor<Format::kRgb, Format::kRgba, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertRgbToGrayscaleRec601(const std::uint8_t* input,
                                 std::uint8_t* output, std::size_t count) {
  impl::ConvertColor<Format::kRgb, Format::kGrayscale, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertRgbToGrayscaleRec709(const std::uint8_t* input,
                                 std::uint8_t* output, std::size_t count) {
  impl::ConvertColor<Format::kRgb, Format::kGrayscale, ColorStandard::kRec709>(
      input, output, count);
}
void ConvertRgbToCmyk(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  impl::ConvertColor<Format::kRgb, Format::kCmyk, ColorStandard::kRec601>(
      input, output, count);
}

void ConvertRgbaToRgb(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  impl::ConvertColor<Format::kRgba, Format::kRgb, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertRgbaToGrayscaleRec601(const std::uint8_t* input,
                                  std::uint8_t* output, std::size_t count) {
  impl::ConvertColor<Format::kRgba, Format::kGrayscale, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertRgbaToGrayscaleRec709(const std::uint8_t* input,
                                  std::uint8_t* output, std::size_t count) {
  impl::ConvertColor<Format::kRgba, Format::kGrayscale, ColorStandard::kRec709>(
      input, output, count);
}
void ConvertRgbaToCmyk(const std::uint8_t* input, std::uint8_t* output,
                       std::size_t count) {
  impl::ConvertColor<Format::kRgba, Format::kCmyk, ColorStandard::kRec601>(
      input, output, count);
}

void ConvertGrayscaleToRgb(const std::uint8_t* input, std::uint8_t* output,
                           std::size_t count) {
  impl::ConvertColor<Format::kGrayscale, Format::kRgb, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertGrayscaleToRgba(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  impl::ConvertColor<Format::kGrayscale, Format::kRgba, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertGrayscaleToCmyk(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  impl::ConvertColor<Format::kGrayscale, Format::kCmyk, ColorStandard::kRec601>(
      input, output, count);
}

void ConvertCmykToRgb(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  impl::ConvertColor<Format::kCmyk, Format::kRgb, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertCmykToRgba(const std::uint8_t* input, std::uint8_t* output,
                       std::size_t count) {
  impl::ConvertColor<Format::kCmyk, Format::kRgba, ColorStandard::kRec601>(
      input, output, count);
}
void ConvertCmykToGrayscale(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  impl::ConvertColor<Format::kCmyk, Format::kGrayscale, ColorStandard::kRec601>(
      input, output, count);
}

#if WQCOLOR_SIMD
}  // namespace HWY_NAMESPACE
#endif  // WQCOLOR_SIMD

}  // namespace weqeqq::color::simd::internal

#if WQCOLOR_SIMD
HWY_AFTER_NAMESPACE();
#endif  // WQCOLOR_SIMD

// ===========================================================================
// Public entry points
// ===========================================================================

#if !WQCOLOR_SIMD || HWY_ONCE

namespace weqeqq::color::simd {

#if WQCOLOR_SIMD

namespace internal {
HWY_EXPORT(ConvertRgbToRgba);
HWY_EXPORT(ConvertRgbToGrayscaleRec601);
HWY_EXPORT(ConvertRgbToGrayscaleRec709);
HWY_EXPORT(ConvertRgbToCmyk);

HWY_EXPORT(ConvertRgbaToRgb);
HWY_EXPORT(ConvertRgbaToGrayscaleRec601);
HWY_EXPORT(ConvertRgbaToGrayscaleRec709);
HWY_EXPORT(ConvertRgbaToCmyk);

HWY_EXPORT(ConvertGrayscaleToRgb);
HWY_EXPORT(ConvertGrayscaleToRgba);
HWY_EXPORT(ConvertGrayscaleToCmyk);

HWY_EXPORT(ConvertCmykToRgb);
HWY_EXPORT(ConvertCmykToRgba);
HWY_EXPORT(ConvertCmykToGrayscale);
}  // namespace internal

#define WQCOLOR_DISPATCH(Function) HWY_DYNAMIC_DISPATCH(internal::Function)

#else

#define WQCOLOR_DISPATCH(Function) internal::Function

#endif  // WQCOLOR_SIMD

#ifndef __CLANGD__

void ConvertRgbToRgba(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbToRgba)(input, output, count);
}
void ConvertRgbToGrayscaleRec601(const std::uint8_t* input,
                                 std::uint8_t* output, std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbToGrayscaleRec601)(input, output, count);
}
void ConvertRgbToGrayscaleRec709(const std::uint8_t* input,
                                 std::uint8_t* output, std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbToGrayscaleRec709)(input, output, count);
}
void ConvertRgbToCmyk(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbToCmyk)(input, output, count);
}

void ConvertRgbaToRgb(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbaToRgb)(input, output, count);
}
void ConvertRgbaToGrayscaleRec601(const std::uint8_t* input,
                                  std::uint8_t* output, std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbaToGrayscaleRec601)(input, output, count);
}
void ConvertRgbaToGrayscaleRec709(const std::uint8_t* input,
                                  std::uint8_t* output, std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbaToGrayscaleRec709)(input, output, count);
}
void ConvertRgbaToCmyk(const std::uint8_t* input, std::uint8_t* output,
                       std::size_t count) {
  WQCOLOR_DISPATCH(ConvertRgbaToCmyk)(input, output, count);
}

void ConvertGrayscaleToRgb(const std::uint8_t* input, std::uint8_t* output,
                           std::size_t count) {
  WQCOLOR_DISPATCH(ConvertGrayscaleToRgb)(input, output, count);
}
void ConvertGrayscaleToRgba(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  WQCOLOR_DISPATCH(ConvertGrayscaleToRgba)(input, output, count);
}
void ConvertGrayscaleToCmyk(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  WQCOLOR_DISPATCH(ConvertGrayscaleToCmyk)(input, output, count);
}

void ConvertCmykToRgb(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t count) {
  WQCOLOR_DISPATCH(ConvertCmykToRgb)(input, output, count);
}
void ConvertCmykToRgba(const std::uint8_t* input, std::uint8_t* output,
                       std::size_t count) {
  WQCOLOR_DISPATCH(ConvertCmykToRgba)(input, output, count);
}
void ConvertCmykToGrayscale(const std::uint8_t* input, std::uint8_t* output,
                            std::size_t count) {
  WQCOLOR_DISPATCH(ConvertCmykToGrayscale)(input, output, count);
}

void Copy(const std::uint8_t* input, std::uint8_t* output, std::size_t count) {
  std::memcpy(output, input, count);
}

#endif  // __CLANGD__

#undef WQCOLOR_DISPATCH

}  // namespace weqeqq::color::simd

#endif  // !WQCOLOR_SIMD || HWY_ONCE
