module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

export module wqcolor.tests.reference;

export namespace reference {

std::uint8_t Div255(std::uint32_t value) {
  return static_cast<std::uint8_t>(value * 257u >> 16);
}

std::vector<std::uint8_t> RandomBytes(std::size_t size, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> dist(0, 255);
  std::vector<std::uint8_t> data(size);
  for (auto& byte : data) {
    byte = static_cast<std::uint8_t>(dist(rng));
  }
  return data;
}

int MaxDiff(const std::vector<std::uint8_t>& lhs,
            const std::vector<std::uint8_t>& rhs) {
  if (lhs.size() != rhs.size()) {
    return 256;
  }
  int max_diff = 0;
  for (std::size_t index = 0; index < lhs.size(); index++) {
    max_diff = std::max(max_diff,
                        std::abs(static_cast<int>(lhs[index]) -
                                 static_cast<int>(rhs[index])));
  }
  return max_diff;
}

std::vector<std::uint8_t> RgbToRgb(const std::vector<std::uint8_t>& in) {
  return in;
}

std::vector<std::uint8_t> RgbToRgba(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / 3;
  std::vector<std::uint8_t> out(n * 4);
  for (std::size_t i = 0; i < n; i++) {
    out[i * 4 + 0] = in[i * 3 + 0];
    out[i * 4 + 1] = in[i * 3 + 1];
    out[i * 4 + 2] = in[i * 3 + 2];
    out[i * 4 + 3] = 0xff;
  }
  return out;
}

template <int Cr, int Cg, int Cb, int Channels>
std::vector<std::uint8_t> ToGray(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / Channels;
  std::vector<std::uint8_t> out(n);
  for (std::size_t i = 0; i < n; i++) {
    out[i] = Div255(in[i * Channels + 0] * Cr + in[i * Channels + 1] * Cg +
                    in[i * Channels + 2] * Cb);
  }
  return out;
}

template <int Channels>
std::vector<std::uint8_t> ToCmyk(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / Channels;
  std::vector<std::uint8_t> out(n * 4);
  for (std::size_t i = 0; i < n; i++) {
    const std::uint32_t r = in[i * Channels + 0];
    const std::uint32_t g = in[i * Channels + 1];
    const std::uint32_t b = in[i * Channels + 2];
    const std::uint32_t mx = std::max({r, g, b});
    if (mx == 0) {
      out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = 0;
      out[i * 4 + 3] = 0xff;
      continue;
    }
    out[i * 4 + 0] = static_cast<std::uint8_t>((mx - r) * 255u / mx);
    out[i * 4 + 1] = static_cast<std::uint8_t>((mx - g) * 255u / mx);
    out[i * 4 + 2] = static_cast<std::uint8_t>((mx - b) * 255u / mx);
    out[i * 4 + 3] = static_cast<std::uint8_t>(255u - mx);
  }
  return out;
}

std::vector<std::uint8_t> RgbaToRgb(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / 4;
  std::vector<std::uint8_t> out(n * 3);
  for (std::size_t i = 0; i < n; i++) {
    out[i * 3 + 0] = in[i * 4 + 0];
    out[i * 3 + 1] = in[i * 4 + 1];
    out[i * 3 + 2] = in[i * 4 + 2];
  }
  return out;
}

std::vector<std::uint8_t> RgbaToRgba(const std::vector<std::uint8_t>& in) {
  return in;
}

std::vector<std::uint8_t> GrayToRgb(const std::vector<std::uint8_t>& in) {
  std::vector<std::uint8_t> out(in.size() * 3);
  for (std::size_t i = 0; i < in.size(); i++) {
    out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = in[i];
  }
  return out;
}

std::vector<std::uint8_t> GrayToRgba(const std::vector<std::uint8_t>& in) {
  std::vector<std::uint8_t> out(in.size() * 4);
  for (std::size_t i = 0; i < in.size(); i++) {
    out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = in[i];
    out[i * 4 + 3] = 0xff;
  }
  return out;
}

std::vector<std::uint8_t> GrayToGray(const std::vector<std::uint8_t>& in) {
  return in;
}

std::vector<std::uint8_t> GrayToCmyk(const std::vector<std::uint8_t>& in) {
  std::vector<std::uint8_t> out(in.size() * 4);
  for (std::size_t i = 0; i < in.size(); i++) {
    out[i * 4 + 0] = out[i * 4 + 1] = out[i * 4 + 2] = 0;
    out[i * 4 + 3] = static_cast<std::uint8_t>(255u - in[i]);
  }
  return out;
}

template <int Channels>
std::vector<std::uint8_t> CmykToRgbLike(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / 4;
  std::vector<std::uint8_t> out(n * Channels);
  for (std::size_t i = 0; i < n; i++) {
    const std::uint32_t invk = 255u - in[i * 4 + 3];
    out[i * Channels + 0] = Div255((255u - in[i * 4 + 0]) * invk);
    out[i * Channels + 1] = Div255((255u - in[i * 4 + 1]) * invk);
    out[i * Channels + 2] = Div255((255u - in[i * 4 + 2]) * invk);
    if constexpr (Channels == 4) {
      out[i * Channels + 3] = 0xff;
    }
  }
  return out;
}

std::vector<std::uint8_t> CmykToRgb(const std::vector<std::uint8_t>& in) {
  return CmykToRgbLike<3>(in);
}

std::vector<std::uint8_t> CmykToRgba(const std::vector<std::uint8_t>& in) {
  return CmykToRgbLike<4>(in);
}

std::vector<std::uint8_t> CmykToCmyk(const std::vector<std::uint8_t>& in) {
  return in;
}

std::vector<std::uint8_t> CmykToGray(const std::vector<std::uint8_t>& in) {
  const std::size_t n = in.size() / 4;
  std::vector<std::uint8_t> out(n);
  for (std::size_t i = 0; i < n; i++) {
    if (in[i * 4 + 0] == 0 && in[i * 4 + 1] == 0 && in[i * 4 + 2] == 0) {
      out[i] = static_cast<std::uint8_t>(255u - in[i * 4 + 3]);
      continue;
    }
    const std::uint32_t invk = 255u - in[i * 4 + 3];
    const std::uint8_t r = Div255((255u - in[i * 4 + 0]) * invk);
    const std::uint8_t g = Div255((255u - in[i * 4 + 1]) * invk);
    const std::uint8_t b = Div255((255u - in[i * 4 + 2]) * invk);
    out[i] = Div255(r * 76u + g * 150u + b * 29u);
  }
  return out;
}

}  // namespace reference
