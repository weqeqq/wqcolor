#pragma once

namespace weqeqq::color {

/**
 * \brief Compile-time flag for SIMD acceleration availability.
 *
 * Mirrors the Meson option `-Dsimd` in this build.
 */
inline constexpr bool kSimd = WQCOLOR_SIMD;

/**
 * \brief Compile-time flag for debug contract/diagnostic mode.
 *
 * Used by API signatures such as `Convert(...) noexcept(!kDebug)`.
 */
inline constexpr bool kDebug = WQCOLOR_DEBUG;

}  // namespace weqeqq::color
