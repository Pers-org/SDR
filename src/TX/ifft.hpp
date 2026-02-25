#pragma once

#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <vector>

template <typename T>
inline std::pair<std::vector<double>, std::vector<std::complex<double>>>
ifft_batch(const std::vector<std::complex<T>> &samples, const int batch_size,
           const int Fs) {

  const int N = samples.size();

  if (N % batch_size != 0) {
    spdlog::error("[fft.hpp]: Vector size % batch_size != 0!");
    return {};
  }

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

  fftwf_plan plan = fftwf_plan_many_dft(
      rank, N, howmany, in.data(), nullptr, 1, batch_size, out.data(), nullptr,
      1, batch_size, FFTW_BACKWARD, FFTW_MEASURE);

  /*execute FFT*/
  fftw_execute(plan);

  /*copy result*/
  std::pair<std::vector<double>, std::vector<std::complex<double>>> result;
  result.first.resize(N);

  for (int i = 0; i < N; ++i)
    result.first[i] = {out[i][0], out[i][1]};

  /*create timeline*/
  double Ts = 1 / static_cast<double>(Fs);

  for (int i = 0; i < result.first.size(); ++i) {
    result.second[i] = i * 1 / Fs;
  }

  /*free memmory*/
  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return result;
}