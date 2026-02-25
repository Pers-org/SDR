#pragma once

#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

template <typename T>
inline std::vector<std::complex<double>>
ifft_batch(const std::vector<std::complex<T>> &samples, const int batch_size,
           const int Fs) {

  if (samples.size() % batch_size != 0) {
    spdlog::error("[fft.hpp]: Vector size % batch_size != 0!");
    return {};
  }

  const int n[] = {batch_size};
  const int N = samples.size();

  /*create fftw3 structs*/
  fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

  /*copy*/
  for (int i = 0; i < N; ++i) {
    in[i][0] = samples[i].real();
    in[i][1] = samples[i].imag();
  }

  const int rank = 1;
  const int howmany = N / batch_size;

  fftw_plan plan =
      fftw_plan_many_dft(rank, n, howmany, in, nullptr, 1, batch_size, out,
                         nullptr, 1, batch_size, FFTW_BACKWARD, FFTW_MEASURE);

  /*execute FFT*/
  fftw_execute(plan);

  /*copy result*/
  std::vector<std::complex<double>> result;
  result.resize(N);

  for (int i = 0; i < N; ++i)
    result[i] = {out[i][0] / N, out[i][1] / N};

  /*free memmory*/
  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return result;
}