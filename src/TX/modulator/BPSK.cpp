/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "../../../includes/TX/modulator.hpp"

std::vector<std::complex<double>>
modulator::BPSK(const std::vector<int16_t> &bits) {
  double norm_coeff = 1 / std::sqrt(2);

  std::vector<std::complex<double>> symbols;
  symbols.reserve(bits.size());

  for (int i = 0; i < bits.size(); ++i) {
    symbols.push_back(
        {norm_coeff * (1 - 2 * bits[i]), norm_coeff * (1 - 2 * bits[i])});
  }

  return symbols;
}