#include "../../includes/pluto_lib.hpp"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/*setup base settings for pluto SDR*/
struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config) {
  // args can be user defined or from the enumeration result
  SoapySDRKwargs args = {};

  char buffer_size[10]; // Allocate enough space

  /*define buffer_size as char-pointer*/
  sprintf(buffer_size, "%d", config->buff_size);

  SoapySDRKwargs_set(&args, "driver", "plutosdr");

  /*set uri*/
  SoapySDRKwargs_set(&args, "uri", config->usb_uri);

  SoapySDRKwargs_set(&args, "direct", "1");

  /*set buffer size (for pluto defualt value is 1920 samples)*/
  SoapySDRKwargs_set(&args, "timestamp_every", buffer_size);
  SoapySDRKwargs_set(&args, "loopback", "0");

  /*create sdr-object*/
  SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
  SoapySDRKwargs_clear(&args);

  if (sdr == NULL) {
    spdlog::error("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
    return NULL;
  }

  /*setup RX/TX device with checking errors*/
  if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0,
                                   config->rx_sample_rate) != 0) {
    printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
  }
  if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, config->rx_carrier_freq,
                                  NULL) != 0) {
    printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
  }
  if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0,
                                   config->tx_sample_rate) != 0) {
    printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
  }
  if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, config->tx_carrier_freq,
                                  NULL) != 0) {
    printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
  }
  printf("SoapySDRDevice_getFrequency tx: %lf\n",
         SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));

  return sdr;
}

/*setup RX or TX stream*/
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr,
                                    sdr_config_t *config, bool isRx) {
  size_t channel_count = sizeof(config->channels) / sizeof(config->channels[0]);
  printf("Channel count = %d\n", channel_count);
  SoapySDRStream *stream;

  /*if we want setup RX stream*/
  if (isRx) {
    /*create stream object*/
    stream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16,
                                        config->channels, channel_count, NULL);
    /*check pointer errors*/
    if (stream == NULL) {
      printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
      SoapySDRDevice_unmake(sdr); /*delete sdr object*/
      return NULL;
    }

    /*set gain*/
    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *config->channels,
                               config->rx_gain) != 0) {
      printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }

    // Get MTU (Maximum Transmission Unit) or buffer size
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, stream);
    printf("MTU - RX: %lu\n", rx_mtu);

    // activate streams
    SoapySDRDevice_activateStream(sdr, stream, 0, 0, 0); // start streaming

    printf("Streams are activated\n");

    /*if we want setup TX stream*/
  } else {
    /*create object*/
    stream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16,
                                        config->channels, channel_count, NULL);
    /*check pointer error*/
    if (stream == NULL) {
      printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
      SoapySDRDevice_unmake(sdr);
      return NULL;
    }

    /*set gain with check error*/
    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, *config->channels,
                               config->tx_gain) != 0) {
      printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());
    }

    /*Get MTU size*/
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, stream);
    printf("MTU - TX: %lu\n", tx_mtu);

    SoapySDRDevice_activateStream(sdr, stream, 0, 0, 0); // start streaming
  }
  return stream;
}