#ifndef HEIM_CORE_ITERATOR_HPP
#define HEIM_CORE_ITERATOR_HPP

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include "entity.hpp"
#include "component.hpp"
#include "specialization_of.hpp"

namespace heim
{
namespace core
{
template<typename T>
concept proxy = requires (T t)
{
  requires specialization_of<T, std::tuple>;
  std::tuple_size_v<T> == 2;

  requires entity<std::remove_cvref_t<std::tuple_element_t<0, T>>>;

  requires []<std::size_t... Is>(std::index_sequence<Is ...>)
  {
    return (component<std::remove_cvref_t<std::tuple_element_t<Is + 1, T>>>
        && ...);
  }
  (std::make_index_sequence<std::tuple_size_v<T> - 1>{ });

};

template<typename T>
concept iterator = requires (T t, std::ptrdiff_t const dist, T u)
{
  requires proxy<typename T::proxy_type>;

  std::is_default_constructible_v<T>;
  std::is_copy_constructible_v   <T>;
  std::is_move_constructible_v   <T>;
  std::is_copy_assignable_v<T>;
  std::is_move_assignable_v<T>;

  { *t } noexcept -> std::same_as<typename T::proxy_type>;

  { ++t } noexcept -> std::same_as<T&>;
  { --t } noexcept -> std::same_as<T&>;
  { t + dist } noexcept -> std::same_as<T>;

  { t == u } noexcept -> std::same_as<bool>;
  { t <= u } noexcept;
};

}
}

#endif // HEIM_CORE_ITERATOR_HPP
