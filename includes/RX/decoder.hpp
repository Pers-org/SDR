#pragma once

#include <vector>

/**
 * @brief This class convert bits to message
 */
class decoder {
public:
  /**
   * @brief convert bits to ASCII string
   *
   * @warning Count of bits must be multiple of 8
   *
   * @param bits vector of bits
   *
   * @return ASCII string
   */
  std::string bin2str(const std::vector<int16_t> &bits);
};