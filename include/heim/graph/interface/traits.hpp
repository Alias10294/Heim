#ifndef HEIM_GRAPH_STRUCTURE_TRAITS_HPP
#define HEIM_GRAPH_STRUCTURE_TRAITS_HPP

#include <type_traits>

namespace heim::graphs
{
template<typename G>
struct vertex
  : std::type_identity<typename G::vertex_type>
{ };

template<typename G>
using vertex_t
= typename vertex<G>::type;


template<typename G>
struct edge
  : std::type_identity<typename G::edge_type>
{ };

template<typename G>
using edge_t
= typename edge<G>::type;


template<typename G>
struct is_directed
  : std::bool_constant<G::is_directed>
{ };

template<typename G>
inline constexpr bool is_directed_v
= is_directed<G>::value;

template<typename G>
concept directed
= is_directed_v<G>;


template<typename G>
struct is_multigraph
  : std::bool_constant<G::is_multigraph>
{ };

template<typename G>
inline constexpr bool is_multigraph_v
= is_multigraph<G>::value;

template<typename G>
concept multigraph
= is_multigraph_v<G>;

} // namespace heim::graphs

#endif // HEIM_GRAPH_STRUCTURE_TRAITS_HPP
