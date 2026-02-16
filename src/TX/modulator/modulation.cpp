/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "../../../includes/TX/modulator.hpp"

std::vector<std::complex<double>>
modulator::QAM(int modulation_order, const std::vector<int16_t> &bits) {
  if (modulation_order == 2)
    return BPSK(bits);
  if (modulation_order == 4)
    return QPSK(bits);
  if (modulation_order == 16)
    return QAM16(bits);

  spdlog::error("[modulation.cpp]: Invalid modulation order!");
  return {};
}