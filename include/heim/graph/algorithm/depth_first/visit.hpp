#ifndef HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_VISIT_HPP
#define HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_VISIT_HPP

#include <concepts>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/algorithm/color.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/algorithm/visit.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"

namespace heim::graphs
{
namespace detail
{
namespace df
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

struct start_vertex
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                             &vis,
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> v) const
  {
    if constexpr (requires { vis.start_vertex(g, v); })
      return invoke_visit([&]{ return vis.start_vertex(g, v); });
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

struct tree_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.tree_edge(g, e); })
      return invoke_visit([&]{ return vis.tree_edge(g, e); });
    else
      return visit::proceed;
  }
};

struct back_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.back_edge(g, e); })
      return invoke_visit([&]{ return vis.back_edge(g, e); });
    else
      return visit::proceed;
  }
};

struct forward_or_cross_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.forward_or_cross_edge(g, e); })
      return invoke_visit([&]{ return vis.forward_or_cross_edge(g, e); });
    else
      return visit::proceed;
  }
};

struct finish_edge
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.finish_edge(g, e); })
      return invoke_visit([&]{ return vis.finish_edge(g, e); });
    else
      return visit::proceed;
  }
};

} // namespace fn


inline constexpr fn::initialize_vertex     initialize_vertex    {};
inline constexpr fn::start_vertex          start_vertex         {};
inline constexpr fn::discover_vertex       discover_vertex      {};
inline constexpr fn::finish_vertex         finish_vertex        {};
inline constexpr fn::examine_edge          examine_edge         {};
inline constexpr fn::tree_edge             tree_edge            {};
inline constexpr fn::back_edge             back_edge            {};
inline constexpr fn::forward_or_cross_edge forward_or_cross_edge{};
inline constexpr fn::finish_edge           finish_edge          {};

} // namespace df

struct df_visit_fn
{
private:
  template<
      typename G,
      typename Vis,
      typename CMap>
  constexpr
  bool
  m_core(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      Vis                            &&vis,
      CMap                             cm) const
  {
    using vertex_type = vertex_t<std::remove_cvref_t<G>>;
    using edge_type   = edge_t  <std::remove_cvref_t<G>>;
    using color_type  = std::remove_cvref_t<properties::readable_map_result_t<CMap &, vertex_type &>>;

    auto ctrl{visit::proceed};

    properties::set(cm, r, colors::gray_v<color_type>);

    ctrl = df::discover_vertex(vis, g, r);
    contract_assert(
        ctrl == visit::proceed
     || ctrl == visit::stop
     || ctrl == visit::prune);

    if      (ctrl == visit::stop)
      return false;
    else if (ctrl != visit::prune)
    {
      for (edge_type e : out_edges(g, r))
      {
        ctrl = df::examine_edge(vis, g, e);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::stop
         || ctrl == visit::skip);

        if      (ctrl == visit::stop)
          return false;
        else if (ctrl == visit::skip)
          continue;

        vertex_type v  {target(g, e)};
        color_type  clr{properties::get(cm, v)};

        if (clr == colors::white_v<color_type>)
        {
          ctrl = df::tree_edge(vis, g, e);
          contract_assert(
              ctrl == visit::proceed
           || ctrl == visit::stop);

          if (ctrl == visit::stop)
            return false;

          if (!this->m_core(g, v, vis, cm))
            return false;
        }
        else
        {
          if (clr == colors::gray_v<color_type>)
          {
            ctrl = df::back_edge(vis, g, e);
            contract_assert(
                ctrl == visit::proceed
             || ctrl == visit::stop);

            if (ctrl == visit::stop)
              return false;
          }
          else
          {
            ctrl = df::forward_or_cross_edge(vis, g, e);
            contract_assert(
                ctrl == visit::proceed
             || ctrl == visit::stop);

            if (ctrl == visit::stop)
              return false;
          }
        }

        ctrl = df::finish_edge(vis, g, e);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::stop);

        if (ctrl == visit::stop)
          return false;
      }
    }

    properties::set(cm, r, colors::black_v<color_type>);

    ctrl = df::finish_vertex(vis, g, r);
    contract_assert(
        ctrl == visit::proceed
     || ctrl == visit::stop);

    if (ctrl == visit::stop)
        return false;

    return true;
  }

public:
  template<
      typename G,
      typename VRange,
      typename Vis,
      typename CMap = default_property_map_t>
  constexpr
  bool
  operator()(
      G const &g,
      VRange &&rs,
      Vis    &&vis,
      CMap     cm  = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<G>>;

    result_storage<std::unordered_map<vertex_type, color>, CMap> c_rstorage{std::move(cm)};
    auto                                                         rcm       {c_rstorage.property_map()};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(rcm) &, vertex_type &>>;

    auto ctrl{visit::proceed};

    for (vertex_type v : vertices(g))
    {
      properties::set(rcm, v, colors::white_v<color_type>);

      ctrl = df::initialize_vertex(vis, g, v);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop);

      if (ctrl == visit::stop)
        return false;
    }

    for (vertex_type r : std::forward<VRange>(rs))
    {
      if (properties::get(rcm, r) != colors::white_v<color_type>)
        continue;

      ctrl = df::start_vertex(vis, g, r);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop
       || ctrl == visit::skip);

      if      (ctrl == visit::stop)
        return false;
      else if (ctrl == visit::skip)
        continue;

      if (!this->m_core(g, std::move(r), vis, rcm))
        return false;
    }

    return true;
  }

  template<
      typename G,
      typename Vis,
      typename CMap = default_property_map_t>
  constexpr
  bool
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> u,
      Vis                            &&vis,
      CMap                             cm  = {}) const
  {
    return (*this)(
        g,
        std::views::single(std::move(u)),
        std::forward<Vis>(vis),
        std::move(cm));
  }
};

} // namespace detail

inline constexpr
detail::df_visit_fn
depth_first_visit{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_DEPTH_FIRST_VISIT_HPP
