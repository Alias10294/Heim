#ifndef HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP
#define HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP

#include <concepts>
#include <cstddef>
#include <limits>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "visit.hpp"

namespace heim::graphs
{
namespace detail
{
struct dfs_fn;


template<
    typename PMap,
    typename DMap,
    typename FMap>
struct df_visitor
{
  [[no_unique_address]] PMap m_pm;
  [[no_unique_address]] DMap m_dm;
  [[no_unique_address]] FMap m_fm;
  std::size_t                m_t;

public:
  constexpr
  df_visitor(
      PMap         pm,
      DMap         dm,
      FMap         fm,
      std::size_t  t  = {})
    : m_pm{std::move(pm)}
    , m_dm{std::move(dm)}
    , m_fm{std::move(fm)}
    , m_t {std::move(t )}
  { }

  template<typename G>
  constexpr
  void
  initialize_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  {
    static constexpr std::size_t t_max
    = std::numeric_limits<std::size_t>::max();

    properties::set(this->m_dm, v, t_max);
    properties::set(this->m_fm, v, t_max);
  }

  template<typename G>
  constexpr
  void
  start_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_pm, v, v); }

  template<typename G>
  constexpr
  void
  discover_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_dm, v, ++this->m_t); }

  template<typename G>
  constexpr
  void
  finish_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_fm, v, ++this->m_t); }

  template<typename G>
  constexpr
  void
  tree_edge(G const &g, edge_t<std::remove_cvref_t<G>> e)
  { properties::set(this->m_pm, target(g, e), source(g, e)); }
};

} // namespace detail


template<
    typename G,
    typename PMap = default_property_map_t,
    typename DMap = default_property_map_t,
    typename FMap = default_property_map_t>
class depth_first_search_result
{
  friend struct detail::dfs_fn;

private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  detail::result_storage<std::unordered_map<vertex_type, vertex_type>, PMap> m_p_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, DMap> m_d_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, FMap> m_f_rstorage;

private:
  [[nodiscard]] constexpr
  auto
  m_predecessor_map() const
  { return this->m_p_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_discovery_time_map() const
  { return this->m_d_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_finish_time_map() const
  { return this->m_f_rstorage.property_map(); }

public:
  constexpr
  depth_first_search_result(
      std::in_place_type_t<G>,
      PMap pm = {},
      DMap dm = {},
      FMap fm = {})
    : m_p_rstorage{std::move(pm)}
    , m_d_rstorage{std::move(dm)}
    , m_f_rstorage{std::move(fm)}
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
  discovery_time_of(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_discovery_time_map(), std::move(v)); }

  [[nodiscard]] constexpr
  auto
  finish_time_of(vertex_t<std::remove_cvref_t<G>> v) const
  { return properties::get(this->m_finish_time_map(), std::move(v)); }

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
    typename DMap = default_property_map_t,
    typename FMap = default_property_map_t>
using dfs_result
= depth_first_search_result<G, PMap, DMap, FMap>;


namespace detail
{
struct dfs_fn
{
  template<
      typename G,
      typename VRange,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename FMap   = default_property_map_t,
      typename CMap   = default_property_map_t>
  constexpr
  void
  operator()(
      G                         const &g,
      dfs_result<G, PMap, DMap, FMap> &res,
      VRange                         &&rs,
      CMap                             cm  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    depth_first_visit(
        g,
        std::forward<VRange>(rs),
        df_visitor{
            res.m_predecessor_map   (),
            res.m_discovery_time_map(),
            res.m_finish_time_map   ()},
        std::move(cm));
  }

  template<
      typename G,
      typename VRange,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename FMap   = default_property_map_t,
      typename CMap   = default_property_map_t>
  constexpr
  dfs_result<G, PMap, DMap, FMap>
  operator()(
      G                         const &g,
      VRange                         &&rs,
      PMap                             pm = {},
      DMap                             dm = {},
      FMap                             fm = {},
      CMap                             cm = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    dfs_result res{
        std::in_place_type<G>,
        std::move(pm),
        std::move(dm),
        std::move(fm)};

    (*this)(g, res, std::forward<VRange>(rs), std::move(cm));
    return res;
  }

  template<
      typename G,
      typename PMap = default_property_map_t,
      typename DMap = default_property_map_t,
      typename FMap = default_property_map_t,
      typename CMap = default_property_map_t>
  constexpr
  void
  operator()(
      G                         const &g,
      dfs_result<G, PMap, DMap, FMap> &res,
      vertex_t<std::remove_cvref_t<G>> r,
      CMap                             cm  = {}) const
  {
    depth_first_visit(
        g,
        std::move(r),
        df_visitor{
            res.m_predecessor_map   (),
            res.m_discovery_time_map(),
            res.m_finish_time_map   ()},
        std::move(cm));
  }

  template<
      typename G,
      typename PMap = default_property_map_t,
      typename DMap = default_property_map_t,
      typename FMap = default_property_map_t,
      typename CMap = default_property_map_t>
  constexpr
  dfs_result<G, PMap, DMap, FMap>
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      PMap                             pm = {},
      DMap                             dm = {},
      FMap                             fm = {},
      CMap                             cm = {}) const
  {
    dfs_result res{
        std::in_place_type<G>,
        std::move(pm),
        std::move(dm),
        std::move(fm)};

    (*this)(g, res, std::move(r), std::move(cm));
    return res;
  }
};

} // namespace detail

inline constexpr detail::dfs_fn depth_first_search{};
inline constexpr detail::dfs_fn dfs               {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_HPP
