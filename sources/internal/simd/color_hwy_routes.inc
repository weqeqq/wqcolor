#if WQCOLOR_SIMD

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kRgba)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};

  auto r = hn::Undefined(tag);
  auto g = hn::Undefined(tag);
  auto b = hn::Undefined(tag);
  hn::LoadInterleaved3(tag, in, r, g, b);

  hn::StoreInterleaved4(r, g, b, hn::Set(tag, 0xff), tag, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kGrayscale)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Std>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved3(tag_u8, in, r_u8, g_u8, b_u8);

  ConvertRgbChannelsToGrayscale<Std>(tag_u8, tag_u16, r_u8, g_u8, b_u8,
                                     coeff_r, coeff_g, coeff_b, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgb && Out == Format::kCmyk)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved3(tag_u8, in, r_u8, g_u8, b_u8);

  ConvertRgbChannelsToCmyk(tag_u8, tag_u16, r_u8, g_u8, b_u8, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgba && Out == Format::kRgb)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};

  auto r = hn::Undefined(tag);
  auto g = hn::Undefined(tag);
  auto b = hn::Undefined(tag);
  auto a = hn::Undefined(tag);
  hn::LoadInterleaved4(tag, in, r, g, b, a);

  hn::StoreInterleaved3(r, g, b, tag, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgba && Out == Format::kGrayscale)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Std>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  auto a_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, in, r_u8, g_u8, b_u8, a_u8);

  ConvertRgbChannelsToGrayscale<Std>(tag_u8, tag_u16, r_u8, g_u8, b_u8,
                                     coeff_r, coeff_g, coeff_b, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kRgba && Out == Format::kCmyk)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto r_u8 = hn::Undefined(tag_u8);
  auto g_u8 = hn::Undefined(tag_u8);
  auto b_u8 = hn::Undefined(tag_u8);
  auto a_u8 = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, in, r_u8, g_u8, b_u8, a_u8);

  ConvertRgbChannelsToCmyk(tag_u8, tag_u16, r_u8, g_u8, b_u8, out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kGrayscale &&
           (Out == Format::kRgb || Out == Format::kRgba))
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto y = hn::LoadU(tag, in);

  if constexpr (Out == Format::kRgb) {
    hn::StoreInterleaved3(y, y, y, tag, out);
  } else {
    hn::StoreInterleaved4(y, y, y, hn::Set(tag, 0xff), tag, out);
  }
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kGrayscale && Out == Format::kCmyk)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto y = hn::LoadU(tag, in);
  const auto zero = hn::Zero(tag);

  hn::StoreInterleaved4(zero, zero, zero, hn::Sub(hn::Set(tag, 255u), y), tag,
                        out);
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kCmyk && (Out == Format::kRgb || Out == Format::kRgba))
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};

  auto c = hn::Undefined(tag_u8);
  auto m = hn::Undefined(tag_u8);
  auto y = hn::Undefined(tag_u8);
  auto k = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, in, c, m, y, k);

  const auto v255 = hn::Set(tag_u16, 255u);

  auto process_half = [&](auto promote_fn) {
    const auto ck = promote_fn(c);
    const auto mk = promote_fn(m);
    const auto yk = promote_fn(y);
    const auto kk = promote_fn(k);
    const auto invk = hn::Sub(v255, kk);

    struct {
      hn::VFromD<decltype(tag_u16)> r;
      hn::VFromD<decltype(tag_u16)> g;
      hn::VFromD<decltype(tag_u16)> b;
    } rgb;

    rgb.r = Div255(tag_u16, hn::Mul(hn::Sub(v255, ck), invk));
    rgb.g = Div255(tag_u16, hn::Mul(hn::Sub(v255, mk), invk));
    rgb.b = Div255(tag_u16, hn::Mul(hn::Sub(v255, yk), invk));
    return rgb;
  };

  auto lo =
      process_half([&](auto value) { return hn::PromoteLowerTo(tag_u16, value); });
  auto hi =
      process_half([&](auto value) { return hn::PromoteUpperTo(tag_u16, value); });

  const auto r_u8 = hn::OrderedDemote2To(tag_u8, lo.r, hi.r);
  const auto g_u8 = hn::OrderedDemote2To(tag_u8, lo.g, hi.g);
  const auto b_u8 = hn::OrderedDemote2To(tag_u8, lo.b, hi.b);

  if constexpr (Out == Format::kRgb) {
    hn::StoreInterleaved3(r_u8, g_u8, b_u8, tag_u8, out);
  } else {
    hn::StoreInterleaved4(r_u8, g_u8, b_u8, hn::Set(tag_u8, 0xff), tag_u8,
                          out);
  }
}

template <Format In, Format Out, Standard Std>
  requires(In == Format::kCmyk && Out == Format::kGrayscale)
void ConvertPixelColor(const std::uint8_t* HWY_RESTRICT in,
                       std::uint8_t* HWY_RESTRICT out) {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  constexpr auto coeff = Coeff<Std>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);

  auto c = hn::Undefined(tag_u8);
  auto m = hn::Undefined(tag_u8);
  auto y = hn::Undefined(tag_u8);
  auto k = hn::Undefined(tag_u8);
  hn::LoadInterleaved4(tag_u8, in, c, m, y, k);

  const auto v255 = hn::Set(tag_u16, 255u);

  auto process_half = [&](auto promote_fn) {
    const auto ck = promote_fn(c);
    const auto mk = promote_fn(m);
    const auto yk = promote_fn(y);
    const auto kk = promote_fn(k);
    const auto invk = hn::Sub(v255, kk);

    struct {
      hn::VFromD<decltype(tag_u16)> r;
      hn::VFromD<decltype(tag_u16)> g;
      hn::VFromD<decltype(tag_u16)> b;
    } rgb;

    rgb.r = Div255(tag_u16, hn::Mul(hn::Sub(v255, ck), invk));
    rgb.g = Div255(tag_u16, hn::Mul(hn::Sub(v255, mk), invk));
    rgb.b = Div255(tag_u16, hn::Mul(hn::Sub(v255, yk), invk));
    return rgb;
  };

  auto lo =
      process_half([&](auto value) { return hn::PromoteLowerTo(tag_u16, value); });
  auto hi =
      process_half([&](auto value) { return hn::PromoteUpperTo(tag_u16, value); });

  const auto r_u8 = hn::OrderedDemote2To(tag_u8, lo.r, hi.r);
  const auto g_u8 = hn::OrderedDemote2To(tag_u8, lo.g, hi.g);
  const auto b_u8 = hn::OrderedDemote2To(tag_u8, lo.b, hi.b);

  ConvertRgbChannelsToGrayscale<Std>(tag_u8, tag_u16, r_u8, g_u8, b_u8, coeff_r,
                                     coeff_g, coeff_b, out);

  const auto neutral =
      hn::And(hn::Eq(c, hn::Zero(tag_u8)),
              hn::And(hn::Eq(m, hn::Zero(tag_u8)), hn::Eq(y, hn::Zero(tag_u8))));
  const auto gray_from_k = hn::Sub(hn::Set(tag_u8, 255u), k);
  const auto projected_gray = hn::LoadU(tag_u8, out);
  hn::StoreU(hn::IfThenElse(neutral, gray_from_k, projected_gray), tag_u8, out);
}

template <Standard Std>
void ConvertRgbToGrayscaleChunkSimd(const std::uint8_t* input,
                                    std::uint8_t* output,
                                    std::size_t pixel_count) noexcept {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  const auto lane_count = hn::Lanes(tag_u8);
  constexpr auto coeff = Coeff<Std>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    auto r_u8 = hn::Undefined(tag_u8);
    auto g_u8 = hn::Undefined(tag_u8);
    auto b_u8 = hn::Undefined(tag_u8);
    hn::LoadInterleaved3(tag_u8, input + offset * 3, r_u8, g_u8, b_u8);
    ConvertRgbChannelsToGrayscale<Std>(tag_u8, tag_u16, r_u8, g_u8, b_u8,
                                       coeff_r, coeff_g, coeff_b,
                                       output + offset);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kRgb, Format::kGrayscale, Std>(
      input + index * 3, output + index, pixel_count - index);
}

template <Standard Std>
void ConvertRgbaToGrayscaleChunkSimd(const std::uint8_t* input,
                                     std::uint8_t* output,
                                     std::size_t pixel_count) noexcept {
  const auto tag_u8 = hn::ScalableTag<std::uint8_t>{};
  const auto tag_u16 = hn::RepartitionToWide<decltype(tag_u8)>{};
  const auto lane_count = hn::Lanes(tag_u8);
  constexpr auto coeff = Coeff<Std>{};
  const auto coeff_r = hn::Set(tag_u16, coeff.kR);
  const auto coeff_g = hn::Set(tag_u16, coeff.kG);
  const auto coeff_b = hn::Set(tag_u16, coeff.kB);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    auto r_u8 = hn::Undefined(tag_u8);
    auto g_u8 = hn::Undefined(tag_u8);
    auto b_u8 = hn::Undefined(tag_u8);
    auto a_u8 = hn::Undefined(tag_u8);
    hn::LoadInterleaved4(tag_u8, input + offset * 4, r_u8, g_u8, b_u8, a_u8);
    ConvertRgbChannelsToGrayscale<Std>(tag_u8, tag_u16, r_u8, g_u8, b_u8,
                                       coeff_r, coeff_g, coeff_b,
                                       output + offset);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kRgba, Format::kGrayscale, Std>(
      input + index * 4, output + index, pixel_count - index);
}

inline void ConvertRgbToRgbaChunkSimd(const std::uint8_t* input,
                                      std::uint8_t* output,
                                      std::size_t pixel_count) noexcept {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto lane_count = hn::Lanes(tag);
  const auto alpha = hn::Set(tag, 0xff);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    auto r = hn::Undefined(tag);
    auto g = hn::Undefined(tag);
    auto b = hn::Undefined(tag);
    hn::LoadInterleaved3(tag, input + offset * 3, r, g, b);
    hn::StoreInterleaved4(r, g, b, alpha, tag, output + offset * 4);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kRgb, Format::kRgba, Standard::kBt601>(
      input + index * 3, output + index * 4, pixel_count - index);
}

inline void ConvertRgbaToRgbChunkSimd(const std::uint8_t* input,
                                      std::uint8_t* output,
                                      std::size_t pixel_count) noexcept {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto lane_count = hn::Lanes(tag);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    auto r = hn::Undefined(tag);
    auto g = hn::Undefined(tag);
    auto b = hn::Undefined(tag);
    auto a = hn::Undefined(tag);
    hn::LoadInterleaved4(tag, input + offset * 4, r, g, b, a);
    hn::StoreInterleaved3(r, g, b, tag, output + offset * 3);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kRgba, Format::kRgb, Standard::kBt601>(
      input + index * 4, output + index * 3, pixel_count - index);
}

inline void ConvertGrayscaleToRgbChunkSimd(const std::uint8_t* input,
                                           std::uint8_t* output,
                                           std::size_t pixel_count) noexcept {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto lane_count = hn::Lanes(tag);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    const auto gray = hn::LoadU(tag, input + offset);
    hn::StoreInterleaved3(gray, gray, gray, tag, output + offset * 3);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kGrayscale, Format::kRgb, Standard::kBt601>(
      input + index, output + index * 3, pixel_count - index);
}

inline void ConvertGrayscaleToRgbaChunkSimd(const std::uint8_t* input,
                                            std::uint8_t* output,
                                            std::size_t pixel_count) noexcept {
  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto lane_count = hn::Lanes(tag);
  const auto alpha = hn::Set(tag, 0xff);
  const auto unroll_pixels = lane_count * 4;
  const auto process_block = [&](std::size_t offset) {
    const auto gray = hn::LoadU(tag, input + offset);
    hn::StoreInterleaved4(gray, gray, gray, alpha, tag, output + offset * 4);
  };

  std::size_t index = 0;
  for (; index + unroll_pixels <= pixel_count; index += unroll_pixels) {
    process_block(index);
    process_block(index + lane_count);
    process_block(index + lane_count * 2);
    process_block(index + lane_count * 3);
  }

  for (; index + lane_count <= pixel_count; index += lane_count) {
    process_block(index);
  }

  ConvertChunkScalar<Format::kGrayscale, Format::kRgba, Standard::kBt601>(
      input + index, output + index * 4, pixel_count - index);
}

template <Format In, Format Out, Standard Std>
void ConvertChunkSimd(const std::uint8_t* input, std::uint8_t* output,
                      std::size_t pixel_count) noexcept {
  constexpr auto kInChannels = ChannelCount(In);
  constexpr auto kOutChannels = ChannelCount(Out);

  const auto tag = hn::ScalableTag<std::uint8_t>{};
  const auto lane_count = hn::Lanes(tag);

  std::size_t index = 0;
  for (; index + lane_count <= pixel_count; index += lane_count) {
    ConvertPixelColor<In, Out, Std>(input + index * kInChannels,
                                    output + index * kOutChannels);
  }

  ConvertChunkScalar<In, Out, Std>(input + index * kInChannels,
                                   output + index * kOutChannels,
                                   pixel_count - index);
}

#endif  // WQCOLOR_SIMD
