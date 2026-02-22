#pragma once

#include <complex>
#include <vector>

#include "ImGUI_interface.h"
#include "pluto_lib.hpp"

struct tx_cfg {
  bool run; // for stop work

  int bitrate;
  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine
  std::vector<int16_t> bits;
  std::vector<std::complex<double>> symbols;
  std::vector<std::complex<int16_t>> tx_samples;
};

struct rx_cfg {
  bool run;

  int mod_order; // 2-BPSK, 4-QPSK, 16-QAM16
  int sps;       // samples per symbol
  int IR_type;   // 0-Rectangle, 1-Raised-Cosine

  // gardner params
  double gardner_BnTs;
  double gardner_Kp;

  // costas params
  double costas_Kp;
  double costas_BnTs;

  // buffers
  std::vector<std::complex<int16_t>> rx_samples;
  std::vector<std::complex<double>> mf_samples_out;
  std::vector<std::complex<double>> raw_symbols;
  std::vector<double> corr_func;
  std::pair<std::vector<std::complex<double>>, std::vector<double>> spectrum;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      CFO_spectrum;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      post_CFO_spectrum;
  std::vector<std::complex<double>> post_cfo_signal;
  std::pair<std::vector<std::complex<double>>, std::vector<double>>
      post_fine_CFO_spectrum;
  std::vector<std::complex<double>> post_costas;
};

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config, sdr_config_t &sdr_config);
