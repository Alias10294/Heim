#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include <type_traits>

namespace heim
{
/*!
 * @brief Determines whether a component type is to be considered as empty or not for its use across the
 *   library.
 *
 * @tparam Component The component type to determine for.
 *
 * @warning This information must be accounted for the implementation of each storage used along the library,
 *   as getters and queries are expected to strictly abide by it.
 * @note This type can be specialized for specific component types in order to circumvent the default
 *   optimization behavior.
 */
template<typename Component>
struct component_tag_value;

template<typename Component>
struct component_tag_value
  : std::is_empty<Component>
{ };

template<typename Component>
static constexpr
bool
component_tag_value_v
= component_tag_value<Component>::value;


}

#endif // HEIM_COMPONENT_HPP