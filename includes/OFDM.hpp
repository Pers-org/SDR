#include "ImGUI_interface.h"
#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

std::vector<std::vector<std::complex<double>>>
batched(const std::vector<std::complex<double>> &data, const int size);
std::vector<std::complex<double>>
batch_ifft(const std::vector<std::complex<double>> &data, int batch_size);
std::vector<std::complex<double>>
add_CP(const std::vector<std::complex<double>> &samples,
       const tx_cfg &tx_config);
std::vector<std::complex<int16_t>>
extract_OFDM_symbols(const std::vector<std::complex<int16_t>> &ofdm_samples,
                     const std::vector<int> &peaks, const int CP_size,
                     const int Nc);
std::vector<std::complex<double>>
batch_fft(const std::vector<std::complex<int16_t>> &data, int batch_size);
std::vector<double>
OFDM_corr_receive(const std::vector<std::complex<double>> &samples,
                  const int symb_size, const int CP_size);

std::complex<double>
corr(const std::vector<std::complex<int16_t>> &symbols,
     const std::vector<std::complex<int16_t>> &sync_seq_symb);

double norm_corr(const std::vector<std::complex<int16_t>> &symbols,
                 const std::vector<std::complex<int16_t>> &sync_seq_symb);

std::vector<double>
OFDM_corr_receive(const std::vector<std::complex<int16_t>> &samples,
                  const int symb_size, const int CP_size);