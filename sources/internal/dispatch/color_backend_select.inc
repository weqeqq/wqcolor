#if WQCOLOR_SIMD
template <Format In, Format Out, Standard Std>
void ConvertImplSimd(const std::uint8_t* input, std::uint8_t* output,
                     std::size_t pixel_count,
                     parallel::ExecutionPolicy execution) noexcept {
  constexpr auto kInChannels = ChannelCount(In);
  constexpr auto kOutChannels = ChannelCount(Out);

  ForEachPixelChunk<In, Out>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
        ConvertChunkSimd<In, Out, Std>(input + start * kInChannels,
                                       output + start * kOutChannels, count);
      });
}
#endif

template <Format In, Format Out, Standard Std>
constexpr ConvertFn SelectGenericBackendFn() {
#if WQCOLOR_SIMD
  if constexpr (RouteTraits<In, Out>::kHasSimd) {
    return &ConvertImplSimd<In, Out, Std>;
  } else {
    return &ConvertImplScalar<In, Out, Std>;
  }
#else
  return &ConvertImplScalar<In, Out, Std>;
#endif
}

template <Format In, Format Out, Standard Std>
constexpr ConvertFn MakeConvertFn() {
  constexpr auto kind = RouteTraits<In, Out>::kKind;

  if constexpr (kind == RouteKind::kIdentity) {
    return &ConvertIdentityImpl<In>;
  } else if constexpr (kind == RouteKind::kRgbToGrayscale) {
    return &ConvertRgbToGrayscaleImpl<Std>;
  } else if constexpr (kind == RouteKind::kRgbaToGrayscale) {
    return &ConvertRgbaToGrayscaleImpl<Std>;
  } else if constexpr (kind == RouteKind::kRgbToRgba) {
    return &ConvertRgbToRgbaImpl<Std>;
  } else if constexpr (kind == RouteKind::kRgbaToRgb) {
    return &ConvertRgbaToRgbImpl<Std>;
  } else if constexpr (kind == RouteKind::kGrayscaleToRgb) {
    return &ConvertGrayscaleToRgbImpl<Std>;
  } else if constexpr (kind == RouteKind::kGrayscaleToRgba) {
    return &ConvertGrayscaleToRgbaImpl<Std>;
  } else {
    static_assert(kind == RouteKind::kGeneric);
    return SelectGenericBackendFn<In, Out, Std>();
  }
}
