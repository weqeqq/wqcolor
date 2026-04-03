# `sources/internal`

Internal implementation layout for `wqcolor`.

## Design rules

- Public API stays in `headers/`; nothing here is public.
- `sources/color.cc` is the only production translation unit for the library.
- Regular internal interfaces use `.h`.
- Include-only implementation fragments use `.inc`.
- SIMD code is compiled through Highway target re-inclusion from `sources/color.cc`.
- Route inventory lives in one place: `color_routes.def`.

## Layout

- `color_types.h`: internal aliases and dispatch-table constants.
- `color_coefficients.h`: luma coefficients by standard.
- `color_chunking.h`: chunk sizing, worker selection, and parallel splitting.
- `color_validate.h`: top-level input/output normalization before dispatch.
- `color_routes.def`: single source of truth for supported `Format -> Format` routes.
- `color_impl.inc`: assembles dispatch fragments into the implementation TU.

### `dispatch/`

- `color_route_traits.h`: route metadata derived from `color_routes.def`.
- `color_route_dispatch.inc`: specialized route entrypoints.
- `color_backend_select.inc`: route-kind to backend selection.
- `color_route_table.inc`: dispatch table generation and `ConvertImpl(...)`.

### `scalar/`

- `color_scalar_common.h`: shared scalar helpers and per-pixel conversions.
- `color_scalar_routes.h`: scalar chunk/identity/generic route execution.

### `simd/`

- `color_hwy_common.inc`: shared Highway helpers.
- `color_hwy_routes.inc`: SIMD route implementations and chunk kernels.

## How to add a new conversion

### If the route can use the generic path

1. Add the route to `color_routes.def`.
2. Implement the scalar per-pixel conversion in `scalar/color_scalar_common.h`.
3. If SIMD is supported, implement the SIMD per-lane conversion in `simd/color_hwy_routes.inc`.
4. Mark SIMD availability correctly in `color_routes.def`.
5. Add or extend tests.

No dispatch-table edits should be needed beyond the registry entry.

### If the route needs a specialized path

Use this for routes that need:

- custom chunk scheduling,
- a dedicated SIMD chunk kernel,
- special execution-policy handling,
- or a dedicated fast path instead of generic `ConvertChunkScalar` / `ConvertChunkSimd`.

Steps:

1. Add the route to `color_routes.def` with a dedicated `RouteKind`.
2. Extend `dispatch/color_route_traits.h` with the new `RouteKind` value.
3. Add the specialized route entrypoint in `dispatch/color_route_dispatch.inc`.
4. Wire the new `RouteKind` in `dispatch/color_backend_select.inc`.
5. Add or extend scalar/SIMD helpers as needed.
6. Add or extend tests.

## Invariants

- Registry coverage must stay complete for all `Format x Format` pairs.
- `ConvertImpl(...)` must stay behavior-compatible with the public API contract.
- SIMD and non-SIMD builds must both compile and pass the same test suite.
- Keep route registration centralized; do not re-introduce scattered manual tables.
- Do not bring back dual-purpose headers guarded by `WQCOLOR_INLINE_HEADERS`.
