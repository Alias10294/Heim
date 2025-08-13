#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include <concepts>
#include <type_traits>

namespace heim
{
template<typename T>
concept component = requires
{
  requires std::same_as<std::remove_cvref_t<T>, T>;
  std::is_move_constructible_v<T>;
  std::is_move_assignable_v   <T>;
  std::is_destructible_v      <T>;

};

}

#endif // HEIM_COMPONENT_HPP
