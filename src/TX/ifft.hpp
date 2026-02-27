#pragma once

#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

std::vector<std::complex<double>>
batch_ifft(const std::vector<std::complex<double>> &data, const int batch_size);