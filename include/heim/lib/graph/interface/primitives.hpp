#ifndef HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP
#define HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP

#include <type_traits>

namespace heim::graphs
{
template<typename G>
struct vertex_descriptor
  : std::type_identity<typename std::remove_cvref_t<G>::vertex_descriptor>
{ };

template<typename G>
using vertex_descriptor_t
= typename vertex_descriptor<G>::type;


template<typename G>
struct edge_descriptor
  : std::type_identity<typename std::remove_cvref_t<G>::edge_descriptor>
{ };

template<typename G>
using edge_descriptor_t
= typename edge_descriptor<G>::type;


template<typename G>
struct is_directed
  : std::bool_constant<std::remove_cvref_t<G>::is_directed>
{ };

template<typename G>
inline constexpr
bool
is_directed_v
= is_directed<G>::value;


template<typename G>
struct is_multigraph
  : std::bool_constant<std::remove_cvref_t<G>::is_multigraph>
{ };

template<typename G>
inline constexpr
bool
is_multigraph_v
= is_multigraph<G>::value;

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP
