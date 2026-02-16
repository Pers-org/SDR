/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "../../../includes/TX/modulator.hpp"

std::vector<std::complex<double>>
modulator::QAM16(const std::vector<int16_t> &bits) {
  double norm_coeff = 1 / std::sqrt(10);

  std::vector<std::complex<double>> symbols;
  symbols.reserve(bits.size());

  for (size_t i = 0; i + 3 < bits.size(); i += 4) {
    double b0 = bits[i];
    double b1 = bits[i + 1];
    double b2 = bits[i + 2];
    double b3 = bits[i + 3];

    double I = (1 - 2 * b0) * (2 - (1 - 2 * b2));
    double Q = (1 - 2 * b1) * (2 - (1 - 2 * b3));

    symbols.push_back({norm_coeff * I, norm_coeff * Q});
  }

  return symbols;
}