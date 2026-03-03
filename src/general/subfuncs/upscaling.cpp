
#include "../../../includes/general/subfuncs.hpp"
#include <complex>
#include <vector>

// upscaling symbols for pluto SDR (12-bit DAC/ADC)
std::vector<std::complex<int16_t>>
upscaling(const std::vector<std::complex<double>> &psf_symbols) {

  std::vector<std::complex<int16_t>> samples(psf_symbols.size());

  for (int i = 0; i < psf_symbols.size(); i++) {
    int16_t I = static_cast<int16_t>(psf_symbols[i].real() * 1500) << 4;
    int16_t Q = static_cast<int16_t>(psf_symbols[i].imag() * 1500) << 4;

    samples[i] = {I * 1, Q * 1};
  }

  return samples;
}
