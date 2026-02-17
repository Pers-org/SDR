#pragma once

#include "ImGUI_interface.h"
#include "RX/complex_less.hpp"
#include "RX/decoder.hpp"
#include "RX/demodulator.hpp"
#include "RX/synchronizer.hpp"
#include "general/filter.hpp"

/**
 * @brief This class implements the processing logic in RX side (converting
 * samples to message)
 */
class receiver {
public:
  filter mf_filter_;        /**< increase SNR on RX side*/
  decoder decoder_;         /**< convert bits to message*/
  demodulator demodulator_; /**< generate demodulation table, quantizes symbols,
                               convert symbols to bits*/
  synchronizer synchronizer_; /**< symbol sync (gardner scheme), coarse and fine
                                 frequency/phase sync (costas loop)*/
};

void RX_proccesing(rx_cfg &rx_config, sdr_config_t &sdr_config);
