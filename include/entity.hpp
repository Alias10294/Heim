#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <concepts>
#include <type_traits>

namespace heim
{
template<typename T>
struct is_entity
{
  constexpr
  static bool value
  =  !std::is_const_v   <T> &&
     !std::is_volatile_v<T> &&
      std::unsigned_integral<T>;

};

template<typename T>
constexpr
inline bool is_entity_v
= is_entity<T>::value;



namespace defaults
{
template<typename = redefine>
struct entity
{
  using type
  = std::uint32_t;

};

template<typename T = redefine>
using entity_t
= typename entity<T>::type;
}
}

#endif // HEIM_ENTITY_HPP
