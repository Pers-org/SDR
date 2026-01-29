#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../../../includes/RX/decoder.hpp"

std::string decoder::bin2str(const std::vector<int16_t> &bits) {
  // if (bits.size() % 8 != 0) {
  //   std::cout << "Bits must have size % 8!\n";
  //   return {};
  // }

  std::string word;

  for (int i = 0; i < bits.size() - bits.size() % 8; i += 8) {
    uint8_t byte = 0;
    for (int j = 0; j < 8; ++j) {
      byte = (byte << 1) | bits[i + j];
    }
    word += static_cast<char>(byte);
  }

  return word;
}