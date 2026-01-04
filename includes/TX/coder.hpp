#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief This class convert a message to bits
 */
class coder {
public:
  /**
   * @brief convert ASCII string to bits
   *
   *
   * @param message input ASCII string
   * @return vector of bits in MSB order
   */
  std::vector<int16_t> ascii_str2bin(const std::string &message);
};