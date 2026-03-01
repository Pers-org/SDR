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
    // result[i] /= batch_size;
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

std::complex<double>
corr(const std::vector<std::complex<int16_t>> &symbols,
     const std::vector<std::complex<int16_t>> &sync_seq_symb) {
  if (symbols.size() != sync_seq_symb.size()) {
    spdlog::error("Vectors must be the same size");
    return {0.0, 0.0};
  }

  std::complex<double> sum{0.0, 0.0};
  std::complex<double> x;
  std::complex<double> y;

  for (int i = 0; i < symbols.size(); ++i) {
    x = std::complex<double>(std::real(symbols[i]), std::imag(symbols[i]));
    y = std::complex<double>(std::real(sync_seq_symb[i]),
                             std::imag(sync_seq_symb[i]));
    sum += x * std::conj(y);
  }

  return sum;
}

double norm_corr(const std::vector<std::complex<int16_t>> &symbols,
                 const std::vector<std::complex<int16_t>> &sync_seq_symb) {
  std::complex<double> unnormal_cor = corr(symbols, sync_seq_symb);

  double norm_coeff_a = 0.0;
  double norm_coeff_b = 0.0;

  for (int i = 0; i < symbols.size(); ++i) {
    norm_coeff_a += std::norm(symbols[i]);
    norm_coeff_b += std::norm(sync_seq_symb[i]);
  }

  double norm_coeff = std::sqrt(norm_coeff_a) * std::sqrt(norm_coeff_b);

  if (norm_coeff < 1e-12)
    return 0.0;

  return std::abs(unnormal_cor / norm_coeff);
}

std::vector<double>
OFDM_corr_receive(const std::vector<std::complex<int16_t>> &samples,
                  const int symb_size, const int CP_size) {
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

std::vector<std::complex<double>>
batch_fft(const std::vector<std::complex<int16_t>> &data, int batch_size) {
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

std::vector<std::complex<int16_t>>
extract_OFDM_symbols(const std::vector<std::complex<int16_t>> &ofdm_samples,
                     const std::vector<int> &peaks, const int CP_size,
                     const int Nc) {
  std::vector<std::complex<int16_t>> result;
  result.reserve(peaks.size() * Nc);

  for (int i = 0; i < peaks.size(); ++i) {
    int peak = peaks[i];
    auto start = ofdm_samples.begin() + peak + CP_size;
    auto end = ofdm_samples.begin() + peak + CP_size + Nc;

    result.insert(result.end(), start, end);
  }

  return result;
}
