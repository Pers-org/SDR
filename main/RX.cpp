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

#include "../includes/Receiver.hpp"
#include "../includes/TX/modulator.hpp"
#include "../includes/TX/overhead_encoder.hpp"
#include "../includes/general/subfuncs.hpp"

int main(int argc, char *argv[]) {
  /*RX parameters*/
  const int mod_order = 4; // point on constellation
  const int SPS = 10;      // samples per symbol
  const int barker_code_size = 13;

  std::vector<double> impulse_response = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // mathced filter impulse response

  std::string RX_log_file = "RX_log.txt";
  std::string RX_samples_file = "../pcm/tx_samples.pcm";

  /*RX object*/
  receiver RX;
  modulator modulator_;
  overhead_encoder overhead_encoder_;

  /*object for logging*/
  auto file_logger = spdlog::basic_logger_mt("file_logger", RX_log_file);

  /*generate barker code*/
  std::vector<int16_t> barker_code =
      overhead_encoder_.generate_barker_code(barker_code_size);

  /*barker code -> symbols*/
  std::vector<std::complex<double>> barker_code_symb =
      modulator_.QAM_modulation(4, barker_code);

  /*RX work logic*/

  /*read samples from file*/
  std::vector<std::complex<int16_t>> samples = read_pcm(RX_samples_file);

  /*coarse freq sync*/
  std::vector<std::complex<double>> cfo_symbols =
      RX.synchronizer_.coarse_freq_offset(samples, barker_code_size);

  /*send samples to mathced filter to increase SNR*/
  std::vector<std::complex<double>> mf_samples =
      RX.mf_filter_.convolve(cfo_symbols, impulse_response, SPS);

  /*symbol sync (gardner scheme). Find offsets for each symbol*/
  std::vector<int16_t> offsets = RX.synchronizer_.gardner(mf_samples, SPS);

  /*samples -> symbols*/
  std::vector<std::complex<double>> symbols =
      RX.mf_filter_.downsampling(mf_samples, offsets, SPS);

  /*get corr_coeffs (simulate correlation receiving)*/
  std::vector<std::complex<double>> corr_coeffs =
      RX.synchronizer_.corr_receiving(symbols, barker_code_symb);

  /*find peak of correlation (start packet)*/
  int start_sync = RX.synchronizer_.find_sync_index(corr_coeffs);

  /*slice symbols*/
  std::vector<std::complex<double>> slice_symbols(symbols.begin() + 0,
                                                  symbols.end());

  /*fine freq sync*/
  std::vector<std::complex<double>> post_costas =
      RX.synchronizer_.costas_loop(slice_symbols);

  /*cut barker*/
  std::vector<std::complex<double>> wo_barker(
      post_costas.begin() + barker_code_size, post_costas.end());

  /*symbols -> true symbols (quantization)*/
  std::vector<std::complex<double>> true_symbols =
      RX.demodulator_.QAM_quantizater(wo_barker, mod_order);

  /*true symbols -> bits*/
  std::vector<int16_t> bits =
      RX.demodulator_.QAM_demodulator(true_symbols, mod_order);

  /*bits -> message*/
  std::string message = RX.decoder_.bin2str(bits);

  /*logging*/
  file_logger->info("#################### RX_PARAMETERS ###################");

  file_logger->info("SPS: {}", SPS);
  file_logger->info("modulation order: {}", mod_order);
  file_logger->info("logs write in: {}", RX_log_file);
  file_logger->info("samples read from: {}", RX_samples_file);

  file_logger->info("QAM demapper table:\n{}",
                    map2str(RX.demodulator_.QAM_demapper_table));

  file_logger->info("mf impulse response", vector2str(impulse_response));

  file_logger->info("#################### RX_LOGIC ###################");

  file_logger->info("samples: {}", vector2str(samples));
  file_logger->info("mf_samples: {}", vector2str(mf_samples));
  file_logger->info("offsets: {}", vector2str(offsets));
  file_logger->info("symbols: {}", vector2str(symbols));
  file_logger->info("barker code: {}", vector2str(barker_code));
  file_logger->info("barker symbols: {}", vector2str(barker_code_symb));
  file_logger->info("correlation coefficient: {}", vector2str(corr_coeffs));

  file_logger->info("start sync seq index: {}", start_sync);

  file_logger->info("slice symbols: {}", vector2str(slice_symbols));
  file_logger->info("post cfo symbols: {}", vector2str(cfo_symbols));

  file_logger->info("post_costas: {}", vector2str(post_costas));
  file_logger->info("true_symbols: {}", vector2str(true_symbols));
  file_logger->info("bits: {}", vector2str(bits));

  file_logger->info("message: {}", message);

  return 0;
}