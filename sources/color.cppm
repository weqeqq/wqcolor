module;

#include <weqeqq/color/simd.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <optional>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module weqeqq.color;

import weqeqq.parallel;

namespace weqeqq::color {

export struct Error : std::runtime_error {
  Error(std::string_view message,
        std::source_location location = std::source_location::current())
      : std::runtime_error(std::format("[{}:{}] {}", location.file_name(),
                                       location.line(), message)) {}
};

export enum class Format {
  kRgb,
  kRgba,
  kGrayscale,
  kCmyk,
  kCount,
};

export enum class Standard {
  kBt601,
  kBt709,
  kCount,
};

export [[nodiscard]] inline constexpr bool IsValidFormat(
    Format format) noexcept {
  return std::to_underlying(format) >= 0 &&
         std::to_underlying(format) < std::to_underlying(Format::kCount);
}

export [[nodiscard]] inline constexpr bool IsValidStandard(
    Standard standard) noexcept {
  return std::to_underlying(standard) >= 0 &&
         std::to_underlying(standard) < std::to_underlying(Standard::kCount);
}

export inline constexpr std::size_t ChannelCount(Format color) {
  switch (color) {
    case Format::kRgb:
      return 3;
    case Format::kRgba:
      return 4;
    case Format::kGrayscale:
      return 1;
    case Format::kCmyk:
      return 4;
    default:
      break;
  }
  return 0;
}

export inline std::size_t RequiredOutputSize(
    std::span<const std::uint8_t> input, Format input_color,
    Format output_color) {
  if (!IsValidFormat(input_color) || !IsValidFormat(output_color) ||
      input.empty()) {
    return 0;
  }
  return input.size() / ChannelCount(input_color) * ChannelCount(output_color);
}

export void Convert(std::span<const std::uint8_t> input,
                    std::span<std::uint8_t> output, Format input_color,
                    Format output_color, Standard standard = Standard::kBt709,
                    parallel::ExecutionPolicy execution =
                        parallel::Execution::kSequential) noexcept;

export inline std::vector<std::uint8_t> Convert(
    std::span<const std::uint8_t> input, Format input_color,
    Format output_color, Standard standard,
    parallel::ExecutionPolicy execution = parallel::Execution::kSequential) {
  std::vector<std::uint8_t> output(
      RequiredOutputSize(input, input_color, output_color));
  Convert(input, output, input_color, output_color, standard, execution);
  return output;
}

namespace internal {

inline constexpr std::size_t kFormatCount =
    static_cast<std::size_t>(Format::kCount);
inline constexpr std::size_t kStandardCount =
    static_cast<std::size_t>(Standard::kCount);
inline constexpr std::size_t kDispatchTableSize =
    kFormatCount * kFormatCount * kStandardCount;

template <typename Enum>
constexpr std::size_t EnumIndex(Enum value) noexcept {
  return static_cast<std::size_t>(value);
}

using ConvertFn = void (*)(const std::uint8_t* input, std::uint8_t* output,
                           std::size_t pixel_count,
                           parallel::ExecutionPolicy execution);

struct ConvertContext {
  const std::uint8_t* input;
  std::uint8_t* output;
  std::size_t pixel_count;
};

// --- validation -----------------------------------------------------------

inline bool ContractViolation(
    std::string_view message,
    std::source_location location = std::source_location::current()) noexcept {
  return false;
}

inline bool SpansOverlap(std::span<const std::uint8_t> input,
                         std::span<std::uint8_t> output) noexcept {
  if (input.empty() || output.empty()) {
    return false;
  }
  const auto input_address = reinterpret_cast<std::uintptr_t>(input.data());
  const auto output_address = reinterpret_cast<std::uintptr_t>(output.data());
  if (input_address <= output_address) {
    return output_address - input_address < input.size();
  }
  return input_address - output_address < output.size();
}

inline bool ValidateConvertArguments(
    std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
    Format input_color, Format output_color, Standard standard,
    std::source_location location = std::source_location::current()) noexcept {
  if (!IsValidFormat(input_color)) {
    return ContractViolation("input format is invalid", location);
  }
  if (!IsValidFormat(output_color)) {
    return ContractViolation("output format is invalid", location);
  }
  if (!IsValidStandard(standard)) {
    return ContractViolation("standard is invalid", location);
  }
  if (SpansOverlap(input, output)) {
    return ContractViolation("input and output spans must not overlap",
                             location);
  }
  return true;
}

inline std::optional<ConvertContext> PrepareConvertContext(
    std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
    Format input_color, Format output_color) noexcept {
  if (input.empty() || output.empty()) {
    return std::nullopt;
  }
  const auto input_pixels = input.size() / ChannelCount(input_color);
  const auto output_pixels = output.size() / ChannelCount(output_color);
  const auto pixel_count = std::min(input_pixels, output_pixels);
  if (pixel_count == 0) {
    return std::nullopt;
  }
  return ConvertContext{
      .input = input.data(),
      .output = output.data(),
      .pixel_count = pixel_count,
  };
}

constexpr std::size_t DispatchTableIndex(Format input_color,
                                         Format output_color,
                                         Standard standard) noexcept {
  return EnumIndex(input_color) * (kFormatCount * kStandardCount) +
         EnumIndex(output_color) * kStandardCount + EnumIndex(standard);
}

// --- chunking -------------------------------------------------------------

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
  return std::visit(
      [](const auto& policy) -> std::size_t {
        using Policy = std::decay_t<decltype(policy)>;
        if constexpr (std::is_same_v<Policy, parallel::Execution>) {
          return policy == parallel::Execution::kParallel
                     ? std::max<std::size_t>(
                           1, parallel::ThreadPool::Global().NumThreads())
                     : std::size_t{1};
        } else {
          return std::max<std::size_t>(1, policy.get().NumThreads());
        }
      },
      execution);
}

template <Format In, Format Out>
inline parallel::ExecutionPolicy MaybePreferSequential(
    std::size_t pixel_count, parallel::ExecutionPolicy execution) {
  if constexpr (!((In == Format::kGrayscale && Out == Format::kRgb) ||
                  (In == Format::kGrayscale && Out == Format::kRgba) ||
                  (In == Out))) {
    return execution;
  } else {
    constexpr std::size_t kSequentialThresholdBytes = 16u << 20;
    constexpr std::size_t kBytesPerPixel = ChannelCount(In) + ChannelCount(Out);

    return std::visit(
        [&](const auto& policy) -> parallel::ExecutionPolicy {
          using Policy = std::decay_t<decltype(policy)>;
          if constexpr (std::is_same_v<Policy, parallel::Execution>) {
            if (policy == parallel::Execution::kParallel &&
                pixel_count * kBytesPerPixel <= kSequentialThresholdBytes) {
              return parallel::Execution::kSequential;
            }
          }
          return execution;
        },
        execution);
  }
}

template <Format In, Format Out>
constexpr std::size_t ChunkPixels() {
  constexpr auto kBytesPerPixel = ChannelCount(In) + ChannelCount(Out);
  return std::max<std::size_t>(
      ChunkingPolicy<In, Out>::kMinChunkPixelsFloor,
      ChunkingPolicy<In, Out>::kMinChunkBytes / kBytesPerPixel);
}

inline constexpr std::size_t kChunkAlign = 256;

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

  auto chunk_size = CeilDiv(pixel_count, chunk_count);
  chunk_size = CeilDiv(chunk_size, kChunkAlign) * kChunkAlign;
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

// --- route -> engine mapping -------------------------------

template <Format F>
constexpr simd::Format ToEngineFormat() {
  if constexpr (F == Format::kRgb) {
    return simd::Format::kRgb;
  } else if constexpr (F == Format::kRgba) {
    return simd::Format::kRgba;
  } else if constexpr (F == Format::kGrayscale) {
    return simd::Format::kGrayscale;
  } else {
    return simd::Format::kCmyk;
  }
}

template <Standard S>
constexpr simd::ColorStandard ToEngineStandard() {
  return S == Standard::kBt601 ? simd::ColorStandard::kRec601
                               : simd::ColorStandard::kRec709;
}

template <Format In, Format Out, Standard Std>
inline void EngineConvertChunk(const std::uint8_t* input, std::uint8_t* output,
                               std::size_t count) {
  simd::ConvertChunk<ToEngineFormat<In>(), ToEngineFormat<Out>(),
                     ToEngineStandard<Std>()>(input, output, count);
}

template <Format In, Format Out, Standard Std>
void ConvertRoute(const std::uint8_t* input, std::uint8_t* output,
                  std::size_t pixel_count,
                  parallel::ExecutionPolicy execution) noexcept {
  constexpr auto kInChannels = ChannelCount(In);
  constexpr auto kOutChannels = ChannelCount(Out);

  const auto effective_execution =
      MaybePreferSequential<In, Out>(pixel_count, execution);

  ForEachPixelChunk<In, Out>(pixel_count, effective_execution,
                             [&](std::size_t start, std::size_t count) {
                               if constexpr (In == Out) {
                                 std::memcpy(output + start * kOutChannels,
                                             input + start * kInChannels,
                                             count * kInChannels);
                               } else {
                                 EngineConvertChunk<In, Out, Std>(
                                     input + start * kInChannels,
                                     output + start * kOutChannels, count);
                               }
                             });
}

// --- dispatch table -------------------------------------------------------

template <std::size_t... Is>
constexpr auto BuildDispatchTable(std::index_sequence<Is...>) {
  return std::array<ConvertFn, sizeof...(Is)>{[] {
    constexpr auto index = Is;
    constexpr auto input_color =
        static_cast<Format>(index / (kFormatCount * kStandardCount));
    constexpr auto output_color =
        static_cast<Format>((index / kStandardCount) % kFormatCount);
    constexpr auto standard = static_cast<Standard>(index % kStandardCount);
    return &ConvertRoute<input_color, output_color, standard>;
  }()...};
}

inline constexpr auto kDispatchTable =
    BuildDispatchTable(std::make_index_sequence<kDispatchTableSize>{});

static_assert(kDispatchTable.size() == kDispatchTableSize);

inline void ConvertImpl(std::span<const std::uint8_t> input,
                        std::span<std::uint8_t> output, Format input_color,
                        Format output_color, Standard standard,
                        parallel::ExecutionPolicy execution) noexcept {
  if (!ValidateConvertArguments(input, output, input_color, output_color,
                                standard)) {
    return;
  }

  const auto context =
      PrepareConvertContext(input, output, input_color, output_color);
  if (!context.has_value()) {
    return;
  }

  const auto index = DispatchTableIndex(input_color, output_color, standard);
  kDispatchTable[index](context->input, context->output, context->pixel_count,
                        execution);
}

}  // namespace internal

void Convert(std::span<const std::uint8_t> input,
             std::span<std::uint8_t> output, Format input_color,
             Format output_color, Standard standard,
             parallel::ExecutionPolicy execution) noexcept {
  internal::ConvertImpl(input, output, input_color, output_color, standard,
                        execution);
}

}  // namespace weqeqq::color

// ---------------------------------------------------------------------------
// std::format support.
// ---------------------------------------------------------------------------

template <>
struct std::formatter<weqeqq::color::Format> : std::formatter<std::string> {
  auto format(weqeqq::color::Format value, auto& context) const {
    using weqeqq::color::Format;
    std::string_view text;
    switch (value) {
      case Format::kRgb:
        text = "Rgb";
        break;
      case Format::kRgba:
        text = "Rgba";
        break;
      case Format::kGrayscale:
        text = "Grayscale";
        break;
      case Format::kCmyk:
        text = "Cmyk";
        break;
      default:
        text = "Unknown";
        break;
    }
    return std::formatter<std::string>::format(std::string(text), context);
  }
};

template <>
struct std::formatter<weqeqq::color::Standard> : std::formatter<std::string> {
  auto format(weqeqq::color::Standard value, auto& context) const {
    using weqeqq::color::Standard;
    std::string_view text;
    switch (value) {
      case Standard::kBt601:
        text = "Bt601";
        break;
      case Standard::kBt709:
        text = "Bt709";
        break;
      default:
        text = "Unknown";
        break;
    }
    return std::formatter<std::string>::format(std::string(text), context);
  }
};
