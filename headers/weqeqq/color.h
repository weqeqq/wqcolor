#pragma once

#include <weqeqq/color/config.h>
#include <weqeqq/color/export.h>
#include <weqeqq/parallel.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <source_location>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace weqeqq::color {

/**
 * \brief Exception type for API contract diagnostics.
 *
 * The message is decorated with source location (`file:line`) captured at
 * construction time.
 */
struct WQCOLOR_EXPORT Error : std::runtime_error {
  Error(std::string_view message,
        std::source_location location = std::source_location::current())
      : std::runtime_error(std::format("[{}:{}] {}", location.file_name(),
                                       location.line(), message)) {}
};

/**
 * \brief Interleaved 8-bit pixel formats supported by `Convert`.
 */
enum class Format {
  /// 3 channels per pixel: `R, G, B`.
  kRgb,
  /// 4 channels per pixel: `R, G, B, A`.
  kRgba,
  /// 1 channel per pixel: gray intensity.
  kGrayscale,
  /// 4 channels per pixel: `C, M, Y, K`.
  kCmyk,

  /// Sentinel value, not a runtime pixel format.
  kCount,
};

/**
 * \brief Color standard used by luma-based conversions.
 */
enum class Standard {
  /// ITU-R BT.601 coefficients.
  kBt601,
  /// ITU-R BT.709 coefficients.
  kBt709,

  /// Sentinel value, not a runtime standard.
  kCount,
};

/**
 * \brief Returns whether a pixel format is a supported runtime value.
 */
[[nodiscard]] inline constexpr bool IsValidFormat(Format format) noexcept {
  return std::to_underlying(format) >= 0 &&
         std::to_underlying(format) < std::to_underlying(Format::kCount);
}

/**
 * \brief Returns whether a luma standard is a supported runtime value.
 */
[[nodiscard]] inline constexpr bool IsValidStandard(
    Standard standard) noexcept {
  return std::to_underlying(standard) >= 0 &&
         std::to_underlying(standard) < std::to_underlying(Standard::kCount);
}

/**
 * \brief Returns channel count for a given format.
 *
 * \param color Pixel format.
 * \return Number of channels per pixel (`1`, `3`, or `4`).
 * \note Passing an invalid format is a contract violation.
 * In debug builds the function throws `Error`; in release builds it returns
 * `0`.
 */
inline constexpr std::size_t ChannelCount(Format color) noexcept(!kDebug) {
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

  if constexpr (kDebug) {
    throw Error("format is invalid");
  }
  return 0;
}

/**
 * \brief Computes output byte size for complete source pixels.
 *
 * The result is based on full source pixels only. Any incomplete trailing input
 * bytes are ignored.
 *
 * \param input Raw source byte buffer.
 * \param input_color Source pixel format.
 * \param output_color Destination pixel format.
 * \return Required output byte count for converting complete source pixels.
 * \note Invalid formats are a contract violation.
 * In debug builds the function throws `Error`; in release builds it returns
 * `0`.
 */
inline std::size_t RequiredOutputSize(std::span<const std::uint8_t> input,
                                      Format input_color,
                                      Format output_color) noexcept(!kDebug) {
  if (!IsValidFormat(input_color)) {
    if constexpr (kDebug) {
      throw Error("input format is invalid");
    }
    return 0;
  }

  if (!IsValidFormat(output_color)) {
    if constexpr (kDebug) {
      throw Error("output format is invalid");
    }
    return 0;
  }

  if (input.empty()) {
    return 0;
  }
  return input.size() / ChannelCount(input_color) * ChannelCount(output_color);
}

/**
 * \brief Converts source pixels into caller-provided output storage.
 *
 * Conversion count is limited to complete pixels that fit in both buffers:
 * `min(input.size() / in_channels, output.size() / out_channels)`.
 * Incomplete trailing bytes are ignored.
 *
 * \param input Source byte buffer.
 * \param output Destination byte buffer.
 * \param input_color Source pixel format.
 * \param output_color Destination pixel format.
 * \param standard Luma standard (`Standard::kBt709` by default).
 * \param execution Execution policy (`parallel::Execution::kSequential` by
 * default).
 *
 * \note If either buffer is empty, or no complete pixel fits, the function
 * returns without writing.
 * \note `input` and `output` must not overlap in memory.
 * Invalid formats, invalid standards, and overlapping spans are contract
 * violations. In debug builds the function throws `Error`; in release builds
 * it returns without writing.
 * \note The exception surface follows `noexcept(!kDebug)`.
 */
WQCOLOR_EXPORT void Convert(
    std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
    Format input_color, Format output_color,
    Standard standard = Standard::kBt709,
    parallel::ExecutionPolicy execution =
        parallel::Execution::kSequential) noexcept(!kDebug);

/**
 * \brief Convenience overload that allocates and returns output bytes.
 *
 * Allocates `RequiredOutputSize(input, input_color, output_color)` bytes and
 * then calls the span overload.
 *
 * \param input Source byte buffer.
 * \param input_color Source pixel format.
 * \param output_color Destination pixel format.
 * \param standard Luma standard.
 * \param execution Execution policy.
 * \return Converted output buffer.
 */
inline std::vector<std::uint8_t> Convert(
    std::span<const std::uint8_t> input, Format input_color,
    Format output_color, Standard standard,
    parallel::ExecutionPolicy execution = parallel::Execution::kSequential) {
  std::vector<std::uint8_t> output(
      RequiredOutputSize(input, input_color, output_color));

  Convert(input, output, input_color, output_color, standard, execution);
  return output;
}

}  // namespace weqeqq::color

/**
 * \brief Formatter for `weqeqq::color::Format`.
 *
 * Produces tokens: `Rgb`, `Rgba`, `Grayscale`, `Cmyk` (or `Unknown` for
 * unexpected values).
 */
template <>
struct std::formatter<weqeqq::color::Format> : std::formatter<std::string> {
  auto format(weqeqq::color::Format format, auto& context) const {
    using namespace weqeqq::color;

    std::string string;
    switch (format) {
      case Format::kRgb:
        string = "Rgb";
        break;
      case Format::kRgba:
        string = "Rgba";
        break;
      case Format::kGrayscale:
        string = "Grayscale";
        break;
      case Format::kCmyk:
        string = "Cmyk";
        break;
      case Format::kCount:
        [[fallthrough]];
      default:
        string = "Unknown";
        break;
    }
    return std::format_to(context.out(), "{}", string);
  }
};

/**
 * \brief Formatter for `weqeqq::color::Standard`.
 *
 * Produces tokens: `Bt601`, `Bt709` (or `Unknown` for unexpected values).
 */
template <>
struct std::formatter<weqeqq::color::Standard> : std::formatter<std::string> {
  auto format(weqeqq::color::Standard standard, auto& context) const {
    using namespace weqeqq::color;

    std::string_view string;
    switch (standard) {
      case Standard::kBt601:
        string = "Bt601";
        break;
      case Standard::kBt709:
        string = "Bt709";
        break;
      case Standard::kCount:
        [[fallthrough]];
      default:
        string = "Unknown";
        break;
    }
    return std::format_to(context.out(), "{}", string);
  }
};
