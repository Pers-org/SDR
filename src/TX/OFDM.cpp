#include "../../includes/ImGUI_interface.h"
#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

std::vector<std::vector<std::complex<double>>>
batched(const std::vector<std::complex<double>> &data, const int size) {
  if (data.size() % size != 0) {
    spdlog::error("[OFDM.cpp]: data size % batch size != 0!");
    return {};
  }

  const int batch_count = data.size() / size;
  std::vector<std::vector<std::complex<double>>> batches;
  batches.reserve(batch_count);

  for (int i = 0; i < batch_count; ++i) {
    auto start = data.cbegin() + (i * size);
    auto end = data.begin() + ((i + 1) * size);

    batches.emplace_back(std::vector<std::complex<double>>(start, end));
  }

  return batches;
}

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
