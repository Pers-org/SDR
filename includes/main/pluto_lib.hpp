#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Config for init Pluto SDR
 */
struct sdr_config_t {
  char *usb_uri;   /**< uri address of pluto (use iio_info -a to see connected
                      plutos)*/
  int buffer_size; /**< count of I/Q samples, that SDR receive/send for one time
                    */
  int tx_sample_rate;
  int tx_carrier_freq;
  float tx_gain;
  int rx_sample_rate;
  int rx_carrier_freq;
  float rx_gain;
  int channels[1] = {0};
};

/**
 * @brief create sdr object
 *
 * @param config pluto config
 * @return pluto object
 */
struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config);

/**
 * @brief setup streams
 *
 * @param sdr sdr object
 * @param config sdr config
 * @param isRx 0-setup TX stream, 1-setup RX stream
 * @return stream object
 */
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr,
                                    sdr_config_t *config, bool isRx);

/**
 * @brief close streams, free memory. Finished work with pluto
 *
 * @param sdr sdr object
 * @param rxStream
 * @param txStream
 *
 * @return stream object
 */
void close_pluto_sdr(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream,
                     SoapySDRStream *txStream);

/**
 * @brief start receiving samples and write their in file
 *
 * @param sdr sdr object
 * @param rx_stream rx stream object
 * @param rx_buffer how many samples sdr receive for one time
 * @param buff_size how many samples sdr receive for one time
 * @param rx_mtu Max Transmission Unit
 * @param filename file, where will be writing receive samples
 * @param work_time time, that sdr will receive signal
 */
void start_rx(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream,
              int16_t *rx_buffer, int buff_size, int rx_mtu, char *filename,
              int work_time);

/**
 * @brief start transmit samples
 *
 * @param sdr_object sdr object
 * @param tx_stream
 * @param rx_stream
 * @param tx_samples samples that will transmited
 * @param tx_samples_count size of tx samples
 * @param rx_buffer
 * @param tx_buffer
 * @param filename file, where will be writing tx samples
 * @param work_time time, that sdr will tx signal
 */
void start_tx(struct SoapySDRDevice *sdr, SoapySDRStream *txStream,
              SoapySDRStream *rxStream, int16_t *tx_samples,
              int tx_samples_count, int16_t *rx_buffer, int16_t *tx_buffer,
              int buff_size, int work_time);