#include <complex>
#include <vector>

#include "../../../includes/general/filter.hpp"

std::vector<std::complex<double>>
filter::downsampling(const std::vector<std::complex<double>> &samples,
                     const std::vector<int16_t> &symb_offset, const int L) {

  std::vector<std::complex<double>> symbols;

  for (int i = 0; i < symb_offset.size(); ++i) {
    symbols.push_back(samples[symb_offset[i]]);
  }

  return symbols;
}

std::vector<std::complex<double>>
filter::downsampling(const std::vector<std::complex<int16_t>> &samples,
                     const std::vector<int16_t> &symb_offset, const int L) {

  std::vector<std::complex<double>> symbols;
  std::complex<double> tmp;
  for (int i = 0; i < symb_offset.size(); ++i) {
    tmp = {std::real(samples[symb_offset[i]]),
           std::imag(samples[symb_offset[i]])};
    symbols.push_back(tmp);
  }

  return symbols;
}