#include <complex>
#include <fftw3.h>
#include <iostream>
#include <vector>

#include "../../../includes/ImGUI_interface.h"
#include "../../../includes/RX/synchronizer.hpp"
#include "../fft.hpp"

double metric(const std::vector<std::complex<double>> &x, double phase, int M) {
  std::complex<double> acc(0.0, 0.0);

  for (int i = 0; i < x.size(); ++i) {
    std::complex<double> xm = std::pow(x[i], M);
    std::complex<double> rot =
        std::exp(std::complex<double>(0, -phase * (double)i));
    acc += xm * rot;
  }

  acc /= (double)x.size();

  return std::abs(acc);
}

void synchronizer::coarse_freq_offset(
    const std::vector<std::complex<double>> &samples, rx_cfg &rx_config,
    int Fs) {
  // coarse
  std::vector<std::complex<double>> pow_samples(samples.size());

  for (int i = 0; i < samples.size(); ++i) {
    pow_samples[i] = std::pow(samples[i], rx_config.mod_order);
  }

  rx_config.CFO_spectrum = fft(pow_samples, Fs);
  int nfft = rx_config.CFO_spectrum.first.size();

  double max = -__DBL_MAX__;
  int index;

  for (int i = 0; i < nfft; ++i) {
    if (std::abs(rx_config.CFO_spectrum.first[i]) > max) {
      max = std::abs(rx_config.CFO_spectrum.first[i]);
      index = i;
    }
  }

  double df = rx_config.CFO_spectrum.second[index] / rx_config.mod_order;
  double dw = -2 * M_PI * df / Fs;
  rx_config.post_cfo_signal.resize(rx_config.rx_samples.size());

  for (int i = 0; i < rx_config.rx_samples.size(); ++i) {
    rx_config.post_cfo_signal[i] =
        samples[i] * std::exp(std::complex<double>(0.0, dw * i));
  }

  // check post cfo spectrum
  for (int i = 0; i < rx_config.post_cfo_signal.size(); ++i) {
    pow_samples[i] =
        std::pow(rx_config.post_cfo_signal[i], rx_config.mod_order);
  }

  rx_config.post_CFO_spectrum = fft(pow_samples, Fs);

  // fine

  // double stepSize = 0.01 * 2 * M_PI / nfft;
  // double curPhase = 0;
  // double minStepSize = 1e-10;
  // bool done = false;
  // int k = 0;
  // int maxIter = 3000;

  // double cur_val;

  // while (!done && k < maxIter)
  // {
  //     double curVal = metric(samples, curPhase, rx_config.mod_order);

  //     double nextPhase = curPhase + stepSize;
  //     double nextVal = metric(samples, nextPhase, rx_config.mod_order);

  //     double grad = (nextVal - curVal) / stepSize;

  //     double newPhase = curPhase + stepSize * grad;

  //     if (std::abs(newPhase - curPhase) < 1e-6)
  //     {
  //         done = true;
  //     }

  //     if (stepSize >= minStepSize)
  //         stepSize *= 0.99;
  //     else
  //         stepSize = minStepSize;

  //     curPhase = newPhase;
  //     k++;
  // }

  // double fineEstimate = curPhase / rx_config.mod_order;

  // for (int i = 0; i < samples.size(); ++i)
  // {
  //     std::complex<double> rot = std::exp(std::complex<double>(0.0,
  //     -fineEstimate * (double)i)); rx_config.post_cfo_signal[i] *= rot;
  // }

  // rx_config.post_fine_CFO_spectrum = fft(rx_config.post_cfo_signal, Fs);
}
