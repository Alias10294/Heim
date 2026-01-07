#ifndef HEIM_ALLOCATOR_HPP
#define HEIM_ALLOCATOR_HPP

#include <memory>
#include <type_traits>
#include "utility.hpp"

namespace heim
{
template<typename Allocator>
struct is_allocator
  : bool_constant<
        requires(
            Allocator                                           &t,
            typename std::allocator_traits<Allocator>::pointer   ptr,
            typename std::allocator_traits<Allocator>::size_type n)
        {
          typename std::allocator_traits<Allocator>::value_type;
          { std::allocator_traits<Allocator>::allocate  (t, n)      };
          { std::allocator_traits<Allocator>::deallocate(t, ptr, n) };
        }>
{ };

template<typename T>
inline constexpr
bool
is_allocator_v
= is_allocator<T>::value;



template<
    typename Allocator,
    typename T>
struct is_allocator_for
  : bool_constant<
        is_allocator_v<Allocator>
     && std::is_same_v<typename Allocator::value_type, T>>
{ };

template<
    typename Allocator,
    typename T>
inline constexpr
bool
is_allocator_for_v
= is_allocator_for<Allocator, T>::value;



template<
    typename Allocator>
struct allocator_traits
  : std::allocator_traits<Allocator>
{
  static_assert(is_allocator_v<Allocator>);
};



template<typename T>
using allocator
= std::allocator<T>;


} // namespace heim

#endif // HEIM_ALLOCATOR_HPP
