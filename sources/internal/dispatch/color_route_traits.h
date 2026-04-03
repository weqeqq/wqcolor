#pragma once

#include "internal/color_types.h"

namespace weqeqq::color::internal {

enum class RouteKind {
  kIdentity,
  kRgbToGrayscale,
  kRgbaToGrayscale,
  kRgbToRgba,
  kRgbaToRgb,
  kGrayscaleToRgb,
  kGrayscaleToRgba,
  kGeneric,
};

template <Format In, Format Out>
struct RouteTraits;

#define WQCOLOR_ROUTE(In, Out, Kind, HasSimd)                         \
  template <>                                                         \
  struct RouteTraits<Format::In, Format::Out> {                       \
    static constexpr RouteKind kKind = RouteKind::Kind;               \
    static constexpr bool kHasSimd = HasSimd;                         \
  };
#include "../color_routes.def"
#undef WQCOLOR_ROUTE

inline constexpr std::size_t kRegisteredRouteCount =
    0
#define WQCOLOR_ROUTE(...) + 1
#include "../color_routes.def"
    ;
#undef WQCOLOR_ROUTE

static_assert(kRegisteredRouteCount == kFormatCount * kFormatCount,
              "Route registry must cover every format pair");

}  // namespace weqeqq::color::internal
