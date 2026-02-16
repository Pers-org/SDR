#pragma once

#include <complex>
#include <iostream>
#include <map>
#include <vector>

/**
 * @brief This class convert bits to symbols
 */
class modulator {
private:
  std::vector<std::complex<double>> BPSK(const std::vector<int16_t> &bits);
  std::vector<std::complex<double>> QPSK(const std::vector<int16_t> &bits);
  std::vector<std::complex<double>> QAM16(const std::vector<int16_t> &bits);

public:
  std::vector<std::complex<double>> QAM(const int modulation_order,
                                        const std::vector<int16_t> &bits);
};