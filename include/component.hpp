#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include <cstddef>
#include "utility.hpp"

namespace heim
{
template<typename = redefine_tag>
struct default_container_page_size
{
  template<typename Component>
  static constexpr std::size_t value_for = 1024;
};

template<typename Component>
struct container_page_size_for
  : size_constant<default_container_page_size<>::value_for<Component>>
{
  using component_type = Component;
};

template<typename Component>
inline constexpr std::size_t container_page_size_for_v = container_page_size_for<Component>::value;



template<typename = redefine_tag>
struct default_is_tag
{
  template<typename Component>
  static constexpr bool value_for = std::is_empty_v<Component>;
};


template<typename Component>
struct is_tag_for
  : bool_constant<default_is_tag<>::value_for<Component>>
{
  using component_type = Component;
};

template<typename Component>
inline constexpr bool is_tag_for_v = is_tag_for<Component>::value;


} // namespace heim

#endif // HEIM_COMPONENT_HPP
