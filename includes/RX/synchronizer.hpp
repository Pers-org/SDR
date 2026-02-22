#pragma once

#include <complex>
#include <iostream>
#include <vector>

#include "../ImGUI_interface.h"
/**
 * @brief This class implements the symbol, coarse and fine frequency/phase sync
 */
class synchronizer {
private:
  /**
   * @brief calculate correlation b/w receive symbols and sync sequence
   *
   * @param symbols vector of complex receiving symbols
   * @param sync_seq_symb vector of complex sync sequence symbols
   *
   * @return correlation
   */
  std::complex<double>
  corr(const std::vector<std::complex<double>> &symbols,
       const std::vector<std::complex<double>> &sync_seq_symb);

  /**
   * @brief calculate normalized correlation b/w receive symbols and sync
   * sequence
   *
   * @param symbols vector of complex receiving symbols
   * @param sync_seq_symb vector of complex sync sequence symbols
   *
   * @return correlation coefficient [0;1]
   */
  double norm_corr(const std::vector<std::complex<double>> &symbols,
                   const std::vector<std::complex<double>> &sync_seq_symb);

public:
  /**
   * @brief coarse frequency sync
   *
   * In TX side at beginning of the packet append two equal sync sequence.
   * This method determines phase shift between sequences and aligns the phase
   * of the signal. I use barker code as sync sequence
   *
   * @param symbols vector of complex receiving symbols
   * @param barker_size size of barker code
   *
   * @return symbols with phase correction
   */
  void coarse_freq_offset(const std::vector<std::complex<double>> &samples,
                          rx_cfg &rx_config, int Fs);

  /**
   * @brief fine frequency sync
   *
   * Correct phase shift
   *
   * @param samples receiving samples
   *
   * @return symbols with phase correction
   */
  std::vector<std::complex<double>>
  costas_loop(const std::vector<std::complex<double>> &samples, const int L,
              const double BnTs, const double Kp, const int mod_order);
  /**
   * @brief symbol sync. This method find start of symbol
   *
   * Gardner scheme find start of symbols by calculating the derivative between
   * symbols
   *
   * @param samples receiving samples
   * @param L samples per symbol
   *
   * @return vector of shifts for each symbol
   */
  std::vector<int16_t> gardner(const std::vector<std::complex<double>> &samples,
                               int const L, float const Kp, float const BnTs);

  /**
   * @brief Calculate correlation function
   *
   * We use cycle shift on receiving symbols and calculate normalized
   * correlation coefficient for each shift
   *
   * @param symbols vector of complex receiving symbols
   * @param sync_symbols vector of complex sync sequence symbols
   *
   * @return vector of shifts for each symbol
   */
  std::vector<double>
  corr_receiving(const std::vector<std::complex<double>> &symbols,
                 const std::vector<std::complex<double>> &sync_symbols);

  /**
   * @brief Frame sync. Find start of sync sequence (start of sync_seq)
   *
   * This method find index of max value in vector
   *
   * @param corr_coeffs correlation funnction
   *
   * @return index - start sync_seq
   */
  int find_sync_index(const std::vector<std::complex<double>> &corr_coeffs);
};