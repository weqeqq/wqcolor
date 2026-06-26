#include <weqeqq/color/simd.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace wc = weqeqq::color::simd;

namespace ref {

std::uint8_t Div255(std::uint32_t v) {
  return static_cast<std::uint8_t>(v * 257u >> 16);
}

void RgbToRgba(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    out[i * 4 + 0] = in[i * 3 + 0];
    out[i * 4 + 1] = in[i * 3 + 1];
    out[i * 4 + 2] = in[i * 3 + 2];
    out[i * 4 + 3] = 0xff;
  }
}
void RgbaToRgb(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    out[i * 3 + 0] = in[i * 4 + 0];
    out[i * 3 + 1] = in[i * 4 + 1];
    out[i * 3 + 2] = in[i * 4 + 2];
  }
}
void GrayToRgb(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++)
    out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = in[i];
}
void GrayToRgba(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = in[i];
    out[i * 4 + 3] = 0xff;
  }
}
void GrayToCmyk(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = 0;
    out[i * 4 + 3] = 255u - in[i];
  }
}
template <int CR, int CG, int CB, int IC>
void ToGray(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++)
    out[i] =
        Div255(in[i * IC + 0] * CR + in[i * IC + 1] * CG + in[i * IC + 2] * CB);
}
template <int IC>
void ToCmyk(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    std::uint32_t r = in[i * IC + 0], g = in[i * IC + 1], b = in[i * IC + 2];
    std::uint32_t mx = std::max({r, g, b});
    if (mx == 0) {
      out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = 0;
      out[i * 4 + 3] = 0xff;
      continue;
    }
    out[i * 4 + 0] = (mx - r) * 255u / mx;
    out[i * 4 + 1] = (mx - g) * 255u / mx;
    out[i * 4 + 2] = (mx - b) * 255u / mx;
    out[i * 4 + 3] = 255u - mx;
  }
}
template <int OC>
void CmykToRgbX(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    std::uint32_t invk = 255u - in[i * 4 + 3];
    out[i * OC + 0] = Div255((255u - in[i * 4 + 0]) * invk);
    out[i * OC + 1] = Div255((255u - in[i * 4 + 1]) * invk);
    out[i * OC + 2] = Div255((255u - in[i * 4 + 2]) * invk);
    if (OC == 4) out[i * OC + 3] = 0xff;
  }
}
template <int CR, int CG, int CB>
void CmykToGray(const std::uint8_t* in, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) {
    if (in[i * 4 + 0] == 0 && in[i * 4 + 1] == 0 && in[i * 4 + 2] == 0) {
      out[i] = 255u - in[i * 4 + 3];
      continue;
    }
    std::uint32_t invk = 255u - in[i * 4 + 3];
    std::uint8_t r = Div255((255u - in[i * 4 + 0]) * invk);
    std::uint8_t g = Div255((255u - in[i * 4 + 1]) * invk);
    std::uint8_t b = Div255((255u - in[i * 4 + 2]) * invk);
    out[i] = Div255(r * CR + g * CG + b * CB);
  }
}

}  // namespace ref

static int g_failures = 0;

void Check(const char* name, const std::vector<std::uint8_t>& got,
           const std::vector<std::uint8_t>& exp, int tol) {
  int max_diff = 0;
  std::size_t bad = 0;
  for (std::size_t i = 0; i < got.size(); i++) {
    int d = std::abs(int(got[i]) - int(exp[i]));
    max_diff = std::max(max_diff, d);
    if (d > tol) bad++;
  }
  if (bad) {
    std::printf("  FAIL %-26s bad=%zu/%zu max_diff=%d (tol=%d)\n", name, bad,
                got.size(), max_diff, tol);
    g_failures++;
  } else {
    std::printf("  ok   %-26s max_diff=%d\n", name, max_diff);
  }
}

int main() {
  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> dist(0, 255);

  for (std::size_t n : {1u, 3u, 7u, 15u, 16u, 17u, 31u, 33u, 64u, 65u, 100u,
                        1000u, 4096u, 5000u}) {
    std::printf("count = %zu\n", n);
    std::vector<std::uint8_t> rgb(n * 3), rgba(n * 4), gray(n), cmyk(n * 4);
    for (auto& v : rgb) v = dist(rng);
    for (auto& v : rgba) v = dist(rng);
    for (auto& v : gray) v = dist(rng);
    for (auto& v : cmyk) v = dist(rng);
    // include some achromatic / black cmyk pixels
    for (std::size_t i = 0; i < n && i < 8; i++) {
      cmyk[i * 4 + 0] = cmyk[i * 4 + 1] = cmyk[i * 4 + 2] = 0;
    }

    std::vector<std::uint8_t> got, exp;
    auto run3 = [&](auto fn, auto reffn, const std::vector<std::uint8_t>& in,
                    int oc, const char* name, int tol) {
      got.assign(n * oc, 0xAB);
      exp.assign(n * oc, 0xCD);
      fn(in.data(), got.data(), n);
      reffn(in.data(), exp.data(), n);
      Check(name, got, exp, tol);
    };

    run3(wc::ConvertRgbToRgba, ref::RgbToRgba, rgb, 4, "RgbToRgba", 0);
    run3(wc::ConvertRgbToGrayscaleRec601, ref::ToGray<76, 150, 29, 3>, rgb, 1,
         "RgbToGrayscaleRec601", 0);
    run3(wc::ConvertRgbToGrayscaleRec709, ref::ToGray<54, 183, 18, 3>, rgb, 1,
         "RgbToGrayscaleRec709", 0);
    run3(wc::ConvertRgbToCmyk, ref::ToCmyk<3>, rgb, 4, "RgbToCmyk", 1);

    run3(wc::ConvertRgbaToRgb, ref::RgbaToRgb, rgba, 3, "RgbaToRgb", 0);
    run3(wc::ConvertRgbaToGrayscaleRec601, ref::ToGray<76, 150, 29, 4>, rgba, 1,
         "RgbaToGrayscaleRec601", 0);
    run3(wc::ConvertRgbaToGrayscaleRec709, ref::ToGray<54, 183, 18, 4>, rgba, 1,
         "RgbaToGrayscaleRec709", 0);
    run3(wc::ConvertRgbaToCmyk, ref::ToCmyk<4>, rgba, 4, "RgbaToCmyk", 1);

    run3(wc::ConvertGrayscaleToRgb, ref::GrayToRgb, gray, 3, "GrayToRgb", 0);
    run3(wc::ConvertGrayscaleToRgba, ref::GrayToRgba, gray, 4, "GrayToRgba", 0);
    run3(wc::ConvertGrayscaleToCmyk, ref::GrayToCmyk, gray, 4, "GrayToCmyk", 0);

    run3(wc::ConvertCmykToRgb, ref::CmykToRgbX<3>, cmyk, 3, "CmykToRgb", 0);
    run3(wc::ConvertCmykToRgba, ref::CmykToRgbX<4>, cmyk, 4, "CmykToRgba", 0);
    run3(wc::ConvertCmykToGrayscale, ref::CmykToGray<76, 150, 29>, cmyk, 1,
         "CmykToGrayscale", 1);
  }

  std::printf("\n%s (%d failing groups)\n", g_failures ? "FAILURES" : "ALL OK",
              g_failures);
  return g_failures ? 1 : 0;
}
