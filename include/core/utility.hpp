#ifndef HEIM_UTILITY_HPP
#define HEIM_UTILITY_HPP

#include <type_traits>

namespace heim
{
struct redefine
{ };



template<typename T>
struct is_arithmetic
{
  constexpr
  static bool value
  = !std::is_const_v     <T> &&
    !std::is_volatile_v  <T> &&
     std::is_arithmetic_v<T>;

};

template<typename T>
constexpr
inline bool is_arithmetic_v
= is_arithmetic<T>::value;



template<
    typename                        T,
    template<typename ...> typename Primary>
struct is_specialization_of
  : std::false_type
{ };

template<
    template<typename ...> typename Primary,
    typename                     ...Args>
struct is_specialization_of<Primary<Args...>, Primary>
  : std::true_type
{ };

template<
    typename                        T,
    template<typename ...> typename Primary>
constexpr
inline bool is_specialization_of_v
= is_specialization_of<T, Primary>::value;

}

#endif // HEIM_UTILITY_HPP
