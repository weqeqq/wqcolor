#include <weqeqq/color.h>
#include <weqeqq/parallel.h>

#include <array>
#include <concepts>
#include <type_traits>

#include "internal/color_chunking.h"
#include "internal/color_coefficients.h"
#include "internal/color_types.h"
#include "internal/color_validate.h"
#include "internal/dispatch/color_route_traits.h"
#include "internal/scalar/color_scalar_common.h"
#include "internal/scalar/color_scalar_routes.h"

#if WQCOLOR_SIMD

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "color.cc"

#include <hwy/foreach_target.h>
#include <hwy/highway.h>

HWY_BEFORE_NAMESPACE();

#endif

namespace weqeqq::color::internal {
#if WQCOLOR_SIMD
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

#include "internal/simd/color_hwy_common.inc"
#include "internal/simd/color_hwy_routes.inc"
#endif

#include "internal/color_impl.inc"

#if WQCOLOR_SIMD
}  // namespace HWY_NAMESPACE
#endif
}  // namespace weqeqq::color::internal

#if WQCOLOR_SIMD
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace weqeqq::color {
namespace internal {
HWY_EXPORT(ConvertImpl);
}  // namespace internal

void Convert(std::span<const std::uint8_t> input,
             std::span<std::uint8_t> output, Format input_color,
             Format output_color, Standard standard,
             parallel::ExecutionPolicy execution) noexcept(!kDebug) {
  HWY_DYNAMIC_DISPATCH(internal::ConvertImpl)(
      input, output, input_color, output_color, standard, execution);
}

}  // namespace weqeqq::color

#endif
#else

namespace weqeqq::color {

void Convert(std::span<const std::uint8_t> input,
             std::span<std::uint8_t> output, Format input_color,
             Format output_color, Standard standard,
             parallel::ExecutionPolicy execution) noexcept(!kDebug) {
  internal::ConvertImpl(input, output, input_color, output_color, standard,
                        execution);
}

}  // namespace weqeqq::color

#endif
