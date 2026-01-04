#include "../../../includes/TX/overhead_encoder.hpp"

std::vector<int16_t> overhead_encoder::add_sync_seq_to_message(
    const std::vector<int16_t> &bits, const std::vector<int16_t> &barker_code) {
  if (barker_code.size() == 0)
    return {};

  std::vector<int16_t> new_bits;

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < barker_code.size(); ++j) {
      new_bits.push_back(barker_code[j]);
    }
  }

  for (int i = 0; i < bits.size(); ++i) {
    new_bits.push_back(bits[i]);
  }

  return new_bits;
}