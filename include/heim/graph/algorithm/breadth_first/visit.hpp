#ifndef HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_VISIT_HPP
#define HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_VISIT_HPP

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include "heim/graph/algorithm/color.hpp"
#include "heim/graph/algorithm/visit.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"

namespace heim::graphs
{
namespace detail
{
namespace bf
{
struct initialize_vertex_fn
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

struct source_vertex_fn
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


struct discover_vertex_fn
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


struct examine_vertex_fn
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


struct finish_vertex_fn
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


struct examine_edge_fn
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


struct tree_edge_fn
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


struct non_tree_edge_fn
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.non_tree_edge(g, e); })
      return invoke_visit([&]{ return vis.non_tree_edge(g, e); });
    else
      return visit::proceed;
  }
};


struct gray_target_fn
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.gray_target(g, e); })
      return invoke_visit([&]{ return vis.gray_target(g, e); });
    else
      return visit::proceed;
  }
};


struct black_target_fn
{
  template<typename Vis, typename G>
  constexpr
  visit
  operator()(
      Vis                           &vis,
      G                       const &g,
      edge_t<std::remove_cvref_t<G>> e) const
  {
    if constexpr (requires { vis.black_target(g, e); })
      return invoke_visit([&] { return vis.black_target(g, e); });
    else
      return visit::proceed;
  }
};


inline constexpr initialize_vertex_fn initialize_vertex{};
inline constexpr source_vertex_fn     source_vertex    {};
inline constexpr discover_vertex_fn   discover_vertex  {};
inline constexpr examine_vertex_fn    examine_vertex   {};
inline constexpr finish_vertex_fn     finish_vertex    {};
inline constexpr examine_edge_fn      examine_edge     {};
inline constexpr tree_edge_fn         tree_edge        {};
inline constexpr non_tree_edge_fn     non_tree_edge    {};
inline constexpr gray_target_fn       gray_target      {};
inline constexpr black_target_fn      black_target     {};

} // namespace bf


struct bf_visit_fn
{
  template<
      typename G,
      typename VRange,
      typename Vis,
      typename CMap   = default_property_map_t,
      typename Queue  = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  bool
  operator()(
      G const &g,
      VRange &&rs,
      Vis    &&vis,
      CMap     cm  = {},
      Queue  &&q   = {}) const
  requires
      std::ranges::input_range<VRange>
   && std::convertible_to<
          std::ranges::range_reference_t<VRange>,
          vertex_t<std::remove_cvref_t<G>>>
  {
    contract_assert(q.empty());

    using vertex_type = vertex_t<std::remove_cvref_t<G>>;
    using edge_type   = edge_t  <std::remove_cvref_t<G>>;

    result_storage<std::unordered_map<vertex_type, color>, CMap> c_rstorage{std::move(cm)};
    auto                                                         rcm       {c_rstorage.property_map()};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(rcm) &, vertex_type &>>;

    auto ctrl{visit::proceed};

    for (vertex_type v : vertices(g))
    {
      properties::set(rcm, v, colors::white_v<color_type>);

      ctrl = bf::initialize_vertex(vis, g, v);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop);

      if (ctrl == visit::stop)
        return false;
    }

    for (vertex_type r : std::forward<VRange>(rs))
    {
      if (properties::get(rcm, r) == colors::white_v<color_type>)
      {
        ctrl = bf::source_vertex(vis, g, r);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::skip
         || ctrl == visit::stop);

        if      (ctrl == visit::stop)
          return false;
        else if (ctrl == visit::skip)
          continue;

        properties::set(rcm, r, colors::gray_v<color_type>);

        ctrl = bf::discover_vertex(vis, g, r);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::prune
         || ctrl == visit::stop);

        if      (ctrl == visit::stop)
          return false;
        else if (ctrl == visit::prune)
        {
          properties::set(rcm, r, colors::black_v<color_type>);

          ctrl = bf::finish_vertex(vis, g, r);
          contract_assert(
              ctrl == visit::proceed
           || ctrl == visit::stop);

          if (ctrl == visit::stop)
            return false;

          continue;
        }

        q.push(r);
      }
    }

    while (!q.empty())
    {
      vertex_type u{q.front()};
      q.pop();

      ctrl = bf::examine_vertex(vis, g, u);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::prune
       || ctrl == visit::stop);

      if (ctrl == visit::stop)
        return false;

      if (ctrl == visit::prune)
      {
        properties::set(rcm, u, colors::black_v<color_type>);

        ctrl = bf::finish_vertex(vis, g, u);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::stop);

        if (ctrl == visit::stop)
          return false;

        continue;
      }

      for (edge_type e : out_edges(g, u))
      {
        ctrl = bf::examine_edge(vis, g, e);
        contract_assert(
            ctrl == visit::proceed
         || ctrl == visit::skip
         || ctrl == visit::stop);

        if      (ctrl == visit::stop)
          return false;
        else if (ctrl == visit::skip)
          continue;

        vertex_type v  {target(g, e)};
        color_type  clr{properties::get(rcm, v)};

        if (clr == colors::white_v<color_type>)
        {
          ctrl = bf::tree_edge(vis, g, e);
          contract_assert(
              ctrl == visit::proceed
           || ctrl == visit::stop);

          if (ctrl == visit::stop)
            return false;

          properties::set(rcm, v, colors::gray_v<color_type>);

          ctrl = bf::discover_vertex(vis, g, v);
          contract_assert(
              ctrl == visit::proceed
           || ctrl == visit::prune
           || ctrl == visit::stop);

          if      (ctrl == visit::stop)
            return false;
          else if (ctrl == visit::prune)
          {
            properties::set(rcm, v, colors::black_v<color_type>);

            ctrl = bf::finish_vertex(vis, g, v);
            contract_assert(
                ctrl == visit::proceed
             || ctrl == visit::stop);

            if (ctrl == visit::stop)
              return false;

            continue;
          }

          q.push(v);
        }
        else
        {
          ctrl = bf::non_tree_edge(vis, g, e);
          contract_assert(
              ctrl == visit::proceed
           || ctrl == visit::skip
           || ctrl == visit::stop);

          if      (ctrl == visit::stop)
            return false;
          else if (ctrl == visit::skip)
            continue;

          if (clr == colors::gray_v<color_type>)
          {
            ctrl = bf::gray_target(vis, g, e);
            contract_assert(
                ctrl == visit::proceed
             || ctrl == visit::stop);

            if (ctrl == visit::stop)
              return false;
          }
          else
          {
            ctrl = bf::black_target(vis, g, e);
            contract_assert(
                ctrl == visit::proceed
             || ctrl == visit::stop);

            if (ctrl == visit::stop)
              return false;
          }
        }
      }

      properties::set(rcm, u, colors::black_v<color_type>);

      ctrl = bf::finish_vertex(vis, g, u);
      contract_assert(
          ctrl == visit::proceed
       || ctrl == visit::stop);

      if (ctrl == visit::stop)
        return false;
    }

    return true;
  }

  template<
      typename G,
      typename Vis,
      typename CMap  = default_property_map_t,
      typename Queue = std::queue<vertex_t<std::remove_cvref_t<G>>>>
  constexpr
  bool
  operator()(
      G                         const &g,
      vertex_t<std::remove_cvref_t<G>> r,
      Vis                            &&vis,
      CMap                             cm  = {},
      Queue                          &&q   = {}) const
  {
    return this->operator()(
        g,
        std::span{std::addressof(r), std::size_t{1}},
        std::forward<Vis>(vis),
        std::move(cm),
        std::forward<Queue>(q));
  }
};

} // namespace detail

inline constexpr
detail::bf_visit_fn
breadth_first_visit{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_BREADTH_FIRST_VISIT_HPP
