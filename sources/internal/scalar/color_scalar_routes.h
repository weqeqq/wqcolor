#pragma once

#include "internal/scalar/color_scalar_common.h"

namespace weqeqq::color::internal {

template <Format In, Format Out, Standard Std>
void ConvertChunkScalar(const std::uint8_t* input_ptr, std::uint8_t* output_ptr,
                        std::size_t pixel_count) noexcept {
  constexpr auto kInChannels = ChannelCount(In);
  constexpr auto kOutChannels = ChannelCount(Out);

  for (std::size_t index = 0; index < pixel_count; ++index) {
    scalar_fallback::ConvertPixelColor<In, Out, Std>(
        input_ptr + index * kInChannels, output_ptr + index * kOutChannels);
  }
}

template <Format Color>
void ConvertIdentityImpl(const std::uint8_t* input_ptr,
                         std::uint8_t* output_ptr, std::size_t pixel_count,
                         parallel::ExecutionPolicy execution) noexcept {
  constexpr auto kChannels = ChannelCount(Color);
  const auto effective_execution =
      MaybePreferSequential<Color, Color>(pixel_count, execution);
  ForEachPixelChunk<Color, Color>(
      pixel_count, effective_execution,
      [&](std::size_t start, std::size_t count) {
        const auto offset = start * kChannels;
        std::memcpy(output_ptr + offset, input_ptr + offset, count * kChannels);
      });
}

template <Format In, Format Out, Standard Std>
void ConvertImplScalar(const std::uint8_t* input_ptr, std::uint8_t* output_ptr,
                       std::size_t pixel_count,
                       parallel::ExecutionPolicy execution) noexcept {
  constexpr auto kInChannels = ChannelCount(In);
  constexpr auto kOutChannels = ChannelCount(Out);

  ForEachPixelChunk<In, Out>(
      pixel_count, execution, [&](std::size_t start, std::size_t count) {
        ConvertChunkScalar<In, Out, Std>(input_ptr + start * kInChannels,
                                         output_ptr + start * kOutChannels,
                                         count);
      });
}

}  // namespace weqeqq::color::internal
