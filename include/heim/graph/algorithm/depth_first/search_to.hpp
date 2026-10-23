#ifndef HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_TO_HPP
#define HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_TO_HPP

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
struct dfs_to_fn;


template<
    typename G,
    typename PMap,
    typename DMap>
struct df_to_visitor
{
private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  vertex_type                m_tgt;
  [[no_unique_address]] PMap m_pm;
  [[no_unique_address]] DMap m_dm;
  std::size_t                m_t;

public:
  constexpr
  df_to_visitor(
      std::in_place_type_t<G>,
      vertex_type tgt,
      PMap        pm,
      DMap        dm,
      std::size_t t   = {})
    : m_tgt{std::move(tgt)}
    , m_pm {std::move(pm )}
    , m_dm {std::move(dm )}
    , m_t  {std::move(t  )}
  { }

  constexpr
  void
  initialize_vertex(G const &, vertex_type v)
  { properties::set(this->m_dm, v, std::numeric_limits<std::size_t>::max()); }

  constexpr
  void
  start_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_pm, v, v); }

  constexpr
  visit
  discover_vertex(G const &, vertex_type v)
  {
    properties::set(this->m_dm, v, ++this->m_t);

    return v == this->m_tgt ? visit::stop : visit::proceed;
  }

  constexpr
  void
  tree_edge(G const &g, edge_t<std::remove_cvref_t<G>> e)
  { properties::set(this->m_pm, target(g, e), source(g, e)); }
};

} // namespace detail


template<
    typename G,
    typename PMap = default_property_map_t,
    typename DMap = default_property_map_t>
class depth_first_search_to_result
{
  friend struct detail::dfs_to_fn;

private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

private:
  vertex_type                                                                m_tgt;
  detail::result_storage<std::unordered_map<vertex_type, vertex_type>, PMap> m_p_rstorage;
  detail::result_storage<std::unordered_map<vertex_type, std::size_t>, DMap> m_d_rstorage;

private:
  [[nodiscard]] constexpr
  vertex_type
  m_target() const
  { return this->m_tgt; }

  [[nodiscard]] constexpr
  auto
  m_predecessor_map() const
  { return this->m_p_rstorage.property_map(); }

  [[nodiscard]] constexpr
  auto
  m_discovery_time_map() const
  { return this->m_d_rstorage.property_map(); }

public:
  constexpr
  depth_first_search_to_result(
      std::in_place_type_t<G>,
      vertex_type tgt,
      PMap        pm  = {},
      DMap        dm  = {})
    : m_tgt       {std::move(tgt)}
    , m_p_rstorage{std::move(pm )}
    , m_d_rstorage{std::move(dm )}
  { }


  [[nodiscard]] constexpr
  bool
  reached() const
  { return properties::get(this->m_discovery_time_map(), this->m_tgt) != std::numeric_limits<std::size_t>::max(); }


  [[nodiscard]] constexpr
  auto
  discovery_time() const
  { return properties::get(this->m_discovery_time_map(), this->m_tgt); }

  [[nodiscard]] constexpr
  auto
  path() const &
  {
    contract_assert(this->reached());
    return predecessor_view{this->m_predecessor_map(), this->m_tgt};
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
    typename DMap = default_property_map_t>
using dfs_to_result
= depth_first_search_to_result<G, PMap, DMap>;


namespace detail
{
struct dfs_to_fn
{
  template<
      typename G,
      typename VRange,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename CMap   = default_property_map_t>
  constexpr
  void
  operator()(
      G                      const &g,
      dfs_to_result<G, PMap, DMap> &res,
      VRange                      &&rs,
      CMap                          cm  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    depth_first_visit(
        g,
        std::forward<VRange>(rs),
        df_to_visitor{
            std::in_place_type<G>,
            res.m_target            (),
            res.m_predecessor_map   (),
            res.m_discovery_time_map()},
        std::move(cm));
  }

  template<
      typename G,
      typename VRange,
      typename PMap   = default_property_map_t,
      typename DMap   = default_property_map_t,
      typename CMap   = default_property_map_t>
  constexpr
  dfs_to_result<G, PMap, DMap>
  operator()(
      G                         const &g,
      VRange                         &&rs,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             pm = {},
      DMap                             dm = {},
      CMap                             cm = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    dfs_to_result res{
        std::in_place_type<G>,
        std::move(tgt),
        std::move(pm ),
        std::move(dm )};

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
      dfs_to_result<G, PMap, DMap>    &res,
      vertex_t<std::remove_cvref_t<G>> r,
      CMap                             cm  = {}) const
  {
    depth_first_visit(
        g,
        std::move(r),
        df_to_visitor{
            std::in_place_type<G>,
            res.m_target            (),
            res.m_predecessor_map   (),
            res.m_discovery_time_map()},
        std::move(cm));
  }

  template<
      typename G,
      typename PMap = default_property_map_t,
      typename DMap = default_property_map_t,
      typename CMap = default_property_map_t>
  constexpr
  dfs_to_result<G, PMap, DMap>
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      vertex_t<std::remove_cvref_t<G>> tgt,
      PMap                             pm = {},
      DMap                             dm = {},
      CMap                             cm = {}) const
  {
    dfs_to_result res{
        std::in_place_type<G>,
        std::move(tgt),
        std::move(pm ),
        std::move(dm )};

    (*this)(g, res, std::move(r), std::move(cm));
    return res;
  }
};

} // namespace detail

inline constexpr detail::dfs_to_fn depth_first_search_to{};
inline constexpr detail::dfs_to_fn dfs_to               {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_SEARCH_TO_HPP
