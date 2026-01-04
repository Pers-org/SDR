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
  /** @brief check modulation order changing */
  int prev_mod_order;

  /**
   * @brief generate QAM modulation table. Using 3GPP TS 38.211 version 18.6.0
   * Release 18
   *
   * @param bits_per_symbol
   *
   * @return map, where key is bits, values is symbol
   */
  std::map<int16_t, std::complex<double>>
  QAM_mapper_table_generator(const int bits_per_symbol);

public:
  /** @brief variable with modulation table*/
  std::map<int16_t, std::complex<double>> QAM_mapper_table;

  modulator() { prev_mod_order = -1; }

  /**
   * @brief convert bits to symbols, using modulation table
   *
   * @param mod_order count of point on constellation
   * @param bits vector of bits
   *
   * @return complex symbols (I, Q)
   */
  std::vector<std::complex<double>>
  QAM_modulation(const int mod_order, const std::vector<int16_t> &bits);
};