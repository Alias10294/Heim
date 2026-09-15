#ifndef HEIM_GRAPH_ALGORITHM_DIJKSTRA_HPP
#define HEIM_GRAPH_ALGORITHM_DIJKSTRA_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/traits.hpp"
#include "color.hpp"
#include "common.hpp"
#include "indirect_d_ary_heap.hpp"
#include "predecessor_view.hpp"

namespace heim::graphs
{
template<
    typename G,
    typename WMap,
    typename PMap,
    typename DMap,
    typename Compare,
    typename Infinity,
    typename PStorage,
    typename DStorage>
class dijkstra_result
{
private:
  [[no_unique_address]] PStorage m_p_storage;
  [[no_unique_address]] DStorage m_d_storage;
  [[no_unique_address]] Compare  m_cmp;
  Infinity                       m_inf;
  [[no_unique_address]] WMap     m_w_map;
  [[no_unique_address]] PMap     m_p_map;
  [[no_unique_address]] DMap     m_d_map;

public:
  constexpr
  dijkstra_result(
      std::in_place_type_t<G>,
      WMap       w_map,
      PMap       p_map,
      DMap       d_map,
      Compare    cmp,
      Infinity   inf,
      PStorage &&p_storage,
      DStorage &&d_storage)
    : m_p_storage{std::move(p_storage)}
    , m_d_storage{std::move(d_storage)}
    , m_cmp      {std::move(cmp)}
    , m_inf      {std::move(inf)}
    , m_w_map    {std::move(w_map)}
    , m_p_map    {std::move(p_map)}
    , m_d_map    {std::move(d_map)}
  { }


  [[nodiscard]] constexpr
  WMap
  weight_map() const
  { return this->m_w_map; }

  [[nodiscard]] constexpr
  PMap
  predecessor_map() const
  { return this->m_p_map; }

  [[nodiscard]] constexpr
  DMap
  distance_map() const
  { return this->m_d_map; }

  [[nodiscard]] constexpr
  Compare
  compare() const
  { return this->m_cmp; }


  [[nodiscard]] constexpr
  decltype(auto)
  predecessor_of(vertex_t<std::remove_cvref_t<G>> v) const
  {
    contract_assert(this->reached(v));

    return properties::get(this->m_p_map, std::move(v));
  }

  [[nodiscard]] constexpr
  decltype(auto)
  distance_to(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_d_map, std::move(v)); }

  [[nodiscard]] constexpr
  bool
  reached(vertex_t<std::remove_cvref_t<G>> v) const
  { return std::invoke(this->m_cmp, properties::get(this->m_d_map, std::move(v)), this->m_inf); }

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


template<typename = void>
struct dijkstra_heap_arity
  : std::integral_constant<std::size_t, 4>
{ };

inline constexpr std::size_t dijkstra_heap_arity_v
= dijkstra_heap_arity<>::value;


namespace detail
{
template<
    typename Graph,
    typename VertexRange,
    typename WeightMap,
    typename PredecessorMap,
    typename DistanceMap,
    typename ColorMap,
    typename Container,
    typename Compare,
    typename Zero,
    typename IndexMap>
constexpr
void
dijkstra_impl(
    Graph                                        const &g,
    VertexRange                                       &&rs,
    WeightMap                                           w,
    PredecessorMap                                      p,
    DistanceMap                                         d,
    ColorMap                                            c,
    Container                                         &&cont,
    Compare                                             cmp,
    Zero                                                zero,
    IndexMap                                            i,
    search_target<vertex_t<std::remove_cvref_t<Graph>>> stop = {})
requires
    std::ranges::input_range<VertexRange>
 && std::convertible_to<
        std::ranges::range_reference_t<VertexRange>,
        vertex_t<std::remove_cvref_t<Graph>>>
{
  contract_assert(std::ranges::empty(cont));

  using vertex_type   = vertex_t<std::remove_cvref_t<Graph>>;
  using edge_type     = edge_t  <std::remove_cvref_t<Graph>>;
  using distance_type = properties::readable_map_result_t<DistanceMap &, vertex_type &>;
  using color_type    = std::remove_cvref_t<properties::readable_map_result_t<ColorMap &, vertex_type &>>;

  indirect_d_ary_heap q{
    std::integral_constant<std::size_t, dijkstra_heap_arity_v>{},
    std::forward<Container>(cont),
    d,
    std::move(i),
    cmp};

  for (vertex_type r : std::forward<VertexRange>(rs))
  {
    if (properties::get(c, r) != colors::white_v<color_type>)
      continue;

    properties::set(p, r, r);
    properties::set(d, r, zero);
    properties::set(c, r, colors::gray_v<color_type>);
    q.insert(r);
  }

  while (!q.empty())
  {
    vertex_type   u     {q.extract_min()};
    distance_type u_dist{properties::get(d, u)};

    properties::set(c, u, colors::black_v<color_type>);

    if (std::invoke(stop, u))
      return;

    for (edge_type e : out_edges(g, u))
    {
      vertex_type    v   {target(g, e)};
      decltype(auto) dist{properties::get(w, e) + u_dist};

      if (std::invoke(cmp, dist, properties::get(d, v)))
      {
        properties::set(p, v, u);
        properties::set(d, v, dist);

        color_type v_clr{properties::get(c, v)};

        if (v_clr == colors::white_v<color_type>)
        {
          properties::set(c, v, colors::gray_v<color_type>);
          q.insert(v);
        }
        else if (v_clr == colors::gray_v<color_type>)
          q.decrease_key(v);
      }
    }
  }
}

template<
    typename Graph,
    typename WeightMap,
    typename PredecessorMap,
    typename DistanceMap,
    typename ColorMap,
    typename Container,
    typename Compare,
    typename Zero,
    typename IndexMap>
constexpr
void
dijkstra_impl(
    Graph                                        const &g,
    vertex_t<std::remove_cvref_t<Graph>>                r,
    WeightMap                                           w,
    PredecessorMap                                      p,
    DistanceMap                                         d,
    ColorMap                                            c,
    Container                                         &&cont,
    Compare                                             cmp,
    Zero                                                zero,
    IndexMap                                            i,
    search_target<vertex_t<std::remove_cvref_t<Graph>>> stop = {})
{
  dijkstra_impl(
      g,
      std::span{std::addressof(r), std::size_t{1}},
      std::move(w),
      std::move(p),
      std::move(d),
      std::move(c),
      std::forward<Container>(cont),
      std::move(cmp),
      std::move(zero),
      std::move(i),
      std::move(stop));
}


struct dijkstra_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename WeightMap,
      typename PredecessorMap = default_property_map_t,
      typename DistanceMap    = default_property_map_t,
      typename ColorMap       = default_property_map_t,
      typename Container      = std::vector<vertex_t<std::remove_cvref_t<Graph>>>,
      typename Compare        = std::less  <std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>>,
      typename Zero           = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename Infinity       = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename IndexMap       = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph   const &g,
      VertexRange  &&rs,
      WeightMap      w,
      PredecessorMap p    = {},
      DistanceMap    d    = {},
      ColorMap       c    = {},
      Container    &&cont = {},
      Compare        cmp  = {},
      Zero           zero = {},
      Infinity       inf  = std::numeric_limits<Infinity>::max(),
      IndexMap       i    = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type = vertex_t<std::remove_cvref_t<Graph>>;
    using edge_type   = edge_t  <std::remove_cvref_t<Graph>>;
    using weight_type = std::remove_cvref_t<properties::readable_map_result_t<WeightMap &, edge_type &>>;

    auto p_storage{resolve_storage<PredecessorMap, std::unordered_map<vertex_type, vertex_type>>()};
    auto d_storage{resolve_storage<DistanceMap   , std::unordered_map<vertex_type, weight_type>>()};
    auto c_storage{resolve_storage<ColorMap      , std::unordered_map<vertex_type, color      >>()};
    auto i_storage{resolve_storage<IndexMap      , std::unordered_map<vertex_type, std::size_t>>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto d_map{resolve_property_map(std::move(d), d_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};
    auto i_map{resolve_property_map(std::move(i), i_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map) &, vertex_type &>>;

    for (vertex_type v : vertices(g))
    {
      properties::set(d_map, v, inf);
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    dijkstra_impl(
        g,
        std::forward<VertexRange>(rs),
        w,
        p_map,
        d_map,
        std::move(c_map),
        std::forward<Container>(cont),
        cmp,
        std::move(zero),
        std::move(i_map));

    return dijkstra_result{
        std::in_place_type<Graph>,
        std::move(w),
        std::move(p_map),
        std::move(d_map),
        std::move(cmp),
        std::move(inf),
        std::move(p_storage),
        std::move(d_storage)};
  }

  template<
      typename Graph,
      typename WeightMap,
      typename PredecessorMap = default_property_map_t,
      typename DistanceMap    = default_property_map_t,
      typename ColorMap       = default_property_map_t,
      typename Container      = std::vector<vertex_t<std::remove_cvref_t<Graph>>>,
      typename Compare        = std::less  <std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>>,
      typename Zero           = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename Infinity       = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename IndexMap       = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      WeightMap                            w,
      PredecessorMap                       p    = {},
      DistanceMap                          d    = {},
      ColorMap                             c    = {},
      Container                          &&cont = {},
      Compare                              cmp  = {},
      Zero                                 zero = {},
      Infinity                             inf  = std::numeric_limits<Infinity>::max(),
      IndexMap                             i    = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(w),
        std::move(p),
        std::move(d),
        std::move(c),
        std::forward<Container>(cont),
        std::move(cmp),
        std::move(zero),
        std::move(inf),
        std::move(i));
  }
};

} // namespace detail

inline constexpr detail::dijkstra_fn dijkstra{};


template<
    typename G,
    typename WMap,
    typename PMap,
    typename DMap,
    typename Compare,
    typename Infinity,
    typename PStorage,
    typename DStorage>
class dijkstra_to_result
{
private:
  vertex_t<std::remove_cvref_t<G>> m_tgt;
  [[no_unique_address]] PStorage   m_p_storage;
  [[no_unique_address]] DStorage   m_d_storage;
  [[no_unique_address]] Compare    m_cmp;
  Infinity                         m_inf;
  [[no_unique_address]] WMap       m_w_map;
  [[no_unique_address]] PMap       m_p_map;
  [[no_unique_address]] DMap       m_d_map;

public:
  constexpr
  dijkstra_to_result(
      std::in_place_type_t<G>,
      vertex_t<std::remove_cvref_t<G>> tgt,
      WMap                             w_map,
      PMap                             p_map,
      DMap                             d_map,
      Compare                          cmp,
      Infinity                         inf,
      PStorage                       &&p_storage,
      DStorage                       &&d_storage)
    : m_tgt      {std::move(tgt)}
    , m_p_storage{std::move(p_storage)}
    , m_d_storage{std::move(d_storage)}
    , m_cmp      {std::move(cmp)}
    , m_inf      {std::move(inf)}
    , m_w_map    {std::move(w_map)}
    , m_p_map    {std::move(p_map)}
    , m_d_map    {std::move(d_map)}
  { }


  [[nodiscard]] constexpr
  vertex_t<std::remove_cvref_t<G>>
  target() const
  { return this->m_tgt; }

  [[nodiscard]] constexpr
  WMap
  weight_map() const
  { return this->m_w_map; }

  [[nodiscard]] constexpr
  PMap
  predecessor_map() const
  { return this->m_p_map; }

  [[nodiscard]] constexpr
  DMap
  distance_map() const
  { return this->m_d_map; }

  [[nodiscard]] constexpr
  Compare
  compare() const
  { return this->m_cmp; }


  [[nodiscard]] constexpr
  decltype(auto)
  distance() const
  { return properties::get(this->m_d_map, this->m_tgt); }

  [[nodiscard]] constexpr
  bool
  reached() const
  { return std::invoke(this->m_cmp, properties::get(this->m_d_map, this->m_tgt), this->m_inf); }

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
struct dijkstra_to_fn
{
  template<
      typename Graph,
      typename VertexRange,
      typename WeightMap,
      typename PredecessorMap = default_property_map_t,
      typename DistanceMap    = default_property_map_t,
      typename ColorMap       = default_property_map_t,
      typename Container      = std::vector<vertex_t<std::remove_cvref_t<Graph>>>,
      typename Compare        = std::less  <std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>>,
      typename Zero           = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename Infinity       = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename IndexMap       = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      VertexRange                        &&rs,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      WeightMap                            w,
      PredecessorMap                       p    = {},
      DistanceMap                          d    = {},
      ColorMap                             c    = {},
      Container                          &&cont = {},
      Compare                              cmp  = {},
      Zero                                 zero = {},
      Infinity                             inf  = std::numeric_limits<Infinity>::max(),
      IndexMap                             i    = {}) const
  requires
      std::ranges::input_range<VertexRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VertexRange>,
          vertex_t<std::remove_cvref_t<Graph>>>
  {
    using vertex_type = vertex_t<std::remove_cvref_t<Graph>>;
    using edge_type   = edge_t  <std::remove_cvref_t<Graph>>;
    using weight_type = std::remove_cvref_t<properties::readable_map_result_t<WeightMap &, edge_type &>>;

    auto p_storage{resolve_storage<PredecessorMap, std::unordered_map<vertex_type, vertex_type>>()};
    auto d_storage{resolve_storage<DistanceMap   , std::unordered_map<vertex_type, weight_type>>()};
    auto c_storage{resolve_storage<ColorMap      , std::unordered_map<vertex_type, color      >>()};
    auto i_storage{resolve_storage<IndexMap      , std::unordered_map<vertex_type, std::size_t>>()};

    auto p_map{resolve_property_map(std::move(p), p_storage)};
    auto d_map{resolve_property_map(std::move(d), d_storage)};
    auto c_map{resolve_property_map(std::move(c), c_storage)};
    auto i_map{resolve_property_map(std::move(i), i_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(c_map) &, vertex_type &>>;

    for (vertex_type v : vertices(g))
    {
      properties::set(d_map, v, inf);
      properties::set(c_map, v, colors::white_v<color_type>);
    }

    dijkstra_impl(
        g,
        std::forward<VertexRange>(rs),
        w,
        p_map,
        d_map,
        std::move(c_map),
        std::forward<Container>(cont),
        cmp,
        std::move(zero),
        std::move(i_map),
        search_target{tgt});

    return dijkstra_to_result{
        std::in_place_type<Graph>,
        std::move(tgt),
        std::move(w),
        std::move(p_map),
        std::move(d_map),
        std::move(cmp),
        std::move(inf),
        std::move(p_storage),
        std::move(d_storage)};
  }

  template<
      typename Graph,
      typename WeightMap,
      typename PredecessorMap = default_property_map_t,
      typename DistanceMap    = default_property_map_t,
      typename ColorMap       = default_property_map_t,
      typename Container      = std::vector<vertex_t<std::remove_cvref_t<Graph>>>,
      typename Compare        = std::less  <std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>>,
      typename Zero           = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename Infinity       = std::remove_cvref_t<properties::readable_map_result_t<
          WeightMap &,
          edge_t<std::remove_cvref_t<Graph>>>>,
      typename IndexMap       = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph                         const &g,
      vertex_t<std::remove_cvref_t<Graph>> r,
      vertex_t<std::remove_cvref_t<Graph>> tgt,
      WeightMap                            w,
      PredecessorMap                       p    = {},
      DistanceMap                          d    = {},
      ColorMap                             c    = {},
      Container                          &&cont = {},
      Compare                              cmp  = {},
      Zero                                 zero = {},
      Infinity                             inf  = std::numeric_limits<Infinity>::max(),
      IndexMap                             i    = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(tgt),
        std::move(w),
        std::move(p),
        std::move(d),
        std::move(c),
        std::forward<Container>(cont),
        std::move(cmp),
        std::move(zero),
        std::move(inf),
        std::move(i));
  }
};

} // namespace detail

inline constexpr detail::dijkstra_to_fn dijkstra_to{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DIJKSTRA_HPP