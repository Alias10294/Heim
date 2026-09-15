#ifndef HEIM_PROPERTY_TRAITS_HPP
#define HEIM_PROPERTY_TRAITS_HPP

#include <type_traits>
#include <utility>
#include "primitives.hpp"

namespace heim::properties
{
template<typename M, typename K>
struct is_readable_map_for
  : std::bool_constant<requires { get(std::declval<M>(), std::declval<K>()); }>
{ };

template<typename M, typename K>
inline constexpr bool is_readable_map_for_v
= is_readable_map_for<M, K>::value;

template<typename M, typename K>
concept readable_map_for
= is_readable_map_for_v<M, K>;


template<typename M, typename K, typename ...Args>
struct is_writable_map_for
  : std::bool_constant<requires { set(std::declval<M>(), std::declval<K>(), std::declval<Args>()...); }>
{ };

template<typename M, typename K, typename ...Args>
inline constexpr bool is_writable_map_for_v
= is_writable_map_for<M, K, Args ...>::value;

template<typename M, typename K, typename ...Args>
concept writable_map_for
= is_writable_map_for_v<M, K, Args ...>;


template<typename M, typename K, typename ...Args>
struct is_map_for
  : std::bool_constant<
        is_readable_map_for_v<M, K>
     && is_writable_map_for_v<M, K, Args ...>>
{ };

template<typename M, typename K, typename ...Args>
inline constexpr bool is_map_for_v
= is_map_for<M, K, Args ...>::value;

template<typename M, typename K, typename ...Args>
concept map_for
= is_map_for_v<M, K, Args ...>;


template<typename M, typename K>
requires readable_map_for<M, K>
struct readable_map_result
  : std::invoke_result<decltype(get), M, K>
{ };

template<typename M, typename K>
using readable_map_result_t
= typename readable_map_result<M, K>::type;


template<typename M, typename K>
struct is_reflective_map_for
  : std::bool_constant<
        is_readable_map_for_v<M, K>
     && is_writable_map_for_v<M, K, readable_map_result_t<M, K>>>
{ };

template<typename M, typename K>
inline constexpr bool is_reflective_map_for_v
= is_reflective_map_for<M, K>::value;

template<typename M, typename K>
concept reflective_map_for
= is_reflective_map_for_v<M, K>;

} // namespace heim::properties

#endif // HEIM_PROPERTY_TRAITS_HPP
