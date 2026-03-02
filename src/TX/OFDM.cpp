#include "../../includes/ImGUI_interface.h"
#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

// std::vector<std::vector<std::complex<double>>> batched(const
// std::vector<std::complex<double>> &data, const int size)
// {
//   if (data.size() % size != 0)
//   {
//     spdlog::error("[OFDM.cpp]: data size % batch size != 0!");
//     return {};
//   }

//   const int batch_count = data.size() / size;
//   std::vector<std::vector<std::complex<double>>> batches;
//   batches.reserve(batch_count);

//   for (int i = 0; i < batch_count; ++i)
//   {
//     auto start = data.cbegin() + (i * size);
//     auto end = data.begin() + ((i + 1) * size);

//     batches.emplace_back(std::vector<std::complex<double>>(start, end));
//   }

//   return batches;
// }

std::vector<std::complex<double>>
batch_ifft(const std::vector<std::complex<double>> &data, int batch_size) {
  const int N = data.size();

  if (batch_size <= 0 || N % batch_size != 0)
    return {};

  const int howmany = N / batch_size;
  const int n[] = {batch_size};

  fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

  std::memcpy(in, data.data(), sizeof(fftw_complex) * N);

  fftw_plan plan =
      fftw_plan_many_dft(1, n, howmany, in, nullptr, 1, batch_size, out,
                         nullptr, 1, batch_size, FFTW_BACKWARD, FFTW_ESTIMATE);

  if (!plan) {
    fftw_free(in);
    fftw_free(out);
    return {};
  }

  fftw_execute(plan);

  std::vector<std::complex<double>> result(N);

  for (int i = 0; i < N; ++i) {
    result[i] = {out[i][0], out[i][1]};
    result[i] /= batch_size;
  }

  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return result;
}

std::vector<std::complex<double>>
add_CP(const std::vector<std::complex<double>> &samples,
       const tx_cfg &tx_config) {
  std::vector<std::complex<double>> result(
      samples.size() + tx_config.count_OFDM_symb * tx_config.CP_size);

  for (int i = 0; i < tx_config.count_OFDM_symb; ++i) {
    auto dst = result.data() + i * (tx_config.CP_size + tx_config.Nc);
    auto src = samples.data() + i * tx_config.Nc;

    std::memcpy(dst, src + (tx_config.Nc - tx_config.CP_size),
                tx_config.CP_size * sizeof(std::complex<double>));

    std::memcpy(dst + tx_config.CP_size, src,
                tx_config.Nc * sizeof(std::complex<double>));
  }

  return result;
}

std::vector<std::complex<double>> BPSK(const std::vector<int16_t> &bits) {
  double norm_coeff = 1 / std::sqrt(2);

  std::vector<std::complex<double>> symbols;
  symbols.reserve(bits.size());

  for (int i = 0; i < bits.size(); ++i) {
    symbols.push_back(
        {norm_coeff * (1 - 2 * bits[i]), norm_coeff * (1 - 2 * bits[i])});
  }

  return symbols;
}

// std::complex<double> corr(const std::vector<std::complex<int16_t>> &symbols,
//                           const std::vector<std::complex<int16_t>>
//                           &sync_seq_symb)
// {
//   if (symbols.size() != sync_seq_symb.size())
//     return {0.0, 0.0};

//   std::complex<double> sum{0.0, 0.0};

//   for (size_t i = 0; i < symbols.size(); ++i)
//   {
//     std::complex<double> x(symbols[i].real(), symbols[i].imag());
//     std::complex<double> y(sync_seq_symb[i].real(), sync_seq_symb[i].imag());
//     sum += x * std::conj(y);
//   }

//   return sum;
// }

// double norm_corr(const std::vector<std::complex<int16_t>> &symbols,
//                  const std::vector<std::complex<int16_t>> &sync_seq_symb)
// {
//   auto unnorm = corr(symbols, sync_seq_symb);

//   double norm_a = 0.0, norm_b = 0.0;
//   for (size_t i = 0; i < symbols.size(); ++i)
//   {
//     std::complex<double> x(symbols[i].real(), symbols[i].imag());
//     std::complex<double> y(sync_seq_symb[i].real(), sync_seq_symb[i].imag());
//     norm_a += std::norm(x);
//     norm_b += std::norm(y);
//   }

//   double norm_coeff = std::sqrt(norm_a * norm_b);
//   if (norm_coeff < 1e-12)
//     return 0.0;

//   return std::abs(unnorm / norm_coeff);
// }

#include <cmath>
#include <complex>
#include <vector>

std::vector<double>
OFDM_corr_receiving(const std::vector<std::complex<double>> &rx,
                    std::vector<double> &cfo, int N, int Lcp) {
  std::vector<double> corr_func;

  const int L = rx.size() - N - Lcp;

  std::complex<double> corr = 0;

  std::vector<std::complex<double>> R(L);
  std::vector<double> A = {0};
  std::vector<double> B = {0};
  // cfo.reserve(L);

  for (int k = 0; k < Lcp; ++k) {
    corr += rx[k] * std::conj(rx[k + N]);
    A[0] += std::abs(rx[k]) * std::abs(rx[k]);
    B[0] += std::abs(rx[k + N]) * std::abs(rx[k + N]);
  }

  // cfo.push_back(std::arg(corr) / (2 * M_PI));

  R.push_back(corr);

  double new_a;
  double new_b;

  corr_func.push_back(std::abs(R.back() / std::sqrt(A.back() * B.back())));

  for (int k = 1; k < L; ++k) {
    std::complex<double> newR =
        R.back() - rx[k - 1] * std::conj(rx[k + N - 1]) +
        rx[k + Lcp - 1] * std::conj(rx[k + N + Lcp - 1]);

    // cfo.push_back(std::arg(newR) / (2 * M_PI));

    R.push_back(newR);

    new_a = A.back() - std::norm(rx[k - 1]) + std::norm(rx[k + Lcp - 1]);

    A.push_back(new_a);

    new_b =
        B.back() - std::norm(rx[k + N - 1]) + std::norm(rx[k + N + Lcp - 1]);

    B.push_back(new_b);

    corr_func.push_back(std::abs(R.back() / std::sqrt(A.back() * B.back())));
  }

  return corr_func;
}

std::vector<std::complex<double>>
batch_fft(const std::vector<std::complex<double>> &data, int batch_size) {
  const int N = data.size();

  if (batch_size <= 0 || N % batch_size != 0 || N == 0) {
    printf("INVALID SIZE");
    return {{0, 0}};
  }

  const int howmany = N / batch_size;
  const int n[] = {batch_size};

  fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

  if (!in || !out) {
    printf("FFTW malloc failed!\n");
    if (in)
      fftw_free(in);
    if (out)
      fftw_free(out);
    return {};
  }

  for (int i = 0; i < N; ++i) {
    in[i][0] = static_cast<double>(data[i].real());
    in[i][1] = static_cast<double>(data[i].imag());
  }

  fftw_plan plan =
      fftw_plan_many_dft(1, n, howmany, in, nullptr, 1, batch_size, out,
                         nullptr, 1, batch_size, FFTW_FORWARD, FFTW_ESTIMATE);

  if (!plan) {
    printf("FFT dont start!");
    fftw_free(in);
    fftw_free(out);
    return {};
  }

  fftw_execute(plan);

  std::vector<std::complex<double>> result(N);

  for (int i = 0; i < N; ++i) {
    result[i] = {static_cast<double>(out[i][0]),
                 static_cast<double>((out[i][1]))};
  }

  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return result;
}

std::vector<std::complex<double>>
extract_OFDM_symbols(const std::vector<std::complex<double>> &ofdm_samples,
                     const std::vector<int> &peaks, const int CP_size,
                     const int Nc) {
  std::vector<std::complex<double>> result;
  result.reserve(peaks.size() * Nc);

  for (int i = 0; i < peaks.size(); ++i) {
    int peak = peaks[i];
    auto start = ofdm_samples.begin() + peak + CP_size;
    auto end = ofdm_samples.begin() + peak + CP_size + Nc;

    result.insert(result.end(), start, end);
  }

  return result;
}

void CFO_correction(std::vector<std::complex<double>> &samples,
                    const std::vector<int> &peaks,
                    const std::vector<double> &cfo, const int Lcp,
                    const int Nc) {
  int peak_idx;
  for (int i = 0; i < peaks.size(); ++i) {
    int peak_idx = peaks[i];
    double eps = cfo[peak_idx];

    for (int k = 0; k < Nc + Lcp; ++k) {
      samples[peak_idx + k] *=
          std::exp(std::complex<double>(0, -2 * M_PI * eps * k / Lcp));
    }
  }
}