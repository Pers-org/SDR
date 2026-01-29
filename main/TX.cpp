/*
    This code convert message to samples and write samples to .pcm file
*/

#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../includes/Transmitter.hpp"
#include "../includes/general/subfuncs.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {

  /*TX parameters*/
  const int SPS = 10; // samples per symbol
  const int barker_code_size = 13;
  std::string message(
      "Mr. and Mrs. Smith have one son and one daughter. The son's name is "
      "John. The daughter's name is Sarah. The Smiths live in a house. They "
      "have a living room. They watch TV in the living room. The father cooks "
      "food in the kitchen. They eat in the dining room. The house has two "
      "bedrooms. They sleep in the bedrooms. They keep their clothes in the "
      "closet. There is one bathroom. They brush their teeth in the bathroom. "
      "The house has a garden. John and Sarah play in the garden. They have a "
      "dog. John and Sarah like to play with the dog."); // tx message

  std::vector<double> impulse_response = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // pulse shaping filter impulse response

  std::string TX_log_file = "TX_log.txt";

  std::string TX_samples_file = "../pcm/tx_samples.pcm";

  /*tx object*/
  transmitter TX;

  /*create loger*/
  auto file_logger = spdlog::basic_logger_mt("file_logger", TX_log_file);

  /*TX work logic*/

  /*string -> bits*/
  std::vector<int16_t> bits = TX.coder_.ascii_str2bin(message);

  /*generate barker code*/
  // std::vector<int16_t> barker_code =
  //     TX.overhead_encoder_.generate_barker_code(barker_code_size);

  /*append sync sequence*/
  // std::vector<int16_t> overhead_bits =
  //     TX.overhead_encoder_.add_sync_seq_to_message(bits, barker_code);

  /*bits -> QAM symbols*/
  std::vector<std::complex<double>> symbols =
      TX.modulator_.QAM_modulation(4, bits);

  /*QAM symbols -> upsampling QAM symbols*/
  std::vector<std::complex<double>> ups_symbols =
      TX.filter_.upsampling(symbols, SPS);
  std::vector<std::complex<double>> samples =
      TX.filter_.convolve(ups_symbols, impulse_response, SPS);

  /*upsampling QAM symbols -> upscale samples (for pluto SDR)*/
  std::vector<std::complex<int16_t>> scale_samples = upscaling(samples);

  write_pcm(TX_samples_file, scale_samples);

  /*logging*/
  file_logger->info("############## TX PARAMETERS #################");

  file_logger->info("message: {}", message);
  file_logger->info("SPS: {}", SPS);
  file_logger->info("logs write to: {}", TX_log_file);
  file_logger->info("samples write to: {}", TX_samples_file);

  file_logger->info("psf impulse response: {}", vector2str(impulse_response));

  file_logger->info("QAM mapper table:\n{}",
                    map2str(TX.modulator_.QAM_mapper_table));

  file_logger->info("############## TX LOGIC #################");
  file_logger->info("bits: {}", vector2str(bits));
  // file_logger->info("barker code: {}", vector2str(barker_code));
  // file_logger->info("bits with barker code: {}", vector2str(overhead_bits));
  file_logger->info("symbols: {}", vector2str(symbols));
  file_logger->info("ups_symbols: {}", vector2str(ups_symbols));
  file_logger->info("samples: {}", vector2str(samples));
  file_logger->info("scale samples: {}", vector2str(scale_samples));

  return 0;
}