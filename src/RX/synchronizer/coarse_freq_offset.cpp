#include <complex>
#include <fftw3.h>
#include <iostream>
#include <vector>

#include "../../../includes/ImGUI_interface.h"
#include "../../../includes/RX/synchronizer.hpp"
#include "../fft.hpp"

void synchronizer::coarse_freq_offset(
    const std::vector<std::complex<int16_t>> &samples, rx_cfg &rx_config,
    int Fs) {
  std::vector<std::complex<double>> pow_samples(samples.size());

  for (size_t i = 0; i < samples.size(); ++i) {
    std::complex<double> x(static_cast<double>(samples[i].real()),
                           static_cast<double>(samples[i].imag()));

    pow_samples[i] = std::pow(x, rx_config.mod_order);
  }

  rx_config.CFO_spectrum = fft(pow_samples, Fs);

  double max = -__DBL_MAX__;
  int index;

  for (int i = 0; i < rx_config.CFO_spectrum.first.size(); ++i) {
    if (std::abs(rx_config.CFO_spectrum.first[i]) > max) {
      max = std::abs(rx_config.CFO_spectrum.first[i]);
      index = i;
    }
  }

  double df = rx_config.CFO_spectrum.second[index];

  for (int i = 0; i < rx_config.rx_samples.size(); ++i) {
    rx_config.rx_samples[i] *=
        std::exp(std::complex<double>(0.0, -2.0 * M_PI * df));
  }
}
