#ifndef HEIM_COMPONENT_ITERATOR_HPP
#define HEIM_COMPONENT_ITERATOR_HPP

#include <tuple>
#include "utils/specialization_of.hpp"

namespace heim
{
template<typename T>
concept proxy = requires (T obj)
{
  requires specialization_of<T, std::tuple>;
  requires std::tuple_size_v<T> >= 2;
  requires std::unsigned_integral<std::tuple_element_t<0, T>>;

};

template<typename T>
concept iterator = requires (T obj, std::ptrdiff_t dist, T other)
{
  requires proxy<typename T::proxy_type>;

  requires std::is_default_constructible_v<T>;
  requires std::is_copy_constructible_v   <T>;
  requires std::is_move_constructible_v   <T>;
  requires std::is_copy_assignable_v<T>;
  requires std::is_move_assignable_v<T>;
  requires std::is_swappable_v<T>;

  { *obj } -> std::same_as<typename T::proxy_type>;

  { ++obj } -> std::same_as<T&>;
  { --obj } -> std::same_as<T&>;
  { obj + dist } -> std::same_as<T>;

  { obj == other } -> std::same_as<bool>;

};

}

#endif // HEIM_COMPONENT_ITERATOR_HPP
