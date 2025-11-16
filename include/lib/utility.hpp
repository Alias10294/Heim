#ifndef HEIM_LIB_UTILITY_HPP
#define HEIM_LIB_UTILITY_HPP

#include <climits>
#include <cstddef>
#include <cstdint>

namespace heim
{
//! @brief An alias useful to express custom redefinition of traits by the user.
using redefine_tag = void;



template<std::size_t Bits>
struct unsigned_integral_for
{
  static_assert(
      0 < Bits && Bits <= 64,
      "heim::unsigned_integral_for<Bits>: "
          "0 < Bits && Bits <= 64;");

public:
  using type
  = std::conditional_t<
      Bits <= CHAR_BIT,
      std::uint8_t,
      std::conditional_t<
          Bits <= CHAR_BIT * 2,
          std::uint16_t,
          std::conditional_t<
              Bits <= CHAR_BIT * 4,
              std::uint32_t,
              std::uint64_t>>>;

};

template<std::size_t Bits>
using unsigned_integral_for_t
= unsigned_integral_for<Bits>::type;


} // namespace heim


#endif //HEIM_LIB_UTILITY_HPP
