#ifndef HEIM_GRAPH_ALGORITHM_DIJKSTRA_SEARCH_HPP
#define HEIM_GRAPH_ALGORITHM_DIJKSTRA_SEARCH_HPP

#include <concepts>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/graph/algorithm/predecessor_view.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "visit.hpp"

namespace heim::graphs
{
namespace detail::dij
{
namespace fn
{
struct algo;

} // namespace fn


template<typename PMap>
struct visitor
{
private:
  [[no_unique_address]] PMap m_pm;

public:
  constexpr
  visitor(PMap pm)
    : m_pm{std::move(pm)}
  { }


  template<typename G>
  constexpr
  void
  source_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_pm, v, v); }

  template<typename G>
  constexpr
  void
  relaxed_edge(G const &g, edge_t<std::remove_cvref_t<G>> e)
  { properties::set(this->m_pm, target(g, e), source(g, e)); }
};

} // namespace detail::dij


template<
    typename G,
    typename WMap,
    typename PMap = default_property_map_t,
    typename DMap = default_property_map_t,
    typename Inf  = detail::dij::dist_t<G, WMap, DMap>,
    typename Cmp  = std::less<>>
class dijkstra_search_result
{
  friend class detail::dij::fn::algo;

private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  detail::deduced_storage_t<vertex_type, vertex_type                   , PMap> m_p_rstorage;
  detail::deduced_storage_t<vertex_type, detail::dij::weight_t<G, WMap>, DMap> m_d_rstorage;
  Inf                                                                          m_inf;
  [[no_unique_address]] Cmp                                                    m_cmp;

private:
  [[nodiscard]] constexpr
  auto
  m_predecessor_map() const
  { return this->m_p_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_distance_map() const
  { return this->m_d_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto &
  m_infinity() const &
  { return this->m_inf; }

  [[nodiscard]] constexpr
  auto &
  m_compare() const &
  { return this->m_cmp; }

public:
  constexpr
  dijkstra_search_result(
      std::in_place_type_t<G>,
      std::in_place_type_t<WMap>,
      PMap  pm  = {},
      DMap  dm  = {},
      Inf &&inf = std::numeric_limits<Inf>::max(),
      Cmp &&cmp = {})
    : m_p_rstorage{std::move(pm)}
    , m_d_rstorage{std::move(dm)}
    , m_inf       {std::forward<Inf>(inf)}
    , m_cmp       {std::forward<Cmp>(cmp)}
  { }


  [[nodiscard]] constexpr
  bool
  reached(vertex_type v) const
  { return std::invoke(this->m_cmp, properties::get(this->m_distance_map(), std::move(v)), this->m_inf); }


  [[nodiscard]] constexpr
  vertex_type
  predecessor_of(vertex_type v) const
  { return properties::get(this->m_predecessor_map(), std::move(v)); }

  [[nodiscard]] constexpr
  auto
  distance_of(vertex_type v) const
  { return properties::get(this->m_distance_map(), std::move(v)); }


  [[nodiscard]] constexpr
  auto
  path_from(vertex_type v) const &
  { return predecessor_view{this->m_predecessor_map(), std::move(v)}; }

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
    typename WMap,
    typename PMap,
    typename DMap,
    typename Inf,
    typename Cmp>
dijkstra_search_result(std::in_place_type_t<G>, std::in_place_type_t<WMap>, PMap, DMap, Inf &&, Cmp &&)
-> dijkstra_search_result<G, WMap, PMap, DMap, Inf, Cmp>;

template<
    typename G,
    typename WMap,
    typename PMap = default_property_map_t,
    typename DMap = default_property_map_t,
    typename Inf  = detail::dij::dist_t<G, WMap, DMap>,
    typename Cmp  = std::less<>>
using dijkstra_result
= dijkstra_search_result<G, WMap, PMap, DMap, Inf, Cmp>;


namespace detail::dij::fn
{
struct algo
{
  template<
      typename G,
      typename VRange,
      typename WMap,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename Inf    = dist_t<G, WMap, DMap>,
      typename Zero   = dist_t<G, WMap, DMap>,
      typename Cmp    = std::less<>,
      typename Cmb    = std::plus<>,
      typename CMap   = default_property_map_t,
      typename Cont   = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap   = default_property_map_t>
  constexpr
  void
  operator()(
      G                                        const &g,
      dijkstra_result<G, WMap, PMap, DMap, Inf, Cmp> &res,
      VRange                                        &&rs,
      WMap                                            wm,
      Zero                                          &&zr  = {},
      Cmb                                           &&cmb = {},
      CMap                                            cm  = {},
      Cont                                          &&cnt = {},
      IMap                                            im  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    dijkstra_visit(
        g,
        std::forward<VRange>(rs),
        std::move(wm),
        visitor{res.m_predecessor_map()},
        res.m_distance_map(),
        res.m_infinity(),
        std::forward<Zero>(zr),
        res.m_compare(),
        std::forward<Cmb>(cmb),
        std::move(cm),
        std::forward<Cont>(cnt),
        std::move(im));
  }

  template<
      typename G,
      typename WMap,
      typename PMap = default_property_map_t,
      typename DMap = default_property_map_t,
      typename Inf  = dist_t<G, WMap, DMap>,
      typename Zero = dist_t<G, WMap, DMap>,
      typename Cmp  = std::less<>,
      typename Cmb  = std::plus<>,
      typename CMap = default_property_map_t,
      typename Cont = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap = default_property_map_t>
  constexpr
  void
  operator()(
      G                                        const &g,
      dijkstra_result<G, WMap, PMap, DMap, Inf, Cmp> &res,
      vertex_t<std::remove_cvref_t<G>>                r,
      WMap                                            wm,
      Zero                                          &&zr  = {},
      Cmb                                           &&cmb = {},
      CMap                                            cm  = {},
      Cont                                          &&cnt = {},
      IMap                                            im  = {}) const
  {
    dijkstra_visit(
        g,
        std::views::single(std::move(r)),
        std::move(wm),
        visitor{res.m_predecessor_map()},
        res.m_distance_map(),
        res.m_infinity(),
        std::forward<Zero>(zr),
        res.m_compare(),
        std::forward<Cmb>(cmb),
        std::move(cm),
        std::forward<Cont>(cnt),
        std::move(im));
  }

  template<
      typename G,
      typename VRange,
      typename WMap,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename Inf    = dist_t<G, WMap, DMap>,
      typename Zero   = dist_t<G, WMap, DMap>,
      typename Cmp    = std::less<>,
      typename Cmb    = std::plus<>,
      typename CMap   = default_property_map_t,
      typename Cont   = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap   = default_property_map_t>
  constexpr
  dijkstra_result<G, WMap, PMap, DMap, Inf, Cmp>
  operator()(
      G const &g,
      VRange &&rs,
      WMap     wm,
      PMap     pm  = {},
      DMap     dm  = {},
      Inf    &&inf = std::numeric_limits<Inf>::max(),
      Zero   &&zr  = {},
      Cmp    &&cmp = {},
      Cmb    &&cmb = {},
      CMap     cm  = {},
      Cont   &&cnt = {},
      IMap     im  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    dijkstra_result res{
        std::in_place_type<G>,
        std::in_place_type<WMap>,
        std::move(pm),
        std::move(dm),
        std::forward<Inf>(inf),
        std::forward<Cmp>(cmp)};

    (*this)(
        g,
        res,
        std::forward<VRange>(rs),
        std::move(wm),
        std::forward<Zero>(zr),
        std::forward<Cmb>(cmb),
        std::move(cm),
        std::forward<Cont>(cnt),
        std::move(im));
    return res;
  }

  template<
      typename G,
      typename WMap,
      typename PMap = default_property_map_t,
      typename DMap = default_property_map_t,
      typename Inf  = dist_t<G, WMap, DMap>,
      typename Zero = dist_t<G, WMap, DMap>,
      typename Cmp  = std::less<>,
      typename Cmb  = std::plus<>,
      typename CMap = default_property_map_t,
      typename Cont = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap = default_property_map_t>
  constexpr
  dijkstra_result<G, WMap, PMap, DMap, Inf, Cmp>
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      WMap                             wm,
      PMap                             pm  = {},
      DMap                             dm  = {},
      Inf                            &&inf = std::numeric_limits<Inf>::max(),
      Zero                           &&zr  = {},
      Cmp                            &&cmp = {},
      Cmb                            &&cmb = {},
      CMap                             cm  = {},
      Cont                           &&cnt = {},
      IMap                             im  = {}) const
  {
    dijkstra_result res{
        std::in_place_type<G>,
        std::in_place_type<WMap>,
        std::move(pm),
        std::move(dm),
        std::forward<Inf>(inf),
        std::forward<Cmp>(cmp)};

    (*this)(
        g,
        res,
        std::views::single(std::move(r)),
        std::move(wm),
        std::forward<Zero>(zr),
        std::forward<Cmb>(cmb),
        std::move(cm),
        std::forward<Cont>(cnt),
        std::move(im));
    return res;
  }
};

} // namespace detail::dij::fn

inline constexpr detail::dij::fn::algo dijkstra_search{};
inline constexpr detail::dij::fn::algo dijkstra       {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DIJKSTRA_SEARCH_HPP
