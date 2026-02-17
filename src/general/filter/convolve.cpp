#include "../../../includes/general/filter.hpp"

// samples -> rect samples

std::vector<std::complex<double>>
filter::convolve(const std::vector<std::complex<double>> &samples,
                 const std::vector<double> &impulse_response, const int L) {

  std::vector<std::complex<double>> filtering_samples(samples.size());

  for (int n = 0; n < samples.size(); ++n) {
    std::complex<double> tmp = 0;
    for (int m = 0; m < L; ++m) {
      if (n - m >= 0) {
        tmp += samples[n - m] * impulse_response[m];
      }
    }

    filtering_samples[n] = tmp;
  }

  return filtering_samples;
}

std::vector<std::complex<double>>
filter::convolve(const std::vector<std::complex<int16_t>> &samples,
                 const std::vector<double> &impulse_response, const int L) {

  std::vector<std::complex<double>> filtering_samples(samples.size());

  for (int n = 0; n < samples.size(); ++n) {
    std::complex<double> tmp = 0;
    for (int m = 0; m < L; ++m) {
      if (n - m >= 0) {
        tmp +=
            std::complex<double>(samples[n - m].real(), samples[n - m].imag()) *
            impulse_response[m];
      }
    }

    filtering_samples[n] = tmp;
  }

  return filtering_samples;
}