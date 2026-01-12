#include "../RX/complex_less.hpp"
#include <complex>
#include <map>
#include <sstream>
#include <vector>

/**
 * @brief convert dec number to bits
 *
 * @param number
 * @param bit_format len of bits seq (example: dec2bin(3, 4) = 0011)
 * @return vector of MBS bits
 */
std::vector<int16_t> dec2bin(const int number, const int bit_format);

/**
 * @brief normalized samples to [-1, 1]. Normalization by max sample value
 *
 * @param samples receiving samples
 * @return vector of samples
 */
std::vector<std::complex<double>>
downscaler(const std::vector<std::complex<double>> &samples);

/**
 * @brief read samples from binary file
 *
 * @param filename reading file
 * @return vector of complex samples
 */
std::vector<std::complex<int16_t>> read_pcm(const std::string &filename);

/**
 * @brief upscale samples to Pluto SDR
 *
 * Pluto Sdr have 12-bits ADC. One bit reserved to sign, so Pluto understand
 * samples in range [-2048, 2047], but Pluto understand samples very strangely
 * and understand only first 12 left bits, so Pluto understand samples in range
 * [-32768, 32767]
 *
 * @param psf_symbols symbols in output of pulse shaping filter
 * @return vector of complex samples
 */
std::vector<std::complex<int16_t>>
upscaling(const std::vector<std::complex<double>> &psf_symbols);

/**
 * @brief write samples to binary file
 *
 * @param filename writing file
 */
void write_pcm(const std::string &filename,
               std::vector<std::complex<int16_t>> samples);
double bits_to_pam_lvl(int a, int b);
// std::vector<int16_t> generate_barker_code(int len);

/**
 * @brief convert vector to string. I use spdlog lib for logging, but lib cant
 * work with std::vector
 *
 * @param vec vector, which we want convert to string
 */
template <typename T> std::string vector2str(std::vector<T> vec) {
  std::ostringstream oss;

  for (const T &el : vec) {
    oss << el << " ";
  }

  return oss.str();
}

/**
 * @brief convert map to string. I use spdlog lib for logging, but lib cant work
 * with std::map
 *
 * @param map map, which we want convert to string
 */
template <typename T, typename K>
std::string map2str(const std::map<T, K> &map) {
  std::ostringstream oss;

  for (const auto &[key, value] : map) {
    oss << key << " -> " << value << '\n';
  }

  return oss.str();
}

template <typename T, typename K>
std::string map2str(const std::map<T, K, ComplexLess> map) {
  std::ostringstream oss;

  for (const auto &[key, value] : map) {
    oss << key << " -> " << value << '\n';
  }

  return oss.str();
}