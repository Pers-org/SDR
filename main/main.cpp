/*
  This code work with Pluto SDR and ImGUI. He init and setup SDR,
  receive/transmit samples and render GUI in other thread
*/

/*ImGUI*/
#include <GL/glew.h>
#include <SDL2/SDL.h>

/*SDR*/
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

/*C++ data types and classes*/
#include <complex.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/*user functions*/
#include "../includes/ImGUI_interface.h"
#include "../includes/Receiver.hpp"
#include "../includes/Transmitter.hpp"
#include "../includes/general/subfuncs.hpp"
#include "../includes/pluto_lib.hpp"

/*ImGUI and ImPLot*/
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

void SDR_run(sdr_config_t &sdr_config, tx_cfg &tx_config, rx_cfg &rx_config) {
  /*setup pluto*/
  struct SoapySDRDevice *sdr = setup_pluto_sdr(&sdr_config);
  struct SoapySDRStream *rxStream = setup_stream(sdr, &sdr_config, 1);
  struct SoapySDRStream *txStream = setup_stream(sdr, &sdr_config, 0);

  /*create buffers*/
  int16_t tx_buffer[2 * sdr_config.buff_size];

  float prev_tx_samplerate = sdr_config.tx_sample_rate;
  float prev_rx_samplerate = sdr_config.rx_sample_rate;

  float prev_rx_freq = sdr_config.rx_sample_rate;
  float prev_tx_freq = sdr_config.tx_sample_rate;

  float prev_rx_gain = sdr_config.rx_gain;
  float prev_tx_gain = sdr_config.tx_gain;

  /*if we want only rx*/
  if (sdr_config.mode == 0) {
    const long timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)

    long long last_time = 0;

    void *rx_buffs[] = {rx_config.rx_samples.data()};
    int flags;        // flags set by receive operation
    long long timeNs; // timestamp for receive buffer

    while (rx_config.run) {
      if (sdr_config.rx_sample_rate != prev_rx_samplerate) {
        prev_rx_samplerate = sdr_config.rx_sample_rate;

        if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0,
                                         sdr_config.rx_sample_rate) != 0) {
          spdlog::error("setSampleRate rx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_sample_rate != prev_tx_samplerate) {
        prev_tx_samplerate = sdr_config.tx_sample_rate;

        if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0,
                                         sdr_config.tx_sample_rate) != 0) {
          spdlog::error("setSampleRate tx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.rx_carrier_freq != prev_rx_freq) {

        prev_rx_freq = sdr_config.rx_carrier_freq;

        if (SoapySDRDevice_setFrequency(
                sdr, SOAPY_SDR_RX, 0, sdr_config.rx_carrier_freq, NULL) != 0) {
          spdlog::error("setFrequency rx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_carrier_freq != prev_tx_freq) {
        prev_tx_freq = sdr_config.tx_carrier_freq;
        if (SoapySDRDevice_setFrequency(
                sdr, SOAPY_SDR_TX, 0, sdr_config.tx_carrier_freq, NULL) != 0) {
          spdlog::error("setFrequency tx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.rx_gain != prev_rx_gain) {
        prev_rx_gain = sdr_config.rx_gain;

        if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *sdr_config.channels,
                                   sdr_config.rx_gain) != 0) {
          spdlog::error("setGain rx fail: %s\n", SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_gain != prev_tx_gain) {
        prev_tx_gain = sdr_config.tx_gain;

        if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *sdr_config.channels,
                                   sdr_config.tx_gain) != 0) {
          spdlog::error("setGain rx fail: %s\n", SoapySDRDevice_lastError());
        }
      }

      // считали буффер RX, записали его в rx_buffer
      SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, sdr_config.buff_size,
                                &flags, &timeNs, timeoutUs);

      RX_proccesing(rx_config, sdr_config);
    }
  }

  else if (sdr_config.mode == 1) {

    /*Мы делаем прием во время передачи, потому что pluto SDR хочет получать
    время отправки, а чтобы его высчитать (у нас на 4 мс в будущее), нужно
    выполнить прием данных*/

    /*кол-во бит в пакете*/

    const long timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)

    /*get current time*/
    time_t start = time(NULL);
    long long last_time = 0;

    void *rx_buffs[] = {rx_config.rx_samples.data()};
    int flags;        // flags set by receive operation
    long long timeNs; // timestamp for receive buffer

    /*формируем отправляемый буфер путем смещения в нашем массиве семплов*/

    /*для отладки пишем отправляемые семплы в файл*/
    // fwrite(samples + offset, 2 * buff_size * sizeof(int16_t), 1, txdata);

    flags = SOAPY_SDR_HAS_TIME;

    /*функция для отправки*/
    while (tx_config.run) {
      if (sdr_config.rx_sample_rate != prev_rx_samplerate) {
        prev_rx_samplerate = sdr_config.rx_sample_rate;

        if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0,
                                         sdr_config.rx_sample_rate) != 0) {
          spdlog::error("setSampleRate rx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_sample_rate != prev_tx_samplerate) {
        prev_tx_samplerate = sdr_config.tx_sample_rate;

        if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0,
                                         sdr_config.tx_sample_rate) != 0) {
          spdlog::error("setSampleRate tx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.rx_carrier_freq != prev_rx_freq) {

        prev_rx_freq = sdr_config.rx_carrier_freq;

        if (SoapySDRDevice_setFrequency(
                sdr, SOAPY_SDR_RX, 0, sdr_config.rx_carrier_freq, NULL) != 0) {
          spdlog::error("setFrequency rx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_carrier_freq != prev_tx_freq) {
        prev_tx_freq = sdr_config.tx_carrier_freq;
        if (SoapySDRDevice_setFrequency(
                sdr, SOAPY_SDR_TX, 0, sdr_config.tx_carrier_freq, NULL) != 0) {
          spdlog::error("setFrequency tx fail: %s\n",
                        SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.rx_gain != prev_rx_gain) {
        prev_rx_gain = sdr_config.rx_gain;

        if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *sdr_config.channels,
                                   sdr_config.rx_gain) != 0) {
          spdlog::error("setGain rx fail: %s\n", SoapySDRDevice_lastError());
        }
      }

      if (sdr_config.tx_gain != prev_tx_gain) {
        prev_tx_gain = sdr_config.tx_gain;

        if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *sdr_config.channels,
                                   sdr_config.tx_gain) != 0) {
          spdlog::error("setGain rx fail: %s\n", SoapySDRDevice_lastError());
        }
      }

      int N = (sdr_config.buff_size / tx_config.sps) * tx_config.mod_order;

      tx_config.bits = std::move(bits_gen(N));

      TX_proccesing(tx_config);

      // считали буффер RX, записали его в rx_buffer
      int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs,
                                         sdr_config.buff_size, &flags, &timeNs,
                                         timeoutUs);

      // RX_proccesing(rx_config, sdr_config);

      last_time = timeNs;

      // Переменная для времени отправки сэмплов относительно текущего приема
      long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее

      void *tx_buffs[] = {tx_config.tx_samples.data()};

      int st = SoapySDRDevice_writeStream(
          sdr, txStream, (const void *const *)tx_buffs, sdr_config.buff_size,
          &flags, tx_time, timeoutUs);

      /*проверка ошибки отправки*/
      if ((size_t)st != sdr_config.buff_size) {
        printf("TX Failed: %in", st);
      }
    }
  } else {
    printf("Invalid mode. Enter mode=0 or mode=1");
  }

  close_pluto_sdr(sdr, rxStream, txStream);
}

int main(int argc, char *argv[]) {

  int mode = std::atoi(argv[2]);

  /*init SDR config*/
  sdr_config_t sdr_config;
  sdr_config.usb_uri = argv[1];
  sdr_config.buff_size = 1920;
  sdr_config.rx_carrier_freq = 800e6;
  sdr_config.tx_carrier_freq = 800e6;
  sdr_config.rx_sample_rate = 1e6;
  sdr_config.tx_sample_rate = 1e6;
  sdr_config.rx_gain = 20;
  sdr_config.tx_gain = 80;
  sdr_config.mode = mode;

  /*init TX config*/
  tx_cfg tx_config;
  tx_config.run = true;
  tx_config.sps = 10;
  tx_config.mod_order = 2;
  tx_config.bitrate = static_cast<int>(sdr_config.tx_sample_rate /
                                       tx_config.sps * tx_config.mod_order);
  tx_config.IR_type = 0;
  tx_config.tx_samples.resize(sdr_config.buff_size);

  /*init RX config*/
  rx_cfg rx_config;
  rx_config.run = true;
  rx_config.costas_BnTs = 0.5;
  rx_config.costas_Kp = 1;
  rx_config.gardner_BnTs = 0.5;
  rx_config.gardner_Kp = 1;
  rx_config.IR_type = 1;
  rx_config.mod_order = 2;
  rx_config.sps = 10;
  rx_config.rx_samples.resize(sdr_config.buff_size);

  std::thread gui_thread(run_gui, std::ref(tx_config), std::ref(rx_config),
                         std::ref(sdr_config));

  std::thread sdr_thread(SDR_run, std::ref(sdr_config), std::ref(tx_config),
                         std::ref(rx_config));
  gui_thread.join();
  sdr_thread.join();

  return 0;
}