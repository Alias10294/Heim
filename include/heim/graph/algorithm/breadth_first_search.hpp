#ifndef HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP
#define HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"
#include "color.hpp"
#include "common.hpp"
#include "predecessor_view.hpp"

namespace heim::graphs
{
namespace detail
{
struct breadth_first_visit_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename Visitor,
      typename ColorMap,
      typename Queue>
  constexpr
  void
  operator()(
      Graph                                        const &g,
      VertexRange                                       &&rs,
      Visitor                                           &&vis,
      ColorMap                                            c,
      Queue                                             &&q,
      search_target<vertex_t<std::remove_cvref_t<Graph>>> stop = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    contract_assert(q.empty());

    using vertex_type = vertex_t<std::remove_cvref_t<Graph>>;
    using edge_type   = edge_t  <std::remove_cvref_t<Graph>>;
    using color_type  = std::remove_cvref_t<properties::readable_map_result_t<ColorMap &, vertex_type const &>>;

    for (vertex_type r : std::forward<VertexRange>(rs))
    {
      if (properties::get(c, r) == colors::white_v<color_type>)
      {
        vis.source_vertex(g, r);
        properties::set(c, r, colors::gray_v<color_type>);
        vis.discover_vertex(g, r);

        if (std::invoke(stop, r))
          return;

        q.push(r);
      }
    }

    while (!q.empty())
    {
      vertex_type u{q.front()};

      q.pop();
      vis.examine_vertex(g, u);

      for (edge_type e : out_edges(g, u))
      {
        vis.examine_edge(g, e);

        vertex_type    v  {target(g, e)};
        decltype(auto) clr{properties::get(c, v)};

        if (clr == colors::white_v<color_type>)
        {
          vis.tree_edge(g, e);
          properties::set(c, v, colors::gray_v<color_type>);
          vis.discover_vertex(g, v);

          if (std::invoke(stop, v))
            return;

          q.push(v);
        }
        else
        {
          vis.non_tree_edge(g, e);
          if (clr == colors::gray_v<color_type>)
            vis.gray_target (g, e);
          else
            vis.black_target(g, e);
        }
      }

      properties::set(c, u, colors::black_v<color_type>);
      vis.finish_vertex(g, u);
    }
  }

  template<
      typename Graph,
      typename Visitor,
      typename ColorMap,
      typename Queue>
  constexpr
  void
  operator()(
      Graph                                        const &g,
      vertex_t<std::remove_cvref_t<Graph>>                r,
      Visitor                                           &&vis,
      ColorMap                                            c,
      Queue                                             &&q,
      search_target<vertex_t<std::remove_cvref_t<Graph>>> stop = {}) const
    {
      this->operator()(
          g,
          std::span{std::addressof(r), std::size_t{1}},
          std::forward<Visitor>(vis),
          std::move(c),
          std::forward<Queue>(q),
          std::move(stop));
    }
};

} // namespace detail

inline constexpr detail::breadth_first_visit_fn breadth_first_visit{};


struct basic_breadth_first_search_visitor
{
  template<
      typename Graph,
      typename Vertex>
  static constexpr
  void
  source_vertex(Graph const &, Vertex)
  noexcept
  { }

  template<
      typename Graph,
      typename Vertex>
  static constexpr
  void
  discover_vertex(Graph const &, Vertex)
  noexcept
  { }

  template<
      typename Graph,
      typename Vertex>
  static constexpr
  void
  examine_vertex(Graph const &, Vertex)
  noexcept
  { }

  template<
      typename Graph,
      typename Vertex>
  static constexpr
  void
  finish_vertex(Graph const &, Vertex)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  examine_edge(Graph const &, Edge)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  tree_edge(Graph const &, Edge)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  non_tree_edge(Graph const &, Edge)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  gray_target(Graph const &, Edge)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  black_target(Graph const &, Edge)
  noexcept
  { }
};

template<
    typename PredecessorMap,
    typename LevelMap,
    typename DiscoveryTimeMap>
class default_breadth_first_search_visitor
  : public basic_breadth_first_search_visitor
{
private:
  [[no_unique_address]] PredecessorMap   m_p_map;
  [[no_unique_address]] LevelMap         m_l_map;
  [[no_unique_address]] DiscoveryTimeMap m_t_map;
  std::size_t                            m_t;

public:
  constexpr
  default_breadth_first_search_visitor()
  = default;

  constexpr
  default_breadth_first_search_visitor(
      PredecessorMap   p_map,
      LevelMap         l_map,
      DiscoveryTimeMap t_map,
      std::size_t      t     = {})
    : m_p_map{std::move(p_map)}
    , m_l_map{std::move(l_map)}
    , m_t_map{std::move(t_map)}
    , m_t    {std::move(t)}
  { }


  template<typename Graph>
  constexpr
  void
  source_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  {
    using level_type
    = std::remove_reference_t<properties::readable_map_result_t<LevelMap &, vertex_t<std::remove_cvref_t<Graph>> &>>;

    properties::set(this->m_p_map, v, v);
    properties::set(this->m_l_map, v, level_type{});
  }

  template<typename Graph>
  constexpr
  void
  discover_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  { properties::set(this->m_t_map, v, ++this->m_t); }

  template<typename Graph>
  constexpr
  void
  tree_edge(Graph const &g, edge_t<std::remove_cvref_t<Graph>> e)
  {
    properties::set(this->m_p_map, target(g, e), source(g, e));
    properties::set(this->m_l_map, target(g, e), properties::get(this->m_l_map, source(g, e)) + 1);
  }
};


template<
    typename G,
    typename PMap,
    typename LMap,
    typename TMap,
    typename PStorage,
    typename LStorage,
    typename TStorage>
class breadth_first_search_result
{
private:
  [[no_unique_address]] PStorage m_p_storage;
  [[no_unique_address]] LStorage m_l_storage;
  [[no_unique_address]] TStorage m_t_storage;
  [[no_unique_address]] PMap     m_p_map;
  [[no_unique_address]] LMap     m_l_map;
  [[no_unique_address]] TMap     m_t_map;

public:
  constexpr
  breadth_first_search_result(
      std::in_place_type_t<G>,
      PMap       p_map,
      LMap       l_map,
      TMap       t_map,
      PStorage &&p_storage,
      LStorage &&l_storage,
      TStorage &&t_storage)
    : m_p_storage{std::move(p_storage)}
    , m_l_storage{std::move(l_storage)}
    , m_t_storage{std::move(t_storage)}
    , m_p_map    {std::move(p_map)}
    , m_l_map    {std::move(l_map)}
    , m_t_map    {std::move(t_map)}
  { }


  [[nodiscard]] constexpr
  PMap
  predecessor_map() const
  { return this->m_p_map; }

  [[nodiscard]] constexpr
  LMap
  level_map() const
  { return this->m_l_map; }

  [[nodiscard]] constexpr
  TMap
  discovery_time_map() const
  { return this->m_t_map; }


  [[nodiscard]] constexpr
  auto
  predecessor_of(vertex_t<std::remove_cvref_t<G>> v) const
  {
    contract_assert(this->reached(v));

    return properties::get(this->m_p_map, std::move(v));
  }

  [[nodiscard]] constexpr
  auto
  level_of(vertex_t<std::remove_cvref_t<G>> v) const
  {
    contract_assert(this->reached(v));

    return properties::get(this->m_l_map, std::move(v));
  }

  [[nodiscard]] constexpr
  auto
  discovery_time_of(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_t_map, std::move(v)); }


  [[nodiscard]] constexpr
  bool
  reached(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_t_map, std::move(v)) != std::numeric_limits<std::size_t>::max(); }

  [[nodiscard]] constexpr
  auto
  path_from(vertex_t<std::remove_cvref_t<G>> v) const &
  {
    contract_assert(this->reached(v));

    return predecessor_view{this->m_p_map, std::move(v)};
  }

  [[nodiscard]] constexpr
  auto
  path_from(vertex_t<std::remove_cvref_t<G>> v) &&
  = delete;

  [[nodiscard]] constexpr
  auto
  path_from(vertex_t<std::remove_cvref_t<G>> v) const &&
  = delete;
};


namespace detail
{
struct breadth_first_search_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename PredecessorMap   = default_property_map_t,
      typename LevelMap         = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Queue            = std::queue<vertex_t<std::remove_cvref_t<Graph>>>>
  constexpr
  auto
  operator()(
      Graph     const &g,
      VertexRange    &&rs,
      PredecessorMap   p  = {},
      LevelMap         l  = {},
      DiscoveryTimeMap t  = {},
      ColorMap         c  = {},
      Queue          &&q  = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto p_storage{resolve_storage<PredecessorMap  , std::unordered_map<vertex_type, vertex_type>>()};
    auto l_storage{resolve_storage<LevelMap        , std::unordered_map<vertex_type, std::size_t>>()};
    auto t_storage{resolve_storage<DiscoveryTimeMap, std::unordered_map<vertex_type, std::size_t>>()};
    auto c_storage{resolve_storage<ColorMap        , std::unordered_map<vertex_type, color      >>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto l_map{resolve_property_map(std::move(l), l_storage)};
    auto t_map{resolve_property_map(std::move(t), t_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map) &, vertex_type &>>;

    for (vertex_type v : vertices(g))
    {
      properties::set(t_map, v, std::numeric_limits<std::size_t>::max());
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    breadth_first_visit(
        g,
        std::forward<VertexRange>(rs),
        default_breadth_first_search_visitor{p_map, l_map, t_map},
        c_map,
        std::forward<Queue>(q));

    return breadth_first_search_result{
        std::in_place_type<Graph>,
        std::move(p_map),
        std::move(l_map),
        std::move(t_map),
        std::move(p_storage),
        std::move(l_storage),
        std::move(t_storage)};
  }

  template<
      typename Graph,
      typename PredecessorMap   = default_property_map_t,
      typename LevelMap         = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Queue            = std::queue<vertex_t<std::remove_cvref_t<Graph>>>>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      PredecessorMap                       p = {},
      LevelMap                             l = {},
      DiscoveryTimeMap                     t = {},
      ColorMap                             c = {},
      Queue                              &&q = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(p),
        std::move(l),
        std::move(t),
        std::move(c),
        std::forward<Queue>(q));
  }
};

} // namespace detail

inline constexpr detail::breadth_first_search_fn breadth_first_search{};


template<
    typename G,
    typename PMap,
    typename LMap,
    typename TMap,
    typename PStorage,
    typename LStorage,
    typename TStorage>
class breadth_first_search_to_result
{
private:
  vertex_t<std::remove_cvref_t<G>> m_tgt;
  [[no_unique_address]] PStorage   m_p_storage;
  [[no_unique_address]] LStorage   m_l_storage;
  [[no_unique_address]] TStorage   m_t_storage;
  [[no_unique_address]] PMap       m_p_map;
  [[no_unique_address]] LMap       m_l_map;
  [[no_unique_address]] TMap       m_t_map;

public:
  constexpr
  breadth_first_search_to_result(
      std::in_place_type_t<G>,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             p_map,
      LMap                             l_map,
      TMap                             t_map,
      PStorage                       &&p_storage,
      LStorage                       &&l_storage,
      TStorage                       &&t_storage)
    : m_tgt      {std::move(tgt)}
    , m_p_storage{std::move(p_storage)}
    , m_l_storage{std::move(l_storage)}
    , m_t_storage{std::move(t_storage)}
    , m_p_map    {std::move(p_map)}
    , m_l_map    {std::move(l_map)}
    , m_t_map    {std::move(t_map)}
  { }


  [[nodiscard]] constexpr
  vertex_t<std::remove_cvref_t<G>>
  target() const
  { return this->m_tgt; }

  [[nodiscard]] constexpr
  PMap
  predecessor_map() const
  { return this->m_p_map; }

  [[nodiscard]] constexpr
  LMap
  level_map() const
  { return this->m_l_map; }

  [[nodiscard]] constexpr
  TMap
  discovery_time_map() const
  { return this->m_t_map; }


  [[nodiscard]] constexpr
  auto
  level() const
  {
    contract_assert(this->reached());

    return properties::get(this->m_l_map, this->m_tgt);
  }

  [[nodiscard]] constexpr
  auto
  discovery_time() const
  { return properties::get(this->m_t_map, this->m_tgt); }


  [[nodiscard]] constexpr
  bool
  reached() const
  { return properties::get(this->m_t_map, this->m_tgt) != std::numeric_limits<std::size_t>::max(); }

  [[nodiscard]] constexpr
  auto
  path() const &
  {
    contract_assert(this->reached());

    return predecessor_view{this->m_p_map, this->m_tgt};
  }

  [[nodiscard]] constexpr
  auto
  path() &&
  = delete;

  [[nodiscard]] constexpr
  auto
  path() const &&
  = delete;
};


namespace detail
{
struct breadth_first_search_to_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename PredecessorMap   = default_property_map_t,
      typename LevelMap         = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Queue            = std::queue<vertex_t<std::remove_cvref_t<Graph>>>>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      VertexRange                        &&rs,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      PredecessorMap                       p  = {},
      LevelMap                             l  = {},
      DiscoveryTimeMap                     t  = {},
      ColorMap                             c  = {},
      Queue                              &&q  = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto p_storage{resolve_storage<PredecessorMap  , std::unordered_map<vertex_type, vertex_type>>()};
    auto l_storage{resolve_storage<LevelMap        , std::unordered_map<vertex_type, std::size_t>>()};
    auto t_storage{resolve_storage<DiscoveryTimeMap, std::unordered_map<vertex_type, std::size_t>>()};
    auto c_storage{resolve_storage<ColorMap        , std::unordered_map<vertex_type, color      >>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto l_map{resolve_property_map(std::move(l), l_storage)};
    auto t_map{resolve_property_map(std::move(t), t_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map) &, vertex_type &>>;

    for (vertex_type v : vertices(g))
    {
      properties::set(t_map, v, std::numeric_limits<std::size_t>::max());
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    breadth_first_visit(
        g,
        std::forward<VertexRange>(rs),
        default_breadth_first_search_visitor{p_map, l_map, t_map},
        c_map,
        std::forward<Queue>(q),
        search_target{tgt});

    return breadth_first_search_to_result{
        std::in_place_type<Graph>,
        std::move(tgt),
        std::move(p_map),
        std::move(l_map),
        std::move(t_map),
        std::move(p_storage),
        std::move(l_storage),
        std::move(t_storage)};
  }

  template<
      typename Graph,
      typename PredecessorMap   = default_property_map_t,
      typename LevelMap         = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Queue            = std::queue<vertex_t<std::remove_cvref_t<Graph>>>>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      PredecessorMap                       p = {},
      LevelMap                             l = {},
      DiscoveryTimeMap                     t = {},
      ColorMap                             c = {},
      Queue                              &&q = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(tgt),
        std::move(p),
        std::move(l),
        std::move(t),
        std::move(c),
        std::forward<Queue>(q));
  }
};

} // namespace detail

inline constexpr detail::breadth_first_search_to_fn breadth_first_search_to{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP
