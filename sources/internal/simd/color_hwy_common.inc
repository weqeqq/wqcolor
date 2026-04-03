#if WQCOLOR_SIMD

template <typename D, typename V = hn::VFromD<D>>
HWY_INLINE V Div255(D d, V v) {
  return hn::MulHigh(v, hn::Set(d, 257));
}

HWY_ALIGN inline constexpr auto kReciprocalTableQ0_16 = [] {
  std::array<std::uint32_t, 256> table{};
  table[0] = 0;
  for (std::size_t i = 1; i < table.size(); ++i) {
    table[i] = static_cast<std::uint32_t>(65535u / i);
  }
  return table;
}();

template <typename Tag, typename V = hn::VFromD<Tag>>
HWY_INLINE V RecipQ0_16(Tag tag_u16, V max_u16) {
  using T = hn::TFromD<Tag>;
  static_assert(std::is_unsigned_v<T> && sizeof(T) == 2);

  HWY_ALIGN T max_buffer[hn::MaxLanes(tag_u16)];
  HWY_ALIGN T inv_buffer[hn::MaxLanes(tag_u16)];

  const auto lane_count = hn::Lanes(tag_u16);
  hn::Store(max_u16, tag_u16, max_buffer);
  for (std::size_t lane = 0; lane < lane_count; ++lane) {
    inv_buffer[lane] = static_cast<T>(kReciprocalTableQ0_16[max_buffer[lane]]);
  }
  return hn::Load(tag_u16, inv_buffer);
}

template <typename Tag, typename V = hn::VFromD<Tag>>
HWY_INLINE V DivByMaxFast(Tag tag_u16, V diff_u16, V inv_u16) {
  return hn::MulHigh(hn::Mul(diff_u16, hn::Set(tag_u16, 255u)), inv_u16);
}

template <typename V>
struct CmyVectors {
  V c;
  V m;
  V y;
};

template <typename Tag, typename V = hn::VFromD<Tag>>
HWY_INLINE V ConvertRgbToGrayscale(Tag tag, V in_r, V in_g, V in_b, V coeff_r,
                                   V coeff_g, V coeff_b) {
  const auto r_part = hn::Mul(in_r, coeff_r);
  const auto g_part = hn::Mul(in_g, coeff_g);
  const auto b_part = hn::Mul(in_b, coeff_b);
  return Div255(tag, hn::Add(hn::Add(r_part, g_part), b_part));
}

template <typename Tag, typename V = hn::VFromD<Tag>>
HWY_INLINE CmyVectors<V> ConvertRgbToCmyFromMax(Tag tag_u16, V r, V g, V b,
                                                V mx) {
  const auto inv = RecipQ0_16(tag_u16, mx);

  return {
      .c = DivByMaxFast(tag_u16, hn::Sub(mx, r), inv),
      .m = DivByMaxFast(tag_u16, hn::Sub(mx, g), inv),
      .y = DivByMaxFast(tag_u16, hn::Sub(mx, b), inv),
  };
}

template <Standard Std, typename TagU8, typename TagU16,
          typename V8 = hn::VFromD<TagU8>>
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

  auto lo =
      ConvertRgbToCmyFromMax(tag_u16, hn::PromoteLowerTo(tag_u16, r_u8),
                             hn::PromoteLowerTo(tag_u16, g_u8),
                             hn::PromoteLowerTo(tag_u16, b_u8), max_lo_u16);
  auto hi =
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
                        hn::IfThenElse(is_zero, zero, y_u8),
                        k_u8, tag_u8, out);
}

#endif  // WQCOLOR_SIMD
