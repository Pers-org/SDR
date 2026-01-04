#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../includes/Transmitter.hpp"
#include "../includes/general/logger.hpp"
#include "../includes/general/subfuncs.hpp"

int main(int argc, char *argv[]) {
  /*################### TX TEST #################*/

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

  /*object for logging*/
  logger TX_log(TX_log_file);

  /*TX work logic*/

  /*string -> bits*/
  std::vector<int16_t> bits = TX.coder_.ascii_str2bin(message);

  /*generate barker code*/
  std::vector<int16_t> barker_code =
      TX.overhead_encoder_.generate_barker_code(barker_code_size);

  /*append sync sequence*/
  std::vector<int16_t> overhead_bits =
      TX.overhead_encoder_.add_sync_seq_to_message(bits, barker_code);

  /*bits -> QAM symbols*/
  std::vector<std::complex<double>> symbols =
      TX.modulator_.QAM_modulation(4, overhead_bits);

  /*QAM symbols -> upsampling QAM symbols*/
  std::vector<std::complex<double>> ups_symbols =
      TX.filter_.upsampling(symbols, SPS);
  std::vector<std::complex<double>> samples =
      TX.filter_.convolve(ups_symbols, impulse_response, SPS);

  /*upsampling QAM symbols -> upscale samples (for pluto SDR)*/
  std::vector<std::complex<int16_t>> scale_samples = upscaling(samples);

  write_pcm(TX_samples_file, scale_samples);

  /*logging*/
  TX_log.label("############## TX PARAMETERS #################");

  TX_log.value("message", message);
  TX_log.value("SPS", SPS);
  TX_log.value("logs write to", TX_log_file);
  TX_log.value("samples write to", TX_samples_file);

  TX_log.vector("psf impulse response", impulse_response);

  TX_log.map("QAM mapper table", TX.modulator_.QAM_mapper_table);

  TX_log.label("############## TX LOGIC #################");
  TX_log.vector("bits", bits);
  TX_log.vector("barker code", barker_code);
  TX_log.vector("bits with barker code", overhead_bits);
  TX_log.vector("symbols", symbols);
  TX_log.vector("ups_symbols", ups_symbols);
  TX_log.vector("samples", samples);
  TX_log.vector("scale samples", scale_samples);

  return 0;
}