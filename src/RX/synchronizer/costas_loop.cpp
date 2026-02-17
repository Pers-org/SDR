#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

#include "../../../includes/RX/synchronizer.hpp"

std::vector<std::complex<double>>
synchronizer::costas_loop(const std::vector<std::complex<double>> &samples) {
  std::vector<std::complex<double>> out(samples.size());

  double theta = 0;
  double freq = 0;

  const double Kp = 0.02;
  const double Ki = 0.0001;

  for (int n = 0; n < samples.size(); ++n) {
    std::complex<double> r =
        samples[n] * std::exp(std::complex<double>(0, -theta));
    out[n] = r;

    double l = r.real();
    double Q = r.imag();

    double I_hat = (l >= 0.0) ? 1 : -1;
    double Q_hat = (Q >= 0.0) ? 1 : -1;

    double error = I_hat * Q - Q_hat * l;
    error = std::clamp(error, -1.0, 1.0);

    freq += Ki * error;
    theta += freq + Kp * error;
    theta = std::fmod(theta + M_PI, 2 * M_PI) - M_PI;
  }

  return out;
}
