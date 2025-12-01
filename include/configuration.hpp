#ifndef HEIM_CONFIGURATION_HPP
#define HEIM_CONFIGURATION_HPP

#include <cstddef>
#include <memory>
#include "lib/index_map.hpp"
#include "lib/utility.hpp"

namespace heim
{
template<typename = redefine_tag>
struct default_allocator
{
  template<typename T>
  using type_for = std::allocator<T>;

};


template<typename T>
struct allocator_for
{
  using type
  = default_allocator<>::template type_for<T>;

};

template<typename T>
using allocator_for_t
= allocator_for<T>::type;



template<typename = redefine_tag>
struct default_page_size
{
  template<typename T>
  static constexpr std::size_t
  value_for = 1024;

};


template<typename T>
struct page_size_for
{
  static constexpr std::size_t
  value = default_page_size<>::template value_for<T>;

};

template<typename T>
inline constexpr std::size_t
page_size_for_v = page_size_for<T>::value;


}


#endif // HEIM_CONFIGURATION_HPP
