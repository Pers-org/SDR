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
