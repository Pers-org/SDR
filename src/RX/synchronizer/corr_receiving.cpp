#include <complex>
#include <vector>

#include "../../../includes/RX/synchronizer.hpp"

std::complex<double>
synchronizer::corr(const std::vector<std::complex<double>> &symbols,
                   const std::vector<std::complex<double>> &sync_seq_symb) {
  if (symbols.size() != sync_seq_symb.size()) {
    printf("Vector must be the same size\n");
    return {0.0, 0.0};
  }

  std::complex<double> result{0.0, 0.0};

  for (int i = 0; i < symbols.size(); ++i) {
    result += std::complex<double>(symbols[i]) *
              std::conj(std::complex<double>(sync_seq_symb[i]));
  }

  return result;
}

double synchronizer::norm_corr(
    const std::vector<std::complex<double>> &symbols,
    const std::vector<std::complex<double>> &sync_seq_symb) {
  std::complex<double> unnormal_cor = corr(symbols, sync_seq_symb);

  double norm_coeff_a = 0.0;
  double norm_coeff_b = 0.0;

  for (int i = 0; i < symbols.size(); ++i) {
    norm_coeff_a += std::norm(std::complex<double>(symbols[i]));
    norm_coeff_b += std::norm(std::complex<double>(sync_seq_symb[i]));
  }

  return std::abs(unnormal_cor) / std::sqrt(norm_coeff_a * norm_coeff_b);
}

std::vector<double> synchronizer::corr_receiving(
    const std::vector<std::complex<double>> &symbols,
    const std::vector<std::complex<double>> &sync_symbols) {
  std::vector<double> corr_coeffs;

  size_t N = symbols.size();
  size_t M = sync_symbols.size();

  if (N < M)
    return corr_coeffs;

  corr_coeffs.reserve(N - M + 1);

  for (size_t i = 0; i <= N - M; ++i) {
    std::complex<double> r{0.0, 0.0};
    double norm_a = 0.0;
    double norm_b = 0.0;

    for (size_t k = 0; k < M; ++k) {
      r += symbols[i + k] * std::conj(sync_symbols[k]);

      norm_a += std::norm(symbols[i + k]);
      norm_b += std::norm(sync_symbols[k]);
    }

    double denom = std::sqrt(norm_a * norm_b);
    corr_coeffs.push_back(denom == 0.0 ? 0.0 : std::abs(r) / denom);
  }

  return corr_coeffs;
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
