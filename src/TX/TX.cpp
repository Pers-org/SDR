/*
    This code convert message to samples and write samples to .pcm file
*/

/*C++ data types and classes*/
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*spdlog for logging*/
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

/*user functions*/
#include "../../includes/ImGUI_interface.h"
#include "../../includes/OFDM.hpp"
#include "../../includes/Transmitter.hpp"
#include "../../includes/general/subfuncs.hpp"

// class FFTW3
// {
// private:
//   int type;
//   int rank;
//   std::vector<int> dims;
//   int batch_count;

//   fftw_plan plan = nullptr;

//   void create_plan(fftw_complex *in, fftw_complex *out)
//   {
//     if (plan)
//       fftw_destroy_plan(plan);

//     int sign = (type == 0) ? FFTW_BACKWARD : FFTW_FORWARD;

//     plan = fftw_plan_many_dft(
//         rank,
//         dims.data(),
//         batch_count,
//         in, nullptr, 1, dims[0],
//         out, nullptr, 1, dims[0],
//         sign,
//         FFTW_ESTIMATE);

//     if (!plan)
//       throw std::runtime_error("FFTW plan creation failed");
//   }

// public:
//   FFTW3(int _type,
//         int _rank,
//         const std::vector<int> &_dims,
//         int _batch_count)
//       : type(_type),
//         rank(_rank),
//         dims(_dims),
//         batch_count(_batch_count)
//   {
//     if (type != 0 && type != 1)
//       spdlog::error("Invalid FFT type");

//     if (dims.empty())
//       spdlog::error("Invalid FFT dimensions");
//   }

//   ~FFTW3()
//   {
//     if (plan)
//       fftw_destroy_plan(plan);
//   }

//   void execute(std::vector<std::complex<double>> &in_vec,
//                std::vector<std::complex<double>> &out_vec)
//   {
//     auto *in = reinterpret_cast<fftw_complex *>(in_vec.data());
//     auto *out = reinterpret_cast<fftw_complex *>(out_vec.data());

//     if (!plan)
//       create_plan(in, out);

//     fftw_execute_dft(plan, in, out);
//   }

//   void reconfigure(int _type,
//                    int _rank,
//                    const std::vector<int> &_dims,
//                    int _batch_count)
//   {
//     type = _type;
//     rank = _rank;
//     dims = _dims;
//     batch_count = _batch_count;

//     if (plan)
//     {
//       fftw_destroy_plan(plan);
//       plan = nullptr;
//     }
//   }
// };

void bits_gen(const int N, std::vector<uint8_t> &bits) {
  bits.clear();

  bits.resize(N);

  for (int i = 0; i < N; ++i) {
    bits[i] = rand() % 2;
  }
}

void BPSK(const std::vector<uint8_t> &bits,
          std::vector<std::complex<double>> &out) {

  double norm_coeff = 1 / std::sqrt(2);

  // out.clear();
  out.resize(bits.size());

  for (int i = 0; i < bits.size(); ++i) {
    out[i] = {norm_coeff * (1 - 2 * bits[i]), norm_coeff * (1 - 2 * bits[i])};
  }
}

void upscaling(const std::vector<std::complex<double>> &samples,
               std::vector<std::complex<int16_t>> &out) {
  out.clear();
  out.resize(samples.size());

  for (int i = 0; i < samples.size(); i++) {
    int16_t re = static_cast<int16_t>(samples[i].real() * 1500) << 4;
    int16_t im = static_cast<int16_t>(samples[i].imag() * 1500) << 4;

    out[i] = {re, im};
  }
}

void TX_proccesing(tx_cfg &config, const sdr_config_t &sdr_cfg) {
  /*tx object*/
  transmitter TX;

  /*Init FFTW3 object for FFT/IFFT*/
  int batch_size[] = {config.Nc};
  int batch_count = sdr_cfg.buff_size / batch_size[0];

  std::vector<std::complex<double>> ofdm_signal;
  std::vector<std::complex<double>> ifft_out;

  int bits_per_symbol;
  int N;

  while (1) {

    // if (!config.OFDM)
    // {
    //   int N = (sdr_cfg.buff_size / config.sps) * config.mod_order;

    //   bits_gen(N, config.bits);

    //   int barker_code_size = 4;

    //   /*create rectangle IR*/
    //   std::vector<double> IR(config.sps, 1);

    //   /*TX work logic*/

    //   /*generate barker code*/
    //   std::vector<int16_t> barker_code =
    //       TX.overhead_encoder_.generate_barker_code(barker_code_size);

    //   /*append sync sequence*/
    //   std::vector<int16_t> overhead_bits =
    //       TX.overhead_encoder_.add_sync_seq_to_message(config.bits,
    //                                                    barker_code);

    //   /*bits -> QAM symbols*/
    //   config.symbols =
    //       std::move(TX.modulator_.QAM(config.mod_order, config.bits));

    //   /*QAM symbols -> upsampling QAM symbols*/
    //   std::vector<std::complex<double>> ups_symbols =
    //       TX.filter_.upsampling(config.symbols, config.sps);

    //   std::vector<std::complex<double>> samples =
    //       TX.filter_.convolve(ups_symbols, IR, config.sps);

    //   /*upsampling QAM symbols -> upscale samples (for pluto SDR)*/
    //   config.tx_samples = std::move(upscaling(samples));
    // }

    /*update params*/
    batch_size[0] = config.Nc;
    batch_count = sdr_cfg.buff_size / batch_size[0];

    /*calculate bits count (SDR buffer have restrictions)*/
    bits_per_symbol = static_cast<int>(std::log2(config.mod_order));
    N = config.Nc * config.count_OFDM_symb * bits_per_symbol;

    /*generate bits*/
    bits_gen(N, config.bits);

    /*bits -> QAM symbols*/
    BPSK(config.bits, config.symbols);

    /*QAM symbols -> IFFT -> OFDM signal*/
    batch_ifft(config.symbols, ifft_out, batch_size[0]);

    ofdm_signal = add_CP(ifft_out, config);

    // upscaling(ofdm_signal, config.tx_samples);
  }
}