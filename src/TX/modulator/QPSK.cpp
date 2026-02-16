/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "../../../includes/TX/modulator.hpp"

std::vector<std::complex<double>>
modulator::QPSK(const std::vector<int16_t> &bits) {
  double norm_coeff = 1 / std::sqrt(2);

  std::vector<std::complex<double>> symbols;
  symbols.reserve(bits.size());

  for (size_t i = 0; i + 1 < bits.size(); i += 2) {
    double I = 1 - 2 * bits[i];
    double Q = 1 - 2 * bits[i + 1];

    symbols.push_back({norm_coeff * I, norm_coeff * Q});
  }

  return symbols;
}