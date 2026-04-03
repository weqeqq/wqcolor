#pragma once

#include <weqeqq/color.h>

#include <algorithm>
#include <cstddef>
#include <functional>

namespace weqeqq::color::internal {

inline constexpr std::size_t kDefaultMinChunkBytes = 1u << 17;
inline constexpr std::size_t kDefaultMinChunkPixelsFloor = 1u << 12;
inline constexpr std::size_t kDefaultChunksPerWorker = 4;

template <Format In, Format Out>
struct ChunkingPolicy {
  static constexpr std::size_t kMinChunkBytes = kDefaultMinChunkBytes;
  static constexpr std::size_t kMinChunkPixelsFloor =
      kDefaultMinChunkPixelsFloor;
  static constexpr std::size_t kChunksPerWorker = kDefaultChunksPerWorker;
};

template <>
struct ChunkingPolicy<Format::kRgb, Format::kRgba> {
  static constexpr std::size_t kMinChunkBytes = 1u << 18;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 13;
  static constexpr std::size_t kChunksPerWorker = 4;
};

template <>
struct ChunkingPolicy<Format::kRgba, Format::kRgb> {
  static constexpr std::size_t kMinChunkBytes = 1u << 18;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 13;
  static constexpr std::size_t kChunksPerWorker = 4;
};

template <>
struct ChunkingPolicy<Format::kGrayscale, Format::kRgb> {
  static constexpr std::size_t kMinChunkBytes = 1u << 18;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 13;
  static constexpr std::size_t kChunksPerWorker = 4;
};

template <>
struct ChunkingPolicy<Format::kGrayscale, Format::kRgba> {
  static constexpr std::size_t kMinChunkBytes = 1u << 18;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 13;
  static constexpr std::size_t kChunksPerWorker = 4;
};

template <>
struct ChunkingPolicy<Format::kCmyk, Format::kGrayscale> {
  static constexpr std::size_t kMinChunkBytes = 1u << 19;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 14;
  static constexpr std::size_t kChunksPerWorker = 2;
};

template <Format Color>
struct ChunkingPolicy<Color, Color> {
  static constexpr std::size_t kMinChunkBytes = 1u << 20;
  static constexpr std::size_t kMinChunkPixelsFloor = 1u << 15;
  static constexpr std::size_t kChunksPerWorker = 2;
};

template <typename T>
constexpr T CeilDiv(T dividend, T divisor) {
  return (dividend + divisor - 1) / divisor;
}

inline std::size_t WorkerCount(parallel::ExecutionPolicy execution) {
  if (auto* pool = std::get_if<std::reference_wrapper<parallel::ThreadPool>>(
          &execution)) {
    return std::max<std::size_t>(1, pool->get().NumThreads());
  }

  return std::get<parallel::Execution>(execution) ==
                 parallel::Execution::kParallel
             ? std::max<std::size_t>(
                   1, parallel::ThreadPool::Global().NumThreads())
             : 1;
}

template <Format In, Format Out>
inline parallel::ExecutionPolicy MaybePreferSequential(
    std::size_t pixel_count, parallel::ExecutionPolicy execution) {
  if constexpr (!((In == Format::kGrayscale && Out == Format::kRgb) ||
                  (In == Format::kGrayscale && Out == Format::kRgba) ||
                  (In == Format::kGrayscale && Out == Format::kGrayscale))) {
    return execution;
  }

  constexpr std::size_t kSequentialThresholdBytes = 16u << 20;
  constexpr std::size_t kBytesPerPixel = ChannelCount(In) + ChannelCount(Out);

  if (std::get_if<std::reference_wrapper<parallel::ThreadPool>>(&execution) !=
      nullptr) {
    return execution;
  }

  if (std::get<parallel::Execution>(execution) ==
          parallel::Execution::kParallel &&
      pixel_count * kBytesPerPixel <= kSequentialThresholdBytes) {
    return parallel::Execution::kSequential;
  }

  return execution;
}

template <Format In, Format Out>
constexpr std::size_t ChunkPixels() {
  constexpr auto kBytesPerPixel = ChannelCount(In) + ChannelCount(Out);
  return std::max<std::size_t>(
      ChunkingPolicy<In, Out>::kMinChunkPixelsFloor,
      ChunkingPolicy<In, Out>::kMinChunkBytes / kBytesPerPixel);
}

template <Format In, Format Out, typename ChunkFn>
void ForEachPixelChunk(std::size_t pixel_count,
                       parallel::ExecutionPolicy execution,
                       ChunkFn&& chunk_fn) {
  if (pixel_count == 0) {
    return;
  }

  const auto worker_count = WorkerCount(execution);
  const auto min_chunk_pixels = ChunkPixels<In, Out>();

  std::size_t chunk_count = 1;
  if (worker_count > 1 && pixel_count > min_chunk_pixels) {
    const auto max_chunks =
        worker_count * ChunkingPolicy<In, Out>::kChunksPerWorker;
    chunk_count = std::min(max_chunks, CeilDiv(pixel_count, min_chunk_pixels));
  }

  if (chunk_count <= 1) {
    chunk_fn(std::size_t{0}, pixel_count);
    return;
  }

  const auto chunk_size = CeilDiv(pixel_count, chunk_count);
  parallel::ForEachIndex(
      execution, std::ptrdiff_t{0}, static_cast<std::ptrdiff_t>(chunk_count),
      [&](std::ptrdiff_t index) {
        const auto chunk_index = static_cast<std::size_t>(index);
        const auto start = chunk_index * chunk_size;
        if (start >= pixel_count) {
          return;
        }

        const auto count = std::min(chunk_size, pixel_count - start);
        chunk_fn(start, count);
      });
}

}  // namespace weqeqq::color::internal
