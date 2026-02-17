#pragma once

#include <algorithm>
#include <complex>
#include <fftw3.h>
#include <vector>

template <typename T> inline void fft_shift(std::vector<T> &fft_samples) {
  int N = fft_samples.size();
  int shift = N / 2;

  std::rotate(fft_samples.begin(), fft_samples.begin() + shift,
              fft_samples.end());
}

inline std::vector<double> fft_freq(const int N, const int Fs) {
  double step = static_cast<double>(Fs) / N;
  std::vector<double> freqs(N);

  for (int i = 0; i < N; ++i)
    freqs[i] = (i - N / 2) * step;

  return freqs;
}

template <typename T>
inline std::pair<std::vector<std::complex<double>>, std::vector<double>>
fft(const std::vector<std::complex<T>> &samples, const int Fs) {

  const int N = samples.size();

  /*create fftw3 structs*/
  fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

  /*copy*/
  for (int i = 0; i < N; ++i) {
    in[i][0] = samples[i].real();
    in[i][1] = samples[i].imag();
  }

  /*define fft parameters*/
  fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  /*execute FFT*/
  fftw_execute(plan);

  /*copy result*/
  std::pair<std::vector<std::complex<double>>, std::vector<double>> result;
  result.first.resize(N);

  for (int i = 0; i < N; ++i)
    result.first[i] = {out[i][0], out[i][1]};

  fft_shift<std::complex<double>>(result.first);

  /*compute freq*/
  result.second = fft_freq(N, Fs);

  /*free memmory*/
  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return result;
}