#include "../../../includes/RX/synchronizer.hpp"
#include <complex>
#include <spdlog/spdlog.h>
#include <vector>

std::complex<double>
synchronizer::corr(const std::vector<std::complex<int16_t>> &symbols,
                   const std::vector<std::complex<int16_t>> &sync_seq_symb) {
  if (symbols.size() != sync_seq_symb.size()) {
    spdlog::error("Vectors must be the same size");
    return {0.0, 0.0};
  }

  std::complex<double> sum{0.0, 0.0};

  for (int i = 0; i < symbols.size(); ++i) {
    std::complex<double> x{static_cast<double>(symbols[i].real()),
                           static_cast<double>(symbols[i].imag())};
    std::complex<double> y{static_cast<double>(sync_seq_symb[i].real()),
                           static_cast<double>(sync_seq_symb[i].imag())};
    sum += x * std::conj(y);
  }

  return sum;
}

double synchronizer::norm_corr(
    const std::vector<std::complex<int16_t>> &symbols,
    const std::vector<std::complex<int16_t>> &sync_seq_symb) {
  std::complex<double> unnormal_cor = corr(symbols, sync_seq_symb);

  double norm_coeff_a = 0.0;
  double norm_coeff_b = 0.0;

  for (int i = 0; i < symbols.size(); ++i) {
    norm_coeff_a += std::norm(
        std::complex<double>(std::real(symbols[i]), std::imag(symbols[i])));
    norm_coeff_b += std::norm(std::complex<double>(
        std::real(sync_seq_symb[i]), std::imag(sync_seq_symb[i])));
  }

  return std::abs(unnormal_cor) / std::sqrt(norm_coeff_a * norm_coeff_b);
}

std::vector<double> synchronizer::OFDM_corr_receive(
    const std::vector<std::complex<int16_t>> &samples, const int symb_size,
    const int CP_size) {
  std::vector<double> corr_function;

  for (int i = 0; i < samples.size() - symb_size - CP_size + 1; ++i) {
    std::vector<std::complex<int16_t>> window1(samples.begin() + i,
                                               samples.begin() + i + CP_size);
    std::vector<std::complex<int16_t>> window2(samples.begin() + i + symb_size,
                                               samples.begin() + i + symb_size +
                                                   CP_size);
    corr_function.push_back(norm_corr(window1, window2));
  }

  return corr_function;
}

int synchronizer::find_sync_index(
    const std::vector<std::complex<double>> &corr_coeffs) {
  double max_val = -__DBL_MAX__;
  int index = -1;

  for (int i = 0; i < corr_coeffs.size(); ++i) {
    double cur_val = std::abs(corr_coeffs[i]);

    if (cur_val > max_val) {
      max_val = cur_val;
      index = i;
    }
  }

  return index;
}
