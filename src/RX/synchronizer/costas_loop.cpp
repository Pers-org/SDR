#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

#include "../../../includes/RX/synchronizer.hpp"

std::vector<std::complex<double>>
synchronizer::costas_loop(const std::vector<std::complex<double>> &samples,
                          const int L, const double BnTs, const double Kp,
                          const int mod_order) {
  const double zeta = std::sqrt(2.0) / 2.0;

  double theta = BnTs / (zeta + 0.25 / zeta);
  double denom = 1 + 2 * zeta * theta + theta * theta;

  double K1 = 4 * zeta * theta / (denom * Kp);
  double K2 = 4 * theta * theta / (denom * Kp);

  double phase = 0.0;
  double freq = 0.0;

  double err;

  std::vector<std::complex<double>> out(samples.size());

  double ROT = M_PI / 4;

  auto sign = [](double x) { return (x > 0) - (x < 0); };

  for (size_t i = 0; i < samples.size(); ++i) {
    std::complex<double> rot(std::cos(phase), -std::sin(phase));

    std::complex<double> sample = samples[i];

    out[i] = sample * rot;

    double re = out[i].real();
    double im = out[i].imag();

    if (mod_order == 2) {
      double d = (re + im >= 0) ? 1.0 : -1.0;
      err = d * (im - re);
    } else {
      err = sign(re) * im - sign(im) * re;
    }

    freq += K2 * err;
    phase += freq + K1 * err;
  }

  return out;
}