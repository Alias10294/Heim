#ifndef HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
#define HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include "heim/lib/utility.hpp"

namespace heim::graphs
{
namespace detail
{
template<typename T>
struct adjacency_list_vertex_descriptor
{ T value; };


template<
    typename T,
    bool     IsOutIncident,
    bool     IsInIncident>
struct adjacency_list_edge_descriptor
{ };

template<typename T> struct adjacency_list_edge_descriptor<T, false, false> { T value; };
template<typename T> struct adjacency_list_edge_descriptor<T, true , false> { T value, head; };
template<typename T> struct adjacency_list_edge_descriptor<T, false, true > { T value, tail; };
template<typename T> struct adjacency_list_edge_descriptor<T, true , true > { T value, tail, head; };

} // namespace detail


/*!
 * \brief
 *   ...
 */
template<
    bool     IsDirected,
    bool     IsMultigraph,
    bool     IsOutIncident = true,
    bool     IsInIncident  = true,
    typename T             = std::size_t,
    typename Allocator     = std::allocator<T>>
requires std::unsigned_integral<T>
      && allocator_for         <Allocator, T>
class adjacency_list
{
public:
  static constexpr bool is_directed     = IsDirected;
  static constexpr bool is_multigraph   = IsMultigraph;
  static constexpr bool is_out_incident = IsOutIncident;
  static constexpr bool is_in_incident  = IsInIncident;

  using value_type     = T;
  using allocator_type = Allocator;

  using vertex_descriptor = detail::adjacency_list_vertex_descriptor<value_type>;
  using edge_descriptor   = detail::adjacency_list_edge_descriptor  <value_type, is_out_incident, is_in_incident>;
};

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
