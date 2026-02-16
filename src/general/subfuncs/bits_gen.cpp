#include <cstdint>
#include <cstdlib>
#include <vector>

std::vector<int16_t> bits_gen(const int N) {
  std::vector<int16_t> bits;
  bits.resize(N);

  for (int i = 0; i < N; ++i) {
    bits[i] = rand() % 2;
  }

  return bits;
}