#ifndef HEIM_UNSAFE_ANY_HPP
#define HEIM_UNSAFE_ANY_HPP

#include <concepts>
#include <utility>

namespace heim
{
class unsafe_any
{
public:
  constexpr
  unsafe_any()
  = default;

  constexpr
  unsafe_any(unsafe_any const &other)
  = default;

  constexpr
  unsafe_any(unsafe_any &&other)
  = default;

  template<typename T>
  constexpr
  unsafe_any(T &&val)
  {
    // TODO: implement
  }

  template<typename T, typename ...Args>
  constexpr
  unsafe_any(std::in_place_type_t<T>, Args &&...args)
  requires (std::constructible_from<T, Args...>)
  {
    // TODO: implement
  }


  constexpr
  ~unsafe_any()
  noexcept
  {
    // TODO: implement
  }


  constexpr unsafe_any &
  operator=(unsafe_any const &other)
  = default;

  constexpr unsafe_any &
  operator=(unsafe_any &&other)
  noexcept
  = default;

};


}


#endif // HEIM_UNSAFE_ANY_HPP
