/*
    This code read samples from .pcm file and convert samples to message
*/

#include "fftw3.h"
#include <complex>
#include <fstream>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "../../includes/ImGUI_interface.h"
#include "../../includes/OFDM.hpp"
#include "../../includes/Receiver.hpp"
#include "../../includes/TX/modulator.hpp"
#include "../../includes/TX/overhead_encoder.hpp"
#include "../../includes/find_peaks.hpp"
#include "../../includes/general/subfuncs.hpp"
#include "./fft.hpp"

std::vector<std::complex<double>>
int_to_double(const std::vector<std::complex<int16_t>> &samples) {

  std::vector<std::complex<double>> result;
  result.resize(samples.size());

  for (int i = 0; i < samples.size(); ++i) {
    result[i] = {static_cast<double>(std::real(samples[i])),
                 static_cast<double>(std::imag(samples[i]))};
  }

  return result;
}

void RX_proccesing(rx_cfg &rx_config, sdr_config_t &sdr_config) {

  receiver RX;

  while (1) {

    if (!rx_config.OFDM) {

      std::vector<double> IR(rx_config.sps, 1); // matched filter IR
      int barker_code_size = 13;
      /*RX object*/
      modulator modulator_;
      overhead_encoder overhead_encoder_;

      std::vector<std::complex<double>> samples =
          int_to_double(rx_config.rx_samples);

      /*CFO*/

      RX.synchronizer_.coarse_freq_offset(samples, rx_config,
                                          sdr_config.rx_sample_rate);

      /*FFT*/

      rx_config.spectrum = fft(rx_config.rx_samples, sdr_config.rx_sample_rate);

      // /*generate barker code*/
      // std::vector<int16_t> barker_code =
      //     overhead_encoder_.generate_barker_code(barker_code_size);

      // /*barker code -> symbols*/
      // std::vector<std::complex<double>> barker_code_symb =
      //     modulator_.QAM(rx_config.mod_order, barker_code);

      /*RX work logic*/

      /*coarse freq sync*/
      // std::vector<std::complex<double>> cfo_symbols =
      //     RX.synchronizer_.coarse_freq_offset(samples2, barker_code_size);

      /*send samples to mathced filter to increase SNR*/
      rx_config.mf_samples_out = std::move(downscaler(RX.mf_filter_.convolve(
          rx_config.post_cfo_signal, IR, rx_config.sps)));

      /*symbol sync (gardner scheme). Find offsets for each symbol*/
      std::vector<int16_t> offsets = RX.synchronizer_.gardner(
          rx_config.mf_samples_out, rx_config.sps, rx_config.gardner_Kp,
          rx_config.gardner_BnTs);

      /*samples -> symbols*/
      rx_config.raw_symbols = RX.mf_filter_.downsampling(
          rx_config.mf_samples_out, offsets, rx_config.sps);

      rx_config.post_costas = RX.synchronizer_.costas_loop(
          rx_config.raw_symbols, 1, rx_config.costas_BnTs, rx_config.costas_Kp,
          rx_config.mod_order);

      // /*get corr_coeffs (simulate correlation receiving)*/
      // rx_config.corr_func =
      //     RX.synchronizer_.corr_receiving(rx_config.raw_symbols,
      //     barker_code_symb);

      // /*find peak of correlation (start packet)*/
      // int start_sync = RX.synchronizer_.find_sync_index(corr_coeffs);

      // /*slice symbols*/
      // std::vector<std::complex<double>> slice_symbols(symbols.begin() + 0,
      //                                                 symbols.end());

      // /*fine freq sync*/
      // std::vector<std::complex<double>> post_costas =
      //     RX.synchronizer_.costas_loop(slice_symbols);

      // /*cut barker*/
      // std::vector<std::complex<double>> wo_barker(
      //     post_costas.begin() + barker_code_size, post_costas.end());

      /*symbols -> true symbols (quantization)*/
      // std::vector<std::complex<double>> true_symbols =
      //     RX.demodulator_.QAM_quantizater(rx_config.raw_symbols,
      //     rx_config.mod_order);

      // /*true symbols -> bits*/
      // std::vector<int16_t> bits =
      //     RX.demodulator_.QAM_demodulator(true_symbols, rx_config.mod_order);
    } else {
      if (rx_config.rx_samples.size() != 1920) {
        continue;
      }

      std::vector<std::complex<double>> samples_d =
          int_to_double(rx_config.rx_samples);

      std::vector<double> cfo;

      rx_config.corr_func =
          OFDM_corr_receiving(samples_d, cfo, rx_config.Nc, rx_config.CP_size);

      // for (int i = 0; i < cfo.size(); ++i)
      // {
      //   std::cout << cfo[i] << " ";
      // }

      rx_config.spectrum = fft(samples_d, sdr_config.rx_sample_rate);

      // findPeaks::PeakConditions conditions;
      // conditions.set_height(0.9, 1.0);
      // // conditions.set_prominence(0.15);
      // // conditions.set_distance(rx_config.Nc / 2);
      // // conditions.set_width(2, rx_config.Nc / 4);
      // // conditions.set_rel_height(0.5);
      // // conditions.set_distance(rx_config.Nc); // At least 2 samples between

      // std::vector<int> peaks = findPeaks::find_peaks(rx_config.corr_func,
      // conditions);

      // std::cout << peaks.size() << "\n";

      // CFO_correction(samples_d, peaks, cfo, rx_config.CP_size, rx_config.Nc);

      // rx_config.corr_func = OFDM_corr_receiving(samples_d, cfo, rx_config.Nc,
      // rx_config.CP_size);

      // std::vector<std::complex<double>> rx_symbols = extract_OFDM_symbols(
      //     samples_d, peaks, rx_config.CP_size, rx_config.Nc);

      // std::cout << rx_symbols.size() << "  ";

      // // std::cout << rx_symbols.size() << " ";

      // rx_config.raw_symbols = batch_fft(rx_symbols, rx_config.Nc);

      // std::cout << rx_config.raw_symbols.size() << " ";

      // std::vector<int16_t> rx_bits(rx_config.raw_symbols.size());

      // for (int i = 0; i < rx_bits.size(); ++i)
      // {
      //   rx_bits[i] = std::real(rx_config.raw_symbols[i]) > 0 ? 0 : 1;
      // }
    }
  }
}