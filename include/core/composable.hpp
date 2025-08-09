#ifndef HEIM_CORE_COMPOSABLE_HPP
#define HEIM_CORE_COMPOSABLE_HPP

#include <concepts>
#include <type_traits>

namespace heim
{
namespace core
{
template<typename T>
concept composable = requires(T t)
{
  requires std::same_as<std::remove_cvref_t<T>, T>;
  std::is_move_constructible_v<T>;
  std::is_move_assignable_v   <T>;
  std::is_destructible_v      <T>;

};

}
}

#endif // HEIM_CORE_COMPOSABLE_HPP
