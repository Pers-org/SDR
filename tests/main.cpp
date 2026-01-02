#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "../includes/general/subfuncs.hpp"
#include "../includes/pluto_lib/pluto_lib.hpp"

int main(int argc, char *argv[]) {

  /*check arguments*/
  if (argc != 4) {
    printf("Programm wait 3 argument:\n \
      1.char* pluto usb_uri (example: )\n \
      2.int rx or tx (0-rx, 1-tx)\n \
      3.int time to work (in seconds)");

    return 1;
  }

  /*tx or rx mode*/
  int mode = std::stoi(argv[2]);

  int time_to_work = std::stoi(argv[3]);

  /*create config for SDR*/
  sdr_config_t config;
  config.usb_uri = argv[1];
  config.buff_size = 1920;
  config.rx_carrier_freq = 734750e3;
  config.tx_carrier_freq = 734750e3;
  config.rx_sample_rate = 1e6;
  config.tx_sample_rate = 1e6;
  config.rx_gain = 25.0;
  config.tx_gain = -10.0;

  std::cout << config.usb_uri << std::endl;

  /*files for tx/rx samples*/
  char *rxdata = "../pcm/rxdata.pcm";
  char *txdata = "../pcm/txdata.pcm";

  /*setup pluto*/
  struct SoapySDRDevice *sdr = setup_pluto_sdr(&config);
  struct SoapySDRStream *rxStream = setup_stream(sdr, &config, 1);
  struct SoapySDRStream *txStream = setup_stream(sdr, &config, 0);

  /*create buffers*/
  int16_t tx_buffer[2 * config.buff_size];
  int16_t rx_buffer[2 * config.buff_size];

  /*if we want only rx*/
  if (mode == 0) {
    start_rx(sdr, rxStream, rx_buffer, config.buff_size, config.buff_size,
             rxdata, time_to_work);
  } else if (mode == 1) {
    std::vector<std::complex<int16_t>> complex_samples =
        read_pcm(std::string("../../pcm/tx_samples.pcm"));

    int uncomplex_samples_size = complex_samples.size() * 2;
    std::vector<int16_t> uncomplex_samples;
    uncomplex_samples.reserve(uncomplex_samples_size);

    for (int i = 0; i < complex_samples.size(); ++i) {
      uncomplex_samples.push_back(complex_samples[i].real());
      uncomplex_samples.push_back(complex_samples[i].imag());
    }

    start_tx(sdr, txStream, rxStream, rx_buffer, uncomplex_samples.data(),
             complex_samples.size(), config.buff_size, txdata, time_to_work);
  } else {
    printf("Invalid mode. Enter mode=0 or mode=1");
  }

  close_pluto_sdr(sdr, rxStream, txStream);

  return EXIT_SUCCESS;
}