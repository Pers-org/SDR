#pragma once

#include "../RX/complex_less.hpp"
#include <bitset>
#include <complex>
#include <fstream>
#include <map>
#include <string>
#include <vector>

/**
 * @brief This class allow to do nice output log info
 */
class logger {
private:
  std::ofstream log_file;

public:
  logger(const std::string &filename) : log_file(filename) {}
  ~logger() { log_file.close(); }

  /**
   * @brief Output label
   */
  void label(const char *description);

  /**
   * @brief Output value with label
   */
  template <typename T> void value(const char *description, const T value);

  /**
   * @brief Output vector with label
   */
  template <typename T>
  void vector(const char *description, const std::vector<T> vector);

  /**
   * @brief Output map with label
   */
  template <typename T, typename K>
  void map(const char *description, const std::map<T, K> &map);

  template <typename T, typename K>
  void map(const char *description, const std::map<T, K, ComplexLess> &map);
};

template <typename T>
void logger::value(const char *description, const T value) {
  log_file << std::string(description) << ": " << value << "\n\n";
}

template <typename T>
void logger::vector(const char *description, const std::vector<T> vector) {
  log_file << std::string(description) << ": ";

  for (const T &el : vector)
    log_file << el << " ";

  log_file << "\n\n";
}

template <typename T, typename K>
void logger::map(const char *description, const std::map<T, K> &map) {

  log_file << std::string(description) << ": \n";

  for (const std::pair<T, K> &el : map) {
    log_file << std::bitset<2>(el.first);
    log_file << " -> " << el.second << '\n';
  }

  log_file << "\n";
}

template <typename T, typename K>
void logger::map(const char *description,
                 const std::map<T, K, ComplexLess> &map) {

  log_file << std::string(description) << ": \n";

  for (const auto &[key, value] : map) {
    log_file << std::bitset<2>(value);
    log_file << " -> " << key << '\n';
  }

  log_file << "\n";
}

void logger::label(const char *description) {
  log_file << std::string(description) << "\n";
}
