#include <cmath>
#include <complex>
#include <vector>

#include "../../../includes/RX/synchronizer.hpp"

std::vector<int16_t>
synchronizer::gardner(const std::vector<std::complex<double>> &samples,
                      int const L) {
  double Kp = 3.82;
  double BnTs = 0.0005;

  double zeta = std::sqrt(2) / 2;
  double theta = (BnTs / 10) / (zeta + 1 / (4 * zeta));
  double den = (1 + 2 * zeta * theta + theta * theta) * Kp;

  double K1 = -4 * zeta * theta / den;
  double K2 = -4 * theta * theta / den;

  double p1 = 0;
  double p2 = 0;

  int offset = L / 2;
  double e = 0;

  int n = 0;

  std::vector<int16_t> offset_list;

  int max_i =
      samples.size() % L == 0 ? samples.size() / L : samples.size() / L + 1;

  for (int i = 0; i < max_i; ++i) {
    n = offset + L * i;

    e = (samples[n + L].real() - samples[n].real()) *
            samples[n + L / 2].real() +
        (samples[n + L].imag() - samples[n].imag()) * samples[n + L / 2].imag();
    e = -e;
    p1 = e * K1;
    p2 += p1 + e * K2;

    p2 -= std::floor(p2);

    offset = static_cast<int>(std::round(p2 * L));
    offset_list.push_back(offset);
  }

  return offset_list;
}