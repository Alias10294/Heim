#ifndef HEIM_LIB_UTILITY_HPP
#define HEIM_LIB_UTILITY_HPP

#include <cstddef>
#include <limits>
#include <type_traits>

namespace heim
{
//! @brief An alias useful to express custom redefinition of traits by the user.
using redefine_tag = void;



/*!
 * @brief The smallest unsigned integral type that can hold @code Bits@endcode
 *   bits.
 *
 * @tparam Bits the number of bits to hold for the deduced type.
 */
template<std::size_t Bits>
struct unsigned_integral_for
{
  static_assert(
      0 < Bits && Bits <= std::numeric_limits<unsigned long long>::digits,
      "heim::unsigned_integral_for<Bits>: "
          "0 < Bits && "
          "Bits <= std::numeric_limits<unsigned long long>::digits;");

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


#endif //HEIM_LIB_UTILITY_HPP
