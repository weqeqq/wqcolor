import weqeqq.color;
import weqeqq.parallel;

#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <random>
#include <span>
#include <vector>

namespace wc = weqeqq::color;
namespace wp = weqeqq::parallel;

static int g_failures = 0;

void Expect(bool ok, const char* what) {
  if (!ok) {
    std::printf("  FAIL %s\n", what);
    g_failures++;
  } else {
    std::printf("  ok   %s\n", what);
  }
}

int main() {
  std::mt19937 rng(777);
  std::uniform_int_distribution<int> dist(0, 255);

  const std::size_t n = 200000;
  std::vector<std::uint8_t> rgb(n * 3);
  for (auto& v : rgb) v = dist(rng);

  auto gray = wc::Convert(rgb, wc::Format::kRgb, wc::Format::kGrayscale,
                          wc::Standard::kBt709);
  Expect(gray.size() == n, "RequiredOutputSize for grayscale");

  std::vector<std::uint8_t> seq(n), par(n);
  wc::Convert(rgb, seq, wc::Format::kRgb, wc::Format::kGrayscale,
              wc::Standard::kBt601, wp::Execution::kSequential);
  wc::Convert(rgb, par, wc::Format::kRgb, wc::Format::kGrayscale,
              wc::Standard::kBt601, wp::Execution::kParallel);
  Expect(seq == par, "parallel == sequential (RGB->Gray)");

  auto rgba = wc::Convert(rgb, wc::Format::kRgb, wc::Format::kRgba,
                          wc::Standard::kBt709, wp::Execution::kParallel);
  Expect(rgba.size() == n * 4, "RGBA size");
  auto back = wc::Convert(rgba, wc::Format::kRgba, wc::Format::kRgb,
                          wc::Standard::kBt709, wp::Execution::kParallel);
  Expect(back == rgb, "RGB->RGBA->RGB round trip");

  wp::ThreadPool pool(4);
  std::vector<std::uint8_t> pooled(n * 4);
  wc::Convert(rgb, pooled, wc::Format::kRgb, wc::Format::kCmyk,
              wc::Standard::kBt709, std::ref(pool));
  auto cmyk_seq = wc::Convert(rgb, wc::Format::kRgb, wc::Format::kCmyk,
                              wc::Standard::kBt709, wp::Execution::kSequential);
  Expect(pooled == cmyk_seq, "thread-pool == sequential (RGB->CMYK)");

  std::vector<std::uint8_t> id(n * 4);
  wc::Convert(rgba, id, wc::Format::kRgba, wc::Format::kRgba,
              wc::Standard::kBt709, wp::Execution::kParallel);
  Expect(id == rgba, "identity RGBA->RGBA");

  auto empty = wc::Convert(std::span<const std::uint8_t>{}, wc::Format::kRgb,
                           wc::Format::kRgba, wc::Standard::kBt709);
  Expect(empty.empty(), "empty input handled");

  Expect(std::format("{}", wc::Format::kCmyk) == "Cmyk", "format Format");
  Expect(std::format("{}", wc::Standard::kBt709) == "Bt709", "format Standard");

  std::printf("\n%s (%d failures)\n", g_failures ? "FAILURES" : "ALL OK",
              g_failures);
  return g_failures ? 1 : 0;
}
