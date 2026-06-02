#ifndef HEIM_GRAPH_INTERFACE_TRAITS_HPP
#define HEIM_GRAPH_INTERFACE_TRAITS_HPP

#include <concepts>
#include <type_traits>

namespace heim::graph
{
/*!
 * \brief
 *   Determines the vertex descriptor type for the specializing graph type.
 *
 * \note
 *   This trait can be partially specialized for user-defined graph types.
 * \warning
 *   The @code type@endcode attribute must satisfy @code std::regular@endcode.
 */
template<typename G>
struct vertex_descriptor
{ };

template<typename G>
requires requires { typename G::vertex_descriptor; }
struct vertex_descriptor<G>
  : std::type_identity<typename G::vertex_descriptor>
{ };

template<typename G>
using vertex_descriptor_t
= typename vertex_descriptor<G>::type;


/*!
 * \brief
 *   Determines the edge descriptor type for the specializing graph type.
 *
 * \note
 *   This trait can be partially specialized for user-defined graph types.
 * \warning
 *   The @code type@endcode attribute must satisfy @code std::regular@endcode.
 */
template<typename G>
struct edge_descriptor
{ };

template<typename G>
requires requires { typename G::edge_descriptor; }
struct edge_descriptor<G>
  : std::type_identity<typename G::edge_descriptor>
{ };

template<typename G>
using edge_descriptor_t
= typename edge_descriptor<G>::type;


/*!
 * \brief
 *   Determines whether the specializing graph type is directed.
 *
 * \note
 *   This trait can be partially specialized for user-defined graph types.
 */
template<typename G>
struct is_directed
{ };

template<typename G>
requires requires { G::is_directed; }
struct is_directed<G>
  : std::bool_constant<G::is_directed>
{ };

template<typename G>
inline constexpr bool is_directed_v
= is_directed<G>::value;

template<typename G>
concept directed
= is_directed_v<G>;


/*!
 * \brief
 *   Determines whether the specializing graph type is a multigraph.
 *
 * \note
 *   This trait can be partially specialized for user-defined graph types.
 */
template<typename G>
struct is_multigraph
{ };

template<typename G>
requires requires { G::is_multigraph; }
struct is_multigraph<G>
  : std::bool_constant<G::is_multigraph>
{ };

template<typename G>
inline constexpr bool is_multigraph_v
= is_multigraph<G>::value;

template<typename G>
concept multigraph
= is_multigraph_v<G>;


} // namespace heim::graph

#endif // HEIM_GRAPH_INTERFACE_TRAITS_HPP
