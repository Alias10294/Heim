#ifndef HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP
#define HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP

#include <algorithm>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "color.hpp"
#include "common.hpp"
#include "depth_first_search.hpp"

namespace heim::graphs
{
namespace detail
{
template<typename OutputIt>
class reverse_topological_sort_visitor
  : public basic_depth_first_search_visitor
{
private:
  OutputIt m_it;

public:
  constexpr
  reverse_topological_sort_visitor()
  = default;

  constexpr
  reverse_topological_sort_visitor(OutputIt it)
    : m_it{std::move(it)}
  { }


  template<typename Graph>
  constexpr
  void
  finish_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  { *this->m_it++ = v; }

  template<typename Graph>
  static constexpr
  void
  back_edge(Graph const &, edge_t<std::remove_cvref_t<Graph>>)
  { throw std::logic_error{"reverse_topological_sort_visitor::back_edge"}; }


  constexpr
  auto
  extract() &&
  { return std::move(this->m_it); }
};


struct reverse_topological_sort_fn
{
  template<
      typename Graph,
      typename OutputIt,
      typename ColorMap>
  constexpr
  OutputIt
  operator()(
      Graph const &g,
      OutputIt     it,
      ColorMap     c) const
  requires
      directed<std::remove_cvref_t<Graph>>
   && std::output_iterator<OutputIt, vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto c_storage{resolve_storage<ColorMap, std::unordered_map<vertex_type, color>>()};
    auto c_map    {resolve_property_map(std::move(c), c_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map), vertex_type &>>;

    for (vertex_type v : vertices(g))
      properties::set(c_map, v, colors::white_v<color_type>);

    reverse_topological_sort_visitor vis{std::move(it)};

    for (vertex_type v : vertices(g))
    {
      if (properties::get(c_map, v) != colors::white_v<color_type>)
        continue;

      depth_first_visit(
          g,
          v,
          vis,
          c_map);
    }

    return std::move(vis).extract();
  }
};

} // namespace detail

inline constexpr detail::reverse_topological_sort_fn reverse_topological_sort{};


namespace detail
{
struct topological_sort_fn
{
  template<
      typename Graph,
      typename Container = std::vector<vertex_t<std::remove_cvref_t<Graph>>>,
      typename ColorMap  = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph const &g,
      Container  &&cont = {},
      ColorMap     c    = {}) const
  requires directed<std::remove_cvref_t<Graph>>
  {
    contract_assert(std::ranges::empty(cont));

    if constexpr (requires (vertex_t<std::remove_cvref_t<Graph>> &v) { cont.push_front(v); })
      reverse_topological_sort(g, std::front_inserter(cont), std::move(c));
    else
    {
      reverse_topological_sort(g, std::back_inserter(cont), std::move(c));
      std::ranges::reverse(cont);
    }

    return std::views::all(std::forward<Container>(cont));
  }
};

} // namespace detail

inline constexpr detail::topological_sort_fn topological_sort{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP
