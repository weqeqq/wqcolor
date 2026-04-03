template <Standard Std>
void ConvertRgbToGrayscaleImpl(const std::uint8_t* input, std::uint8_t* output,
                               std::size_t pixel_count,
                               parallel::ExecutionPolicy execution) noexcept {
  ForEachPixelChunk<Format::kRgb, Format::kGrayscale>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertRgbToGrayscaleChunkSimd<Std>(input + start * 3, output + start,
                                            count);
#else
        ConvertChunkScalar<Format::kRgb, Format::kGrayscale, Std>(
            input + start * 3, output + start, count);
#endif
      });
}

template <Standard Std>
void ConvertRgbaToGrayscaleImpl(const std::uint8_t* input,
                                std::uint8_t* output,
                                std::size_t pixel_count,
                                parallel::ExecutionPolicy execution) noexcept {
  ForEachPixelChunk<Format::kRgba, Format::kGrayscale>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertRgbaToGrayscaleChunkSimd<Std>(input + start * 4, output + start,
                                             count);
#else
        ConvertChunkScalar<Format::kRgba, Format::kGrayscale, Std>(
            input + start * 4, output + start, count);
#endif
      });
}

template <Standard Std>
void ConvertRgbToRgbaImpl(const std::uint8_t* input, std::uint8_t* output,
                          std::size_t pixel_count,
                          parallel::ExecutionPolicy execution) noexcept {
  ForEachPixelChunk<Format::kRgb, Format::kRgba>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertRgbToRgbaChunkSimd(input + start * 3, output + start * 4, count);
#else
        ConvertChunkScalar<Format::kRgb, Format::kRgba, Std>(
            input + start * 3, output + start * 4, count);
#endif
      });
}

template <Standard Std>
void ConvertRgbaToRgbImpl(const std::uint8_t* input, std::uint8_t* output,
                          std::size_t pixel_count,
                          parallel::ExecutionPolicy execution) noexcept {
  ForEachPixelChunk<Format::kRgba, Format::kRgb>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertRgbaToRgbChunkSimd(input + start * 4, output + start * 3, count);
#else
        ConvertChunkScalar<Format::kRgba, Format::kRgb, Std>(
            input + start * 4, output + start * 3, count);
#endif
      });
}

template <Standard Std>
void ConvertGrayscaleToRgbImpl(const std::uint8_t* input,
                               std::uint8_t* output,
                               std::size_t pixel_count,
                               parallel::ExecutionPolicy execution) noexcept {
  const auto effective_execution =
      MaybePreferSequential<Format::kGrayscale, Format::kRgb>(pixel_count,
                                                              execution);
  ForEachPixelChunk<Format::kGrayscale, Format::kRgb>(
      pixel_count, effective_execution,
      [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertGrayscaleToRgbChunkSimd(input + start, output + start * 3,
                                       count);
#else
        ConvertChunkScalar<Format::kGrayscale, Format::kRgb, Std>(
            input + start, output + start * 3, count);
#endif
      });
}

template <Standard Std>
void ConvertGrayscaleToRgbaImpl(const std::uint8_t* input,
                                std::uint8_t* output,
                                std::size_t pixel_count,
                                parallel::ExecutionPolicy execution) noexcept {
  const auto effective_execution =
      MaybePreferSequential<Format::kGrayscale, Format::kRgba>(pixel_count,
                                                               execution);
  ForEachPixelChunk<Format::kGrayscale, Format::kRgba>(
      pixel_count, effective_execution,
      [&](std::size_t start, std::size_t count) {
#if WQCOLOR_SIMD
        ConvertGrayscaleToRgbaChunkSimd(input + start, output + start * 4,
                                        count);
#else
        ConvertChunkScalar<Format::kGrayscale, Format::kRgba, Std>(
            input + start, output + start * 4, count);
#endif
      });
}
