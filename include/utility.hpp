#ifndef HEIM_UTILITY_HPP
#define HEIM_UTILITY_HPP

#include <cstddef>
#include <limits>
#include <type_traits>

namespace heim
{
//! @brief An alias to explicit redefinition of types and traits.
using redefine_tag = void;



/*!
 * @brief A remodel of the STL's integral_constant, made to bring consistency
 *   to code notation.
 *
 * @tparam T   The value type.
 * @tparam val The value.
 */
template<
    typename T,
    T        val>
struct constant
{
  //! @brief The value type.
  using value_type = T;

  //! @returns The value.
  static consteval
  value_type
  value()
  noexcept
  {
    return val;
  }
};

template<
    bool val>
using bool_constant
= constant<bool, val>;


template<
    std::size_t val>
using size_constant
= constant<std::size_t, val>;



template<
    typename T>
struct is_unqualified
  : bool_constant<std::is_same_v<T, std::remove_cvref_t<T>>>
{ };

template<
    typename T>
inline constexpr
bool
is_unqualified_v
= is_unqualified<T>::value();



template<
    typename T,
    bool     IsConst>
struct maybe_const
{
  static_assert(std::is_same_v<T, std::remove_const_t<T>>);

public:
  using type = std::conditional_t<IsConst, T const, T>;
};

template<
    typename T,
    bool     IsConst>
using maybe_const_t = typename maybe_const<T, IsConst>::type;




/*!
 * @brief The smallest unsigned integral type of at least Bits bits.
 *
 * @tparam Bits the minimum number of bits of the determined type.
 */
template<std::size_t Bits>
struct unsigned_integral_for
{
  static_assert(0 < Bits && Bits <= std::numeric_limits<unsigned long long>::digits);

public:
  using type
  = std::conditional_t<
      Bits <= std::numeric_limits<unsigned char>::digits,
      unsigned char,
      std::conditional_t<
          Bits <= std::numeric_limits<unsigned short>::digits,
          unsigned short,
          std::conditional_t<
              Bits <= std::numeric_limits<unsigned int>::digits,
              unsigned int,
              std::conditional_t<
                  Bits <= std::numeric_limits<unsigned long>::digits,
                  unsigned long,
                  unsigned long long>>>>;

};

template<std::size_t Bits>
using unsigned_integral_for_t
= typename unsigned_integral_for<Bits>::type;


} // namespace heim

#endif // HEIM_UTILITY_HPP
