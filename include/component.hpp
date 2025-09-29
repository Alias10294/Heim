#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include "core/utility.hpp"

namespace heim
{
template<typename T>
struct is_component
{
  constexpr
  static bool value
  = !std::is_const_v   <T> &&
    !std::is_volatile_v<T> &&
     std::is_object_v  <T> &&
     std::is_nothrow_constructible_v     <T> &&
     std::is_nothrow_move_constructible_v<T> &&
     std::is_nothrow_move_assignable_v   <T> &&
     std::is_nothrow_destructible_v      <T>;

};

template<typename T>
constexpr
inline bool is_component_v
= is_component<T>::value;



template<typename Component, typename T>
struct is_component_allocator
{
  static_assert(
      is_component_v<Component>,
      "heim::is_component_allocator: Component must satisfy: "
          "is_component_v<Component>.");

  constexpr
  static bool value
  = std::is_same_v<
      typename std::allocator_traits<T>::value_type,
      Component>;

};

template<typename Component, typename T>
constexpr
inline bool is_component_allocator_v
= is_component_allocator<Component, T>::value;



namespace defaults
{
template<typename = redefine>
struct page_size
{
  constexpr
  static std::size_t value
  = 4096;

};

template<typename T = redefine>
constexpr
inline std::size_t page_size_v
= page_size<T>::value;



template<
    typename Component,
    typename T    = redefine>
struct component_allocator
{
  static_assert(
      is_component_v<Component>,
      "heim::component_allocator: Component must satisfy: "
          "is_component_v<Component>.");



  using type
  = std::allocator<Component>;

};

template<
    typename Component,
    typename T    = redefine>
using component_allocator_t
= typename component_allocator<Component, T>::type;

}
}

#endif // HEIM_COMPONENT_HPP