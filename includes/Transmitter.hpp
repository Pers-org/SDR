#pragma once

#include "TX/coder.hpp"
#include "TX/modulator.hpp"
#include "TX/overhead_encoder.hpp"
#include "general/filter.hpp"

/**
 * @brief This class implements the processing logic in TX side (converting
 * message to samples)
 */
class transmitter {
public:
  filter filter_;       /**< give shape to samples (rectangle of RC)*/
  coder coder_;         /**< convert message to bits*/
  modulator modulator_; /**< generate modulation table, convert bits to symbol*/
  overhead_encoder
      overhead_encoder_; /**< generate overheads (CRC, sync sequence)*/
};
