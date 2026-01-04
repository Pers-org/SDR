#pragma once

#include <complex>

/**
 * @brief This struct use for compare complex number. Used in std::map for
 * movement on tree
 */
struct ComplexLess {
  bool operator()(const std::complex<double> &a,
                  const std::complex<double> &b) const {
    if (a.real() < b.real())
      return true;
    if (a.real() > b.real())
      return false;
    return a.imag() < b.imag();
  }
};