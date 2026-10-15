#ifndef HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_TO_HPP
#define HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_TO_HPP

#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <queue>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/algorithm/color.hpp"
#include "heim/graph/algorithm/predecessor_view.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"
#include "visit.hpp"

namespace heim::graphs
{
namespace detail
{
template<
    typename G,
    typename PMap,
    typename LMap,
    typename TMap>
struct bf_to_visitor
{
private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  vertex_type                m_tgt;
  [[no_unique_address]] PMap m_pm;
  [[no_unique_address]] LMap m_lm;
  [[no_unique_address]] TMap m_tm;
  std::size_t                m_t;

public:
  constexpr
  bf_to_visitor(
      std::in_place_type_t<G>,
      vertex_type tgt,
      PMap        pm,
      LMap        lm,
      TMap        tm,
      std::size_t t  = {})
    : m_tgt{std::move(tgt)}
    , m_pm {std::move(pm )}
    , m_lm {std::move(lm )}
    , m_tm {std::move(tm )}
    , m_t  {std::move(t  )}
  { }


  constexpr
  void
  source_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  {
    using level_type
    = std::remove_reference_t<properties::readable_map_result_t<LMap &, vertex_t<std::remove_cvref_t<G>> &>>;

    properties::set(this->m_pm, v, v);
    properties::set(this->m_lm, v, level_type{});
  }

  constexpr
  visit
  discover_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  {
    properties::set(this->m_tm, v, ++this->m_t);

    if (v == this->m_tgt)
      return visit::stop;

    return visit::proceed;
  }

  constexpr
  void
  tree_edge(G const &g, edge_t<std::remove_cvref_t<G>> e)
  {
    properties::set(this->m_pm, target(g, e), source(g, e));
    properties::set(this->m_lm, target(g, e), properties::get(this->m_lm, source(g, e)) + 1);
  }
};

} // namespace detail


template<
    typename G,
    typename PMap = default_property_map_t,
    typename LMap = default_property_map_t,
    typename TMap = default_property_map_t>
class breadth_first_search_to_result
{
private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  vertex_type                                                                m_tgt;
  detail::result_storage<std::unordered_map<vertex_type, vertex_type>, PMap> m_p_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, LMap> m_l_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, TMap> m_t_rstorage;

public:
  constexpr
  breadth_first_search_to_result(
      std::in_place_type_t<G>,
      vertex_type dst,
      PMap        pm  = {},
      LMap        lm  = {},
      TMap        tm  = {})
    : m_tgt       {std::move(dst)}
    , m_p_rstorage{std::move(pm )}
    , m_l_rstorage{std::move(lm )}
    , m_t_rstorage{std::move(tm )}
  { }


  [[nodiscard]] constexpr
  vertex_type
  destination() const
  { return this->m_tgt; }

  [[nodiscard]] constexpr
  auto
  predecessor_map() const
  { return this->m_p_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  level_map() const
  { return this->m_l_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  discovery_time_map() const
  { return this->m_t_rstorage.property_map(); }


  [[nodiscard]] constexpr
  bool
  reached() const
  { return properties::get(this->discovery_time_map(), this->m_tgt) != std::numeric_limits<std::size_t>::max(); }


  [[nodiscard]] constexpr
  auto
  level() const
  {
    contract_assert(this->reached());
    return properties::get(this->level_map(), this->m_tgt);
  }

  [[nodiscard]] constexpr
  auto
  discovery_time() const
  { return properties::get(this->discovery_time_map(), this->m_tgt); }

  [[nodiscard]] constexpr
  auto
  path() const &
  {
    contract_assert(this->reached());
    return predecessor_view{this->predecessor_map(), this->m_tgt};
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

template<
    typename G,
    typename PMap = default_property_map_t,
    typename LMap = default_property_map_t,
    typename TMap = default_property_map_t>
using bfs_to_result
= breadth_first_search_to_result<G, PMap, LMap, TMap>;


namespace detail
{
struct bfs_to_fn
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
      G                            const &g,
      bfs_to_result<G, PMap, LMap, TMap> &res,
      VRange                            &&rs,
      CMap                                cm  = {},
      Queue                             &&q   = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<G>>;

    auto dst{res.destination       ()};
    auto rpm{res.predecessor_map   ()};
    auto rlm{res.level_map         ()};
    auto rtm{res.discovery_time_map()};


    for (vertex_type v : vertices(g))
      properties::set(rtm, v, std::numeric_limits<std::size_t>::max());

    breadth_first_visit(
        g,
        std::forward<VRange>(rs),
        bf_to_visitor{std::in_place_type<G>, dst, rpm, rlm, rtm},
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
      G                            const &g,
      bfs_to_result<G, PMap, LMap, TMap> &res,
      vertex_t<std::remove_cvref_t<G>>    r,
      CMap                                cm = {},
      Queue                             &&q  = {}) const
  {
    this->operator()(
        g,
        res,
        std::span{std::addressof(r), std::size_t{1}},
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
  bfs_to_result<G, PMap, LMap, TMap>
  operator()(
      G                         const &g,
      VRange                         &&rs,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             pm  = {},
      LMap                             lm  = {},
      TMap                             tm  = {},
      CMap                             cm  = {},
      Queue                          &&q   = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    bfs_to_result res{
        std::in_place_type<G>,
        std::move(tgt),
        std::move(pm ),
        std::move(lm ),
        std::move(tm )};

    this->operator()(
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
  bfs_to_result<G, PMap, LMap, TMap>
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             pm  = {},
      LMap                             lm  = {},
      TMap                             tm  = {},
      CMap                             cm  = {},
      Queue                          &&q   = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::move(tgt),
        std::move(pm ),
        std::move(lm ),
        std::move(tm ),
        std::move(cm ),
        std::forward<Queue>(q));
  }
};

} // namespace detail

inline constexpr detail::bfs_to_fn breadth_first_search_to{};
inline constexpr detail::bfs_to_fn bfs_to                 {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_SEARCH_TO_HPP
