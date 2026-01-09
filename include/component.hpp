#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include <cstddef>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief Determines the page size for the default container of a component type.
 */
template<typename = redefine_tag>
struct default_container_page_size
{
  template<typename Component>
  static constexpr
  std::size_t
  value_for
  = 1024;
};

/*!
 * @brief Determines the page size for the given component type.
 *
 * @tparam Component The component type.
 */
template<typename Component>
struct container_page_size_for
  : size_constant<
        default_container_page_size<>::value_for<Component>>
{
  using component_type
  = Component;
};

template<typename Component>
inline constexpr
std::size_t
container_page_size_for_v
= container_page_size_for<Component>::value;



/*!
 * @brief Determines whether a component type is a tag (i.e. is solely used for its association
 *   with an entity).
 */
template<typename = redefine_tag>
struct default_tag_value
{
  template<typename Component>
  static constexpr
  bool value_for
  = std::is_empty_v<Component>;
};

/*!
 * @brief Determines the tag value for the given component, @c true if it is a tag, @c false
 *   otherwise.
 *
 * @tparam Component The component type.
 */
template<typename Component>
struct tag_value_for
  : bool_constant<default_tag_value<>::value_for<Component>>
{
  using component_type
  = Component;
};

template<typename Component>
inline constexpr
bool
tag_value_for_v
= tag_value_for<Component>::value;


} // namespace heim

#endif // HEIM_COMPONENT_HPP
