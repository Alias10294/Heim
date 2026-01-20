#ifndef HEIM_UTILITY_HPP
#define HEIM_UTILITY_HPP

#include <limits>
#include <type_traits>
#include "fwd.hpp"

namespace heim
{
template<int Digits>
struct unsigned_integral_for
{
  static constexpr int digits = Digits;

  static_assert(
      0 < digits && digits <= std::numeric_limits<unsigned long long>::digits,
      "digits must be strictly positive and within the limits of the existing unsigned integral "
      "types.");

  using type
  = std::conditional_t<
      digits <= std::numeric_limits<unsigned char>::digits,
      unsigned char,
      std::conditional_t<
          digits <= std::numeric_limits<unsigned short>::digits,
          unsigned short,
          std::conditional_t<
              digits <= std::numeric_limits<unsigned int>::digits,
              unsigned int,
              std::conditional_t<
                  digits <= std::numeric_limits<unsigned long>::digits,
                  unsigned long,
                  unsigned long long>>>>;
};



template<
    typename T,
    bool     IsConst>
struct maybe_const
{
  static_assert(
      std::is_same_v<T, std::remove_const_t<T>>,
      "T must not already be const-qualified.");

  using type = std::conditional_t<IsConst, T const, T>;
};



template<typename T>
struct is_unqualified
  : bool_constant<std::is_same_v<T, std::remove_cvref_t<T>>>
{ };


}

#endif // HEIM_UTILITY_HPP