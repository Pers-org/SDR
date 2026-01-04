#pragma once

#include <complex>
#include <iostream>
#include <map>
#include <vector>

#include "complex_less.hpp"

/**
 * @brief This class convert symbols to bits
 */
class demodulator {
private:
  /**
   * @brief generate demodulation table [key=symbol, value=bits]. See 3GPP
   * TS 38.211 version 18.6.0 Release 18
   * @note I use table (map), because it look like a real system. The table is
   * generated once and no need to do calculations when converting each symbol
   * @param mod_order number of point in constelation (BPSK-2, QPSK-4, QAM16-16)
   *
   * @return demodulation table
   */
  std::map<std::complex<double>, int16_t, ComplexLess>
  QAM_demapper_table_generator(const int mod_order);

public:
  /**
   * @brief convert symbols to true symbols
   *
   * At the receiving side, the symbols are very distorted, but if the receiver
   * knows what the constellation should look like (knows the true points), then
   * it can calculate the distance between the received symbols and the true
   * symbols and determine which true symbol the received symbol belongs to.
   *
   * @param mod_order number of point in constelation (BPSK-2, QPSK-4, QAM16-16)
   *
   * @return vector of true symbols
   */
  std::vector<std::complex<double>>
  QAM_quantizater(const std::vector<std::complex<double>> &symbols,
                  const int mod_order);
  /**
   * @brief variable, storied demodulation table
   */
  std::map<std::complex<double>, int16_t, ComplexLess> QAM_demapper_table;
  /**
   * @brief converte symbols to bits
   *
   * @param symbols vector of complex symbols
   * @param mod_order number of point in constelation (BPSK-2, QPSK-4, QAM16-16)
   *
   * @return vector of bits
   */
  std::vector<int16_t>
  QAM_demodulator(const std::vector<std::complex<double>> &symbols,
                  const int mod_order);
};