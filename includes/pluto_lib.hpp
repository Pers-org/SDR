#pragma once

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*Init pluto*/
struct sdr_config_t {
  char *usb_uri;
  int buff_size;
  float tx_sample_rate;
  float tx_carrier_freq;
  float tx_gain;
  float rx_sample_rate;
  float rx_carrier_freq;
  float rx_gain;
  const size_t channels[1] = {0};
  int mode;
};

struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config);
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr,
                                    sdr_config_t *config, bool isRx);

void close_pluto_sdr(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream,
                     SoapySDRStream *txStream);

void start_rx(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream,
              int16_t *rx_buffer, int buff_size);

void start_tx(struct SoapySDRDevice *sdr, SoapySDRStream *txStream,
              SoapySDRStream *rxStream, int16_t *rx_buffer, int16_t *samples,
              int tx_samples_count, int buff_size);

// // Преобразование сэмплов из двух массивов (I[N], Q[N]) в вид Pluto
// (buff[N*2] =
// // {I, Q, I, Q, ..., I, Q})
// void transform_to_pluto_type_smples(std::vector<float> &I_part,
//                                     std::vector<float> &Q_part,
//                                     int16_t *buffer);
// void transform_from_pluto_type_samples(std::vector<float> &I_part,
//                                        std::vector<float> &Q_part,
//                                        int16_t *buffer);