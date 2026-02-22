#pragma once

#include <complex>
#include <vector>

/**
 * @brief This class implements the filter logic
 */
class filter {
public:
  /**
   * @brief discrete convolve
   */
  std::vector<std::complex<double>>
  convolve(const std::vector<std::complex<double>> &samples,
           const std::vector<double> &impulse_response, const int L);

  std::vector<std::complex<double>>
  convolve(const std::vector<std::complex<int16_t>> &samples,
           const std::vector<double> &impulse_response, const int L);

  /**
   * @brief Add L-1 zeros between symbols
   *
   * @param symbols symbols after modulator
   *
   * @param L samples per symbol
   */
  std::vector<std::complex<double>>
  upsampling(const std::vector<std::complex<double>> &symbols, const int L);

  /**
   * @brief discrete convolve
   *
   * @param samples receiving samples
   * @param symb_offset vector of offsets for each symbol
   * @param L samples per symbol
   */
  std::vector<std::complex<double>>
  downsampling(const std::vector<std::complex<double>> &samples,
               const std::vector<int16_t> &symb_offset, const int L);

  std::vector<std::complex<double>>
  downsampling(const std::vector<std::complex<int16_t>> &samples,
               const std::vector<int16_t> &symb_offset, const int L);
};
