#ifndef HEIM_UTILITY_HPP
#define HEIM_UTILITY_HPP

#include <limits>
#include <type_traits>

namespace heim
{
//! @brief An expressive alias allowing for explicit user redefinition of type traits.
using redefine_tag = void;



/*!
 * @brief A type representing a constant value.
 *
 * @tparam T   The value type.
 * @tparam val The value.
 */
template<
    typename T,
    T        val>
using constant
= std::integral_constant<T, val>;

template<bool val>
using bool_constant
= constant<bool, val>;

template<std::size_t val>
using size_constant
= constant<std::size_t, val>;



/*!
 * @brief Determines the unsigned integral type which is composed of at least the given number of
 *   bits.
 *
 * @tparam Digits The number of bits.
 */
template<int Digits>
struct unsigned_integral_for;

template<int Digits>
using unsigned_integral_for_t
= typename unsigned_integral_for<Digits>::type;

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



/*!
 * @brief Determines the type with a conditionally added @c const qualifier.
 *
 * @tparam T       The type to determine for.
 * @tparam IsConst The condition value.
 */
template<
    typename T,
    bool     IsConst>
struct maybe_const;

template<
    typename T,
    bool     IsConst>
using maybe_const_t
= typename maybe_const<T, IsConst>::type;

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



/*!
 * @brief Determines if the given type is unqualified (i.e. neither @c const, @c volatile nor a
 *   reference).
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct is_unqualified;

template<typename T>
inline constexpr
bool
is_unqualified_v
= is_unqualified<T>::value;

template<typename T>
struct is_unqualified
  : bool_constant<std::is_same_v<T, std::remove_cvref_t<T>>>
{ };


}

#endif // HEIM_UTILITY_HPP