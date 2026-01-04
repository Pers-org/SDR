#include <complex>
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
downscaling(const std::vector<std::complex<double>> &samples);

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
 * Pluto Sdr have 12-bits ADC. One bit reserved to sing, so Pluto understand
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
std::vector<int16_t> generate_barker_code(int len);