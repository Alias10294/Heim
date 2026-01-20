#ifndef HEIM_ALLOCATOR_HPP
#define HEIM_ALLOCATOR_HPP

#include <memory>
#include <type_traits>
#include "fwd.hpp"

namespace heim
{
template<typename T>
struct is_an_allocator
  : bool_constant<
        requires(
            T                                           &t,
            typename std::allocator_traits<T>::pointer   ptr,
            typename std::allocator_traits<T>::size_type n)
        {
          typename std::allocator_traits<T>::value_type;
          { std::allocator_traits<T>::allocate  (t, n)      };
          { std::allocator_traits<T>::deallocate(t, ptr, n) };
        }>
{ };



template<
    typename A,
    typename T>
struct is_an_allocator_for
  : bool_constant<
        is_an_allocator_v<A>
     && std::is_same_v<typename A::value_type, T>>
{ };


}

#endif // HEIM_ALLOCATOR_HPP