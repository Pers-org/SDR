#include <cmath>
#include <complex>
#include <vector>

#include "../../../includes/RX/synchronizer.hpp"

std::vector<int16_t>
synchronizer::gardner(const std::vector<std::complex<double>> &samples,
                      int const L, float const Kp, float const BnTs) {

  double K1 = 0;
  double K2 = 0;
  double p1 = 0;
  double p2 = 0;
  double e = 0;
  int offset = 0;

  float zeta = std::sqrt(2) / 2;

  float teta = ((BnTs) / L) / (zeta + 1 / (4 * zeta));

  float denom = (1 + 2 * zeta * teta + teta * teta);

  K1 = (-4 * zeta * teta) / (denom * Kp);
  K2 = (-4 * teta * teta) / (denom * Kp);

  std::vector<int16_t> offset_list;

  int iterations = static_cast<int>(samples.size()) / L - 1;

  for (int i = 0; i < iterations; ++i) {
    double base = L * i;

    int idx0 = base + offset;
    int idx1 = base + offset + L;
    int idxm = base + offset + L / 2;

    e = (std::real(samples[idx1]) - std::real(samples[idx0])) *
        std::real(samples[idxm]);

    e += (std::imag(samples[idx1]) - std::imag(samples[idx0])) *
         std::imag(samples[idxm]);

    p1 = e * K1;
    p2 += p1 + e * K2;

    p2 = std::fmod(p2, 1.0);
    if (p2 < 0.0)
      p2 += 1.0;

    offset = std::lround(p2 * L);

    offset_list.push_back(offset + base);
  }

  return offset_list;
}
