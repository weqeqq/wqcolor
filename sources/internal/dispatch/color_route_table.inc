template <std::size_t... Is>
constexpr auto BuildDispatchTable(std::index_sequence<Is...>) {
  return std::array<ConvertFn, sizeof...(Is)>{[] {
    constexpr auto index = Is;
    constexpr auto input_color =
        static_cast<Format>(index / (kFormatCount * kStandardCount));
    constexpr auto output_color =
        static_cast<Format>((index / kStandardCount) % kFormatCount);
    constexpr auto standard = static_cast<Standard>(index % kStandardCount);
    return MakeConvertFn<input_color, output_color, standard>();
  }()...};
}

inline constexpr auto kDispatchTable =
    BuildDispatchTable(std::make_index_sequence<kDispatchTableSize>{});

static_assert(kDispatchTable.size() == kDispatchTableSize);

void ConvertImpl(std::span<const std::uint8_t> input,
                 std::span<std::uint8_t> output, Format input_color,
                 Format output_color, Standard standard,
                 parallel::ExecutionPolicy execution) {
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
