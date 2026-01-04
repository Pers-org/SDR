#pragma once

#include <iostream>
#include <vector>
/**
 * @brief append overheads to packet (CRC, sync_seq)
 */
class overhead_encoder {
public:
  /** @brief check modulation order changing */
  int prev_mod_order;

  /**
   * @brief generate barker code
   *
   * @param len len of barker code (supporting only 2,3,4,5,7,11,13)
   *
   * @return vector of bits
   */
  std::vector<int16_t> generate_barker_code(const int len);

  /**
   * @brief append sync sequence to start of packet
   *
   * @param bits vector of bits, where you want append barker code
   * @param sync_seq sync sequence (barker, gold)
   *
   * @return new packet (vector of bits)
   */
  std::vector<int16_t>
  add_sync_seq_to_message(const std::vector<int16_t> &bits,
                          const std::vector<int16_t> &sync_seq);

  /*TODO: add CRC to message*/
};