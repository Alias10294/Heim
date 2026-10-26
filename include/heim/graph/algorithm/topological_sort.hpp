#ifndef HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP
#define HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP

#include <algorithm>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "depth_first/depth_first.hpp"

namespace heim::graphs
{
namespace detail::topo
{
template<typename OutputIt>
class visitor
{
private:
  OutputIt m_it;

public:
  constexpr
  visitor(OutputIt it)
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
  { throw std::logic_error{"heim::graphs::reverse_topological_sort: detected cycle"}; }


  constexpr
  auto
  get() &&
  { return std::move(this->m_it); }
};

namespace fn
{
struct reverse_sort
{
  template<
      typename G,
      typename OutIt,
      typename CMap  = default_property_map_t>
  constexpr
  OutIt
  operator()(
      G const &g,
      OutIt    it,
      CMap     cm = {}) const
  requires
      directed<std::remove_cvref_t<G>>
   && std::output_iterator<OutIt, vertex_t<std::remove_cvref_t<G>>>
  {
    visitor vis{std::move(it)};

    depth_first_visit(g, vertices(g), vis, std::move(cm));
    return std::move(vis).get();
  }
};

} // namespace fn
} // namespace detail::topo

inline constexpr
detail::topo::fn::reverse_sort
reverse_topological_sort{};


namespace detail::topo::fn
{
struct sort
{
  template<
      typename G,
      typename Cont = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename CMap = default_property_map_t>
  constexpr
  auto
  operator()(
      G const &g,
      Cont   &&cnt = {},
      CMap     cm  = {}) const
  requires directed<std::remove_cvref_t<G>>
  {
    contract_assert(std::ranges::empty(cnt));

    if constexpr (requires (vertex_t<std::remove_cvref_t<G>> &v) { cnt.push_front(v); })
      reverse_topological_sort(g, std::front_inserter(cnt), std::move(cm));
    else
    {
      reverse_topological_sort(g, std::back_inserter(cnt), std::move(cm));
      std::ranges::reverse(cnt);
    }

    return std::views::all(std::forward<Cont>(cnt));
  }
};

} // namespace detail::topo::fn

inline constexpr
detail::topo::fn::sort
topological_sort{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_TOPOLOGICAL_SORT_HPP
