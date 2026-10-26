#ifndef HEIM_GRAPH_ALGORITHM_DIJKSTRA_VISIT_HPP
#define HEIM_GRAPH_ALGORITHM_DIJKSTRA_VISIT_HPP

#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/graph/algorithm/indirect_d_ary_heap.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/algorithm/visit.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"

namespace heim::graphs
{
template<typename = void>
struct dijkstra_heap_arity
  : std::integral_constant<std::size_t, 4>
{ };

inline constexpr
std::size_t
dijkstra_heap_arity_v
= dijkstra_heap_arity<>::value;


namespace detail::dij
{
template<
    typename G,
    typename WMap>
using weight_t
= std::remove_cvref_t<properties::readable_map_result_t<WMap &, edge_t<std::remove_cvref_t<G>> &>>;

template<
    typename G,
    typename WMap,
    typename DMap>
using dist_t
= std::remove_cvref_t<properties::readable_map_result_t<
    typename deduced_storage_t<vertex_t<std::remove_cvref_t<G>>, weight_t<G, WMap>, DMap>::property_map_type,
    vertex_t<std::remove_cvref_t<G>>>>;


namespace vis
{
namespace fn
{
struct initialize_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                             &vis,
    G                         const &g,
    vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.initialize_vertex(g, v); })
      return invoke_visit([&]{ return vis.initialize_vertex(g, v); });
    else
      return visit::proceed;
  }
};

struct source_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                             &vis,
    G                         const &g,
    vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.source_vertex(g, v); })
      return invoke_visit([&]{ return vis.source_vertex(g, v); });
    else
      return visit::proceed;
  }
};

struct discover_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                             &vis,
    G                         const &g,
    vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.discover_vertex(g, v); })
      return invoke_visit([&]{ return vis.discover_vertex(g, v); });
    else
      return visit::proceed;
  }
};

struct examine_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                             &vis,
    G                         const &g,
    vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.examine_vertex(g, v); })
      return invoke_visit([&]{ return vis.examine_vertex(g, v); });
    else
      return visit::proceed;
  }
};


struct finish_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                             &vis,
    G                         const &g,
    vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.finish_vertex(g, v); })
      return invoke_visit([&]{ return vis.finish_vertex(g, v); });
    else
      return visit::proceed;
  }
};


struct examine_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                           &vis,
    G                       const &g,
    edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.examine_edge(g, e); })
      return invoke_visit([&]{ return vis.examine_edge(g, e); });
    else
      return visit::proceed;
  }
};

struct relaxed_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                           &vis,
    G                       const &g,
    edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.relaxed_edge(g, e); })
      return invoke_visit([&]{ return vis.relaxed_edge(g, e); });
    else
      return visit::proceed;
  }
};

struct non_relaxed_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
    Vis                           &vis,
    G                       const &g,
    edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.non_relaxed_edge(g, e); })
      return invoke_visit([&]{ return vis.non_relaxed_edge(g, e); });
    else
      return visit::proceed;
  }
};

} // namespace fn


inline constexpr fn::initialize_vertex initialize_vertex{};
inline constexpr fn::source_vertex     source_vertex    {};
inline constexpr fn::discover_vertex   discover_vertex  {};
inline constexpr fn::examine_vertex    examine_vertex   {};
inline constexpr fn::finish_vertex     finish_vertex    {};
inline constexpr fn::examine_edge      examine_edge     {};
inline constexpr fn::relaxed_edge      relaxed_edge     {};
inline constexpr fn::non_relaxed_edge  non_relaxed_edge {};


template<
  typename WMap,
  typename Vis,
  typename DMap,
  typename Inf,
  typename Zero,
  typename Cmp,
  typename Cmb,
  typename Heap>
struct bfs_visitor
{
private:
  template<typename G> using vertex_type = vertex_t<std::remove_cvref_t<G>>;
  template<typename G> using edge_type   = edge_t  <std::remove_cvref_t<G>>;

private:
  [[no_unique_address]] WMap m_wm;
  [[no_unique_address]] Vis  m_vis;
  [[no_unique_address]] DMap m_dm;
  Inf                        m_inf;
  Zero                       m_zr;
  [[no_unique_address]] Cmp &m_cmp;
  [[no_unique_address]] Cmb  m_cmb;
  Heap                      &m_heap;

public:
  constexpr
  bfs_visitor(
      WMap   wm,
      Vis  &&vis,
      DMap   dm,
      Inf  &&inf,
      Zero &&zr,
      Cmp   &cmp,
      Cmb  &&cmb,
      Heap  &heap)
    : m_wm  {std::move(wm)}
  , m_vis {std::forward<Vis>(vis)}
  , m_dm  {std::move(dm)}
  , m_inf {std::forward<Inf>(inf)}
  , m_zr  {std::forward<Zero>(zr)}
  , m_cmp {cmp}
  , m_cmb {std::forward<Cmb>(cmb)}
  , m_heap{heap}
  { }


  template<typename G>
  constexpr
  visit
  initialize_vertex(G const &g, vertex_type<G> v)
  {
    properties::set(this->m_dm, v, this->m_inf);

    return vis::initialize_vertex(this->m_vis, g, v);
  }

  template<typename G>
  constexpr
  visit
  source_vertex(G const &g, vertex_type<G> v)
  {
    auto ctrl{vis::source_vertex(this->m_vis, g, v)};
    contract_assert(
        ctrl == visit::proceed
     || ctrl == visit::stop
     || ctrl == visit::skip);

    if (ctrl == visit::stop)
      return visit::stop;
    if (ctrl == visit::skip)
      return visit::skip;

    properties::set(this->m_dm, v, this->m_zr);

    return visit::proceed;
  }

  template<typename G>
  constexpr
  visit
  discover_vertex(G const &g, vertex_type<G> v)
  {
    auto ctrl{vis::discover_vertex(this->m_vis, g, v)};
    contract_assert(
        ctrl == visit::proceed
     || ctrl == visit::stop);

    return ctrl;
  }

  template<typename G>
  constexpr
  visit
  examine_vertex(G const &g, vertex_type<G> v)
  { return vis::examine_vertex(this->m_vis, g, v); }

  template<typename G>
  constexpr
  visit
  finish_vertex(G const &g, vertex_type<G> v)
  { return vis::finish_vertex(this->m_vis, g, v); }

  template<typename G>
  constexpr
  visit
  examine_edge(G const &g, edge_type<G> e)
  {
    auto ctrl{vis::examine_edge(this->m_vis, g, e)};
    contract_assert(
        ctrl == visit::proceed
     || ctrl == visit::stop
     || ctrl == visit::skip);

    if (ctrl == visit::stop)
      return visit::stop;
    if (ctrl == visit::skip)
      return visit::skip;

    vertex_type<G> v   {target(g, e)};
    auto           dist{std::invoke(
        this->m_cmb,
        properties::get(this->m_dm, source(g, e)),
        properties::get(this->m_wm, e))};

    if (std::invoke(this->m_cmp, dist, properties::get(this->m_dm, v)))
    {
      properties::set(this->m_dm, v, dist);

      ctrl = vis::relaxed_edge(this->m_vis, g, e);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop);

      return ctrl;
    }
    else
    {
      ctrl = vis::non_relaxed_edge(this->m_vis, g, e);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop);

      if (ctrl == visit::stop)
        return visit::stop;

      return visit::skip;
    }
  }

  template<typename G>
  constexpr
  void
  gray_target(G const &g, edge_t<std::remove_cvref_t<G>> e)
  { this->m_heap.decrease_key(target(g, e)); }
};

template<
    typename WMap,
    typename Vis,
    typename DMap,
    typename Inf,
    typename Zero,
    typename Cmp,
    typename Cmb,
    typename Heap>
bfs_visitor(WMap, Vis &&, DMap, Inf &&, Zero &&, Cmp &, Cmb &&, Heap &)
-> bfs_visitor<WMap, Vis, DMap, Inf, Zero, Cmp, Cmb, Heap>;


namespace fn
{
struct algo
{
  template<
      typename G,
      typename VRange,
      typename WMap,
      typename Vis,
      typename DMap   = default_property_map_t,
      typename Inf    = dist_t<G, WMap, DMap>,
      typename Zero   = dist_t<G, WMap, DMap>,
      typename Cmp    = std::less<>,
      typename Cmb    = std::plus<>,
      typename CMap   = default_property_map_t,
      typename Cont   = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap   = default_property_map_t>
  constexpr
  bool
  operator()(
      G const &g,
      VRange &&rs,
      WMap     wm,
      Vis    &&vis,
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
    using vertex_type
    = vertex_t<std::remove_cvref_t<G>>;

    deduced_storage_t<vertex_type, weight_t<G, WMap>, DMap> d_rstorage{std::move(dm)};
    auto                                                    rdm       {d_rstorage.property_map()};

    indirect_d_ary_heap heap{
      dijkstra_heap_arity{},
      std::forward<Cont>(cnt),
      rdm,
      std::move(im),
      cmp};

    return breadth_first_visit(
        g,
        std::forward<VRange>(rs),
        bfs_visitor{
            std::move(wm),
            std::forward<Vis>(vis),
            rdm,
            std::forward<Inf >(inf),
            std::forward<Zero>(zr),
            cmp,
            std::forward<Cmb>(cmb),
            heap},
        std::move(cm),
        heap);
  }

  template<
      typename G,
      typename WMap,
      typename Vis,
      typename DMap   = default_property_map_t,
      typename Inf    = dist_t<G, WMap, DMap>,
      typename Zero   = dist_t<G, WMap, DMap>,
      typename Cmp    = std::less<>,
      typename Cmb    = std::plus<>,
      typename CMap   = default_property_map_t,
      typename Cont   = std::vector<vertex_t<std::remove_cvref_t<G>>>,
      typename IMap   = default_property_map_t>
  constexpr
  bool
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      WMap                             wm,
      Vis                            &&vis,
      DMap                             dm  = {},
      Inf                            &&inf = std::numeric_limits<Inf>::max(),
      Zero                           &&zr  = {},
      Cmp                            &&cmp = {},
      Cmb                            &&cmb = {},
      CMap                             cm  = {},
      Cont                           &&cnt = {},
      IMap                             im  = {}) const
  {
    return (*this)(
        g,
        std::views::single(r),
        std::move(wm),
        std::forward<Vis>(vis),
        std::move(dm),
        std::forward<Inf >(inf),
        std::forward<Zero>(zr ),
        std::forward<Cmp >(cmp),
        std::forward<Cmb >(cmb),
        std::move(cm),
        std::forward<Cont>(cnt),
        std::move(im));
  }
};

} // namespace fn
} // namespace vis
} // namespace detail::dij


inline constexpr
detail::dij::vis::fn::algo
dijkstra_visit{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DIJKSTRA_VISIT_HPP
