/*
    This code convert message to samples and write samples to .pcm file
*/

/*C++ data types and classes*/
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/*user functions*/
#include "../../includes/ImGUI_interface.h"
#include "../../includes/Transmitter.hpp"
#include "../../includes/general/subfuncs.hpp"

void TX_proccesing(tx_cfg &config) {

  int barker_code_size = 4;
  /*tx object*/
  transmitter TX;

  /*create rectangle IR*/
  std::vector<double> IR(config.sps, 1);

  /*TX work logic*/

  /*generate barker code*/
  std::vector<int16_t> barker_code =
      TX.overhead_encoder_.generate_barker_code(barker_code_size);

  /*append sync sequence*/
  std::vector<int16_t> overhead_bits =
      TX.overhead_encoder_.add_sync_seq_to_message(config.bits, barker_code);

  /*bits -> QAM symbols*/
  config.symbols = std::move(TX.modulator_.QAM(config.mod_order, config.bits));

  /*QAM symbols -> upsampling QAM symbols*/
  std::vector<std::complex<double>> ups_symbols =
      TX.filter_.upsampling(config.symbols, config.sps);

  std::vector<std::complex<double>> samples =
      TX.filter_.convolve(ups_symbols, IR, config.sps);

  /*upsampling QAM symbols -> upscale samples (for pluto SDR)*/
  config.tx_samples = std::move(upscaling(samples));
}