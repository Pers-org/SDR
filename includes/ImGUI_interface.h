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
  float gardner_BnTs;
  float gardner_Kp;

  // costas params
  float costas_Kp;
  float costas_Ki;

  // buffers
  std::vector<std::complex<int16_t>> rx_samples;
  std::vector<std::complex<double>> mf_samples_out;
  std::vector<std::complex<double>> raw_symbols;
};

void run_gui(tx_cfg &tx_config, rx_cfg &rx_config, sdr_config_t &sdr_config);
