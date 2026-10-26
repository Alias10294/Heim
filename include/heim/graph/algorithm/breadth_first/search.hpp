#ifndef HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP
#define HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP

#include <concepts>
#include <cstddef>
#include <limits>
#include <queue>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/algorithm/predecessor_view.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"
#include "visit.hpp"

namespace heim::graphs
{
namespace detail::bfs
{
namespace fn
{
struct algo;

} // namespace fn

template<
    typename PMap,
    typename LMap,
    typename TMap>
struct visitor
{
private:
  [[no_unique_address]] PMap m_pm;
  [[no_unique_address]] LMap m_lm;
  [[no_unique_address]] TMap m_tm;
  std::size_t                m_t;

public:
  constexpr
  visitor(
      PMap        pm,
      LMap        lm,
      TMap        tm,
      std::size_t t  = {})
    : m_pm{std::move(pm)}
    , m_lm{std::move(lm)}
    , m_tm{std::move(tm)}
    , m_t {std::move(t )}
  { }


  template<typename G>
  constexpr
  void
  initialize_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_tm, v, std::numeric_limits<std::size_t>::max()); }

  template<typename G>
  constexpr
  void
  source_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  {
    using level_type
    = std::remove_reference_t<properties::readable_map_result_t<LMap &, vertex_t<std::remove_cvref_t<G>> &>>;

    properties::set(this->m_pm, v, v);
    properties::set(this->m_lm, v, level_type{});
  }

  template<typename G>
  constexpr
  void
  discover_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_tm, v, ++this->m_t); }

  template<typename G>
  constexpr
  void
  tree_edge(G const &g, edge_t<std::remove_cvref_t<G>> e)
  {
    properties::set(this->m_pm, target(g, e), source(g, e));
    properties::set(this->m_lm, target(g, e), properties::get(this->m_lm, source(g, e)) + 1);
  }
};

} // namespace detail::bfs


template<
    typename G,
    typename PMap = default_property_map_t,
    typename LMap = default_property_map_t,
    typename TMap = default_property_map_t>
class breadth_first_search_result
{
  friend struct detail::bfs::fn::algo;

private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  detail::result_storage<std::unordered_map<vertex_type, vertex_type>, PMap> m_p_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, LMap> m_l_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, TMap> m_t_rstorage;

private:
  [[nodiscard]] constexpr
  auto
  m_predecessor_map() const
  { return this->m_p_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_level_map() const
  { return this->m_l_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_discovery_time_map() const
  { return this->m_t_rstorage.property_map(); }

public:
  constexpr
  breadth_first_search_result(
      std::in_place_type_t<G>,
      PMap pm = {},
      LMap lm = {},
      TMap tm = {})
    : m_p_rstorage{std::move(pm)}
    , m_l_rstorage{std::move(lm)}
    , m_t_rstorage{std::move(tm)}
  { }


  [[nodiscard]] constexpr
  bool
  reached(vertex_type v) const
  { return properties::get(this->m_discovery_time_map(), std::move(v)) != std::numeric_limits<std::size_t>::max(); }


  [[nodiscard]] constexpr
  vertex_type
  predecessor_of(vertex_type v) const
  {
    contract_assert(this->reached(v));
    return properties::get(this->m_predecessor_map(), std::move(v));
  }

  [[nodiscard]] constexpr
  auto
  level_of(vertex_type v) const
  {
    contract_assert(this->reached(v));
    return properties::get(this->m_level_map(), std::move(v));
  }

  [[nodiscard]] constexpr
  auto
  discovery_time_of(vertex_type v) const
  { return properties::get(this->m_discovery_time_map(), std::move(v)); }

  [[nodiscard]] constexpr
  auto
  path_from(vertex_type v) const &
  {
    contract_assert(this->reached(v));
    return predecessor_view{this->m_predecessor_map(), std::move(v)};
  }

  [[nodiscard]] constexpr
  auto
  path_from(vertex_type) &&
  = delete;

  [[nodiscard]] constexpr
  auto
  path_from(vertex_type) const &&
  = delete;
};

template<
    typename G,
    typename PMap = default_property_map_t,
    typename LMap = default_property_map_t,
    typename TMap = default_property_map_t>
using bfs_result
= breadth_first_search_result<G, PMap, LMap, TMap>;


namespace detail::bfs::fn
{
struct algo
{
  template<
      typename G,
      typename VRange,
      typename PMap,
      typename LMap,
      typename TMap,
      typename CMap   = default_property_map_t,
      typename Queue  = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  void
  operator()(
      G                         const &g,
      bfs_result<G, PMap, LMap, TMap> &res,
      VRange                         &&rs,
      CMap                             cm  = {},
      Queue                          &&q   = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    breadth_first_visit(
        g,
        std::forward<VRange>(rs),
        visitor{
            res.m_predecessor_map   (),
            res.m_level_map         (),
            res.m_discovery_time_map()},
        std::move(cm),
        std::forward<Queue>(q));
  }

  template<
      typename G,
      typename PMap,
      typename LMap,
      typename TMap,
      typename CMap  = default_property_map_t,
      typename Queue = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  void
  operator()(
      G                         const &g,
      bfs_result<G, PMap, LMap, TMap> &res,
      vertex_t<std::remove_cvref_t<G>> r,
      CMap                             cm = {},
      Queue                          &&q  = {}) const
  {
    breadth_first_visit(
        g,
        std::views::single(std::move(r)),
        visitor{
            res.m_predecessor_map   (),
            res.m_level_map         (),
            res.m_discovery_time_map()},
        std::move(cm),
        std::forward<Queue>(q));
  }

  template<
      typename G,
      typename VRange,
      typename PMap   = default_property_map_t,
      typename LMap   = default_property_map_t,
      typename TMap   = default_property_map_t,
      typename CMap   = default_property_map_t,
      typename Queue  = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  bfs_result<G, PMap, LMap, TMap>
  operator()(
      G const &g,
      VRange &&rs,
      PMap     pm = {},
      LMap     lm = {},
      TMap     tm = {},
      CMap     cm = {},
      Queue  &&q  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    bfs_result res{
        std::in_place_type<G>,
        std::move(pm),
        std::move(lm),
        std::move(tm)};

    (*this)(
        g,
        res,
        std::forward<VRange>(rs),
        std::move(cm),
        std::forward<Queue>(q));
    return res;
  }

  template<
      typename G,
      typename PMap  = default_property_map_t,
      typename LMap  = default_property_map_t,
      typename TMap  = default_property_map_t,
      typename CMap  = default_property_map_t,
      typename Queue = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  bfs_result<G, PMap, LMap, TMap>
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      PMap                             pm = {},
      LMap                             lm = {},
      TMap                             tm = {},
      CMap                             cm = {},
      Queue                          &&q  = {}) const
  {
    bfs_result res{
        std::in_place_type<G>,
        std::move(pm),
        std::move(lm),
        std::move(tm)};

    (*this)(
        g,
        res,
        std::views::single(std::move(r)),
        std::move(cm),
        std::forward<Queue>(q));
    return res;
  }
};

} // namespace detail::bfs::fn

inline constexpr detail::bfs::fn::algo breadth_first_search{};
inline constexpr detail::bfs::fn::algo bfs                 {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_HPP
