/*
    This code read samples from .pcm file and convert samples to message
*/

#include <complex>
#include <fstream>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "../../includes/ImGUI_interface.h"
#include "../../includes/Receiver.hpp"
#include "../../includes/TX/modulator.hpp"
#include "../../includes/TX/overhead_encoder.hpp"
#include "../../includes/general/subfuncs.hpp"

void RX_proccesing(rx_cfg &rx_config) {

  std::vector<double> IR(rx_config.sps, 1); // matched filter IR

  /*RX object*/
  receiver RX;
  modulator modulator_;
  overhead_encoder overhead_encoder_;

  /*generate barker code*/
  // std::vector<int16_t> barker_code =
  //     overhead_encoder_.generate_barker_code(barker_code_size);

  /*barker code -> symbols*/
  // std::vector<std::complex<double>> barker_code_symb =
  //     modulator_.QAM_modulation(4, barker_code);

  /*RX work logic*/

  /*coarse freq sync*/
  // std::vector<std::complex<double>> cfo_symbols =
  //     RX.synchronizer_.coarse_freq_offset(samples2, barker_code_size);

  /*send samples to mathced filter to increase SNR*/
  rx_config.mf_samples_out = std::move(
      RX.mf_filter_.convolve(rx_config.rx_samples, IR, rx_config.sps));

  /*symbol sync (gardner scheme). Find offsets for each symbol*/
  std::vector<int16_t> offsets =
      RX.synchronizer_.gardner(rx_config.mf_samples_out, rx_config.sps,
                               rx_config.gardner_Kp, rx_config.gardner_BnTs);

  /*samples -> symbols*/
  rx_config.raw_symbols = RX.mf_filter_.downsampling(rx_config.mf_samples_out,
                                                     offsets, rx_config.sps);

  // /*get corr_coeffs (simulate correlation receiving)*/
  // std::vector<std::complex<double>> corr_coeffs =
  //     RX.synchronizer_.corr_receiving(symbols, barker_code_symb);

  // /*find peak of correlation (start packet)*/
  // int start_sync = RX.synchronizer_.find_sync_index(corr_coeffs);

  // /*slice symbols*/
  // std::vector<std::complex<double>> slice_symbols(symbols.begin() + 0,
  //                                                 symbols.end());

  // /*fine freq sync*/
  // std::vector<std::complex<double>> post_costas =
  //     RX.synchronizer_.costas_loop(slice_symbols);

  // /*cut barker*/
  // std::vector<std::complex<double>> wo_barker(
  //     post_costas.begin() + barker_code_size, post_costas.end());

  /*symbols -> true symbols (quantization)*/
  // std::vector<std::complex<double>> true_symbols =
  //     RX.demodulator_.QAM_quantizater(rx_config.raw_symbols,
  //     rx_config.mod_order);

  // /*true symbols -> bits*/
  // std::vector<int16_t> bits =
  //     RX.demodulator_.QAM_demodulator(true_symbols, rx_config.mod_order);
}