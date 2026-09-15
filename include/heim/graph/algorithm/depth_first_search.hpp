#ifndef HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP
#define HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP

#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
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
struct depth_first_visit_fn
{
  template<
      typename Graph,
      typename Visitor,
      typename ColorMap>
  constexpr
  bool
  operator()(
      Graph                                        const &g,
      vertex_t<std::remove_cvref_t<Graph>>                u,
      Visitor                                           &&vis,
      ColorMap                                            c,
      search_target<vertex_t<std::remove_cvref_t<Graph>>> stop = {}) const
  {
    using vertex_type = vertex_t<std::remove_cvref_t<Graph>>;
    using edge_type   = edge_t  <std::remove_cvref_t<Graph>>;
    using color_type  = std::remove_cvref_t<properties::readable_map_result_t<ColorMap &, vertex_type const &>>;

    contract_assert(properties::get(c, u) == colors::white_v<color_type>);

    properties::set(c, u, colors::gray_v<color_type>);
    vis.discover_vertex(g, u);

    if (std::invoke(stop, u))
      return true;

    for (edge_type const e : out_edges(g, u))
    {
      vis.examine_edge(g, e);

      vertex_type    v  {target(g, e)};
      decltype(auto) clr{properties::get(c, v)};

      if (clr == colors::white_v<color_type>)
      {
        vis.tree_edge(g, e);

        if (this->operator()(g, v, vis, c, stop))
          return true;
      }
      else
      {
        if (clr == colors::gray_v<color_type>)
          vis.back_edge(g, e);
        else
          vis.forward_or_cross_edge(g, e);
      }
    }

    properties::set(c, u, colors::black_v<color_type>);
    vis.finish_vertex(g, u);

    return false;
  }
};

} // namespace detail

inline constexpr detail::depth_first_visit_fn depth_first_visit{};


struct basic_depth_first_search_visitor
{
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
  back_edge(Graph const &, Edge)
  noexcept
  { }

  template<
      typename Graph,
      typename Edge>
  static constexpr
  void
  forward_or_cross_edge(Graph const &, Edge)
  noexcept
  { }
};


template<
    typename PredecessorMap,
    typename DiscoveryTimeMap,
    typename FinishTimeMap,
    typename Time>
class default_depth_first_search_visitor
  : public basic_depth_first_search_visitor
{
private:
  [[no_unique_address]] PredecessorMap   m_p_map;
  [[no_unique_address]] DiscoveryTimeMap m_d_map;
  [[no_unique_address]] FinishTimeMap    m_f_map;
  Time                                  &m_t;

public:
  constexpr
  default_depth_first_search_visitor()
  = default;

  constexpr
  default_depth_first_search_visitor(
      PredecessorMap   p_map,
      DiscoveryTimeMap d_map,
      FinishTimeMap    f_map,
      Time            &t)
    : m_p_map{std::move(p_map)}
    , m_d_map{std::move(d_map)}
    , m_f_map{std::move(f_map)}
    , m_t    {t}
  { }


  template<typename Graph>
  constexpr
  void
  discover_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  { properties::set(this->m_d_map, v, ++this->m_t); }

  template<typename Graph>
  constexpr
  void
  finish_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  { properties::set(this->m_f_map, v, ++this->m_t); }

  template<typename Graph>
  constexpr
  void
  tree_edge(Graph const &g, edge_t<std::remove_cvref_t<Graph>> e)
  { properties::set(this->m_p_map, target(g, e), source(g, e)); }
};


template<
    typename G,
    typename PMap,
    typename DMap,
    typename FMap,
    typename PStorage,
    typename DStorage,
    typename FStorage,
    typename Time>
class depth_first_search_result
{
private:
  [[no_unique_address]] PStorage m_p_storage;
  [[no_unique_address]] DStorage m_d_storage;
  [[no_unique_address]] FStorage m_f_storage;
  [[no_unique_address]] PMap     m_p_map;
  [[no_unique_address]] DMap     m_d_map;
  [[no_unique_address]] FMap     m_f_map;

public:
  constexpr
  depth_first_search_result(
      std::in_place_type_t<G>,
      PMap       p_map,
      DMap       d_map,
      FMap       f_map,
      PStorage &&p_storage,
      DStorage &&d_storage,
      FStorage &&f_storage,
      Time     &&)
    : m_p_storage{std::move(p_storage)}
    , m_d_storage{std::move(d_storage)}
    , m_f_storage{std::move(f_storage)}
    , m_p_map    {std::move(p_map)}
    , m_d_map    {std::move(d_map)}
    , m_f_map    {std::move(f_map)}
  { }

public:
  [[nodiscard]] constexpr
  PMap
  predecessor_map() const
  { return this->m_p_map; }

  [[nodiscard]] constexpr
  DMap
  discovery_time_map() const
  { return this->m_d_map; }

  [[nodiscard]] constexpr
  FMap
  finish_time_map() const
  { return this->m_f_map; }


  [[nodiscard]] constexpr
  auto
  predecessor_of(vertex_t<std::remove_cvref_t<G>> v) const
  {
    contract_assert(this->reached(v));

    return properties::get(this->m_p_map, std::move(v));
  }

  [[nodiscard]] constexpr
  auto
  discovery_time_of(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_d_map, std::move(v)); }

  [[nodiscard]] constexpr
  auto
  finish_time_of(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_f_map, std::move(v)); }


  [[nodiscard]] constexpr
  bool
  reached(vertex_t<std::remove_cvref_t<G>> v) const
  {
    return properties::get(this->m_d_map, std::move(v))
        != std::numeric_limits<std::remove_cvref_t<Time>>::max();
  }

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
struct depth_first_search_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename PredecessorMap   = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename FinishTimeMap    = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Time             = std::size_t>
  constexpr
  auto
  operator()(
      Graph       const &g,
      VertexRange      &&rs,
      PredecessorMap     p  = {},
      DiscoveryTimeMap   d  = {},
      FinishTimeMap      f  = {},
      ColorMap           c  = {},
      Time             &&t  = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto p_storage{resolve_storage<PredecessorMap  , std::unordered_map<vertex_type, vertex_type>>()};
    auto d_storage{resolve_storage<DiscoveryTimeMap, std::unordered_map<vertex_type, std::size_t>>()};
    auto f_storage{resolve_storage<FinishTimeMap   , std::unordered_map<vertex_type, std::size_t>>()};
    auto c_storage{resolve_storage<ColorMap        , std::unordered_map<vertex_type, color      >>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto d_map{resolve_property_map(std::move(d), d_storage)};
    auto f_map{resolve_property_map(std::move(f), f_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map), vertex_type &>>;

    static constexpr std::size_t t_max
    = std::numeric_limits<std::size_t>::max();

    for (vertex_type v : vertices(g))
    {
      properties::set(d_map, v, t_max);
      properties::set(f_map, v, t_max);
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    for (vertex_type r : std::forward<VertexRange>(rs))
    {
      if (properties::get(c_map, r) == colors::white_v<color_type>)
      {
        properties::set(p_map, r, r);

        depth_first_visit(
            g,
            std::move(r),
            default_depth_first_search_visitor{p_map, d_map, f_map, t},
            c_map);
      }
    }

    return depth_first_search_result{
        std::in_place_type<Graph>,
        std::move(p_map),
        std::move(d_map),
        std::move(f_map),
        std::move(p_storage),
        std::move(d_storage),
        std::move(f_storage),
        std::forward<Time>(t)};
  }

  template<
      typename Graph,
      typename PredecessorMap   = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename FinishTimeMap    = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Time             = std::size_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      PredecessorMap                       p  = {},
      DiscoveryTimeMap                     d  = {},
      FinishTimeMap                        f  = {},
      ColorMap                             c  = {},
      Time                               &&t  = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(p),
        std::move(d),
        std::move(f),
        std::move(c),
        std::forward<Time>(t));
  }
};

} // namespace detail

inline constexpr detail::depth_first_search_fn depth_first_search{};


template<
    typename G,
    typename PMap,
    typename DMap,
    typename FMap,
    typename PStorage,
    typename DStorage,
    typename FStorage,
    typename Time>
class depth_first_search_to_result
{
private:
  vertex_t<std::remove_cvref_t<G>> m_tgt;
  [[no_unique_address]] PStorage   m_p_storage;
  [[no_unique_address]] DStorage   m_d_storage;
  [[no_unique_address]] FStorage   m_f_storage;
  [[no_unique_address]] PMap       m_p_map;
  [[no_unique_address]] DMap       m_d_map;
  [[no_unique_address]] FMap       m_f_map;

public:
  constexpr
  depth_first_search_to_result(
      std::in_place_type_t<G>,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             p_map,
      DMap                             d_map,
      FMap                             f_map,
      PStorage                       &&p_storage,
      DStorage                       &&d_storage,
      FStorage                       &&f_storage,
      Time                           &&)
    : m_tgt      {std::move(tgt)}
    , m_p_storage{std::move(p_storage)}
    , m_d_storage{std::move(d_storage)}
    , m_f_storage{std::move(f_storage)}
    , m_p_map    {std::move(p_map)}
    , m_d_map    {std::move(d_map)}
    , m_f_map    {std::move(f_map)}
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
  DMap
  discovery_time_map() const
  { return this->m_d_map; }

  [[nodiscard]] constexpr
  FMap
  finish_time_map() const
  { return this->m_f_map; }


  [[nodiscard]] constexpr
  auto
  discovery_time() const
  { return properties::get(this->m_d_map, this->m_tgt); }


  [[nodiscard]] constexpr
  bool
  reached() const
  { return properties::get(this->m_d_map, this->m_tgt) != std::numeric_limits<std::remove_cvref_t<Time>>::max(); }

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
struct depth_first_search_to_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename PredecessorMap   = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename FinishTimeMap    = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Time             = std::size_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      VertexRange                        &&rs,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      PredecessorMap                       p  = {},
      DiscoveryTimeMap                     d  = {},
      FinishTimeMap                        f  = {},
      ColorMap                             c  = {},
      Time                               &&t  = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto p_storage{resolve_storage<PredecessorMap  , std::unordered_map<vertex_type, vertex_type>>()};
    auto d_storage{resolve_storage<DiscoveryTimeMap, std::unordered_map<vertex_type, std::size_t>>()};
    auto f_storage{resolve_storage<FinishTimeMap   , std::unordered_map<vertex_type, std::size_t>>()};
    auto c_storage{resolve_storage<ColorMap        , std::unordered_map<vertex_type, color      >>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto d_map{resolve_property_map(std::move(d), d_storage)};
    auto f_map{resolve_property_map(std::move(f), f_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map), vertex_type &>>;

    static constexpr std::size_t t_max
    = std::numeric_limits<std::size_t>::max();

    for (vertex_type v : vertices(g))
    {
      properties::set(d_map, v, t_max);
      properties::set(f_map, v, t_max);
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    for (vertex_type r : std::forward<VertexRange>(rs))
    {
      if (properties::get(c_map, r) == colors::white_v<color_type>)
      {
        properties::set(p_map, r, r);

        if (depth_first_visit(
            g,
            std::move(r),
            default_depth_first_search_visitor{p_map, d_map, f_map, t},
            c_map,
            search_target{tgt}))
          break;
      }
    }

    return depth_first_search_to_result{
        std::in_place_type<Graph>,
        std::move(tgt),
        std::move(p_map),
        std::move(d_map),
        std::move(f_map),
        std::move(p_storage),
        std::move(d_storage),
        std::move(f_storage),
        std::forward<Time>(t)};
  }

  template<
      typename Graph,
      typename PredecessorMap   = default_property_map_t,
      typename DiscoveryTimeMap = default_property_map_t,
      typename FinishTimeMap    = default_property_map_t,
      typename ColorMap         = default_property_map_t,
      typename Time             = std::size_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      PredecessorMap                       p  = {},
      DiscoveryTimeMap                     d  = {},
      FinishTimeMap                        f  = {},
      ColorMap                             c  = {},
      Time                               &&t  = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(tgt),
        std::move(p),
        std::move(d),
        std::move(f),
        std::move(c),
        std::forward<Time>(t));
  }
};

} // namespace detail

inline constexpr detail::depth_first_search_to_fn depth_first_search_to{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP
