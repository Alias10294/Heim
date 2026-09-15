#ifndef HEIM_GRAPH_STRUCTURE_PRIMITIVES_HPP
#define HEIM_GRAPH_STRUCTURE_PRIMITIVES_HPP

#include <concepts>
#include <ranges>
#include <utility>
#include "traits.hpp"

namespace heim::graphs
{
namespace detail
{
template<typename R, typename G>
concept vertex_range
=   std::ranges::range<R>
 && std::convertible_to<std::ranges::range_reference_t<R>, vertex_t<std::remove_cvref_t<G>>>;


struct vertices_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().vertices() } -> vertex_range<G>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { vertices(std::declval<G &&>()) } -> vertex_range<G>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().vertices());
    else
      return noexcept(vertices(std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).vertices();
    else
      return vertices(std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::vertices_fn
vertices;


namespace detail
{
template<typename R, typename G>
concept edge_range
=   std::ranges::range<R>
 && std::convertible_to<std::ranges::range_reference_t<R>, edge_t<std::remove_cvref_t<G>>>;


struct edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().edges() } -> edge_range<G>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { edges(std::declval<G &&>()) } -> edge_range<G>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().edges());
    else
      return noexcept(edges(std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).edges();
    else
      return edges(std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::edges_fn
edges;


namespace detail
{
struct vertex_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().vertex_count(); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { vertex_count(std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_vertices_fn
  = requires { std::ranges::size(vertices(std::declval<G &&>())); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_vertices_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().vertex_count());
    else if constexpr (s_adl_fn<G>)
      return noexcept(vertex_count(std::declval<G &&>()));
    else
      return noexcept(std::ranges::size(vertices(std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_vertices_fn<G>
  [[nodiscard]] constexpr
  auto
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).vertex_count();
    else if constexpr (s_adl_fn<G>)
      return vertex_count(std::forward<G>(g));
    else
      return std::ranges::size(vertices(std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::vertex_count_fn vertex_count;
inline constexpr detail::vertex_count_fn order;


namespace detail
{
struct edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().edge_count(); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { edge_count(std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_edges_fn
  = requires { std::ranges::size(edges(std::declval<G &&>())); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_edges_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().edge_count());
    else if constexpr (s_adl_fn<G>)
      return noexcept(edge_count(std::declval<G &&>()));
    else
      return noexcept(std::ranges::size(edges(std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_edges_fn<G>
  [[nodiscard]] constexpr
  auto
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).edge_count();
    else if constexpr (s_adl_fn<G>)
      return edge_count(std::forward<G>(g));
    else
      return std::ranges::size(edges(std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::edge_count_fn edge_count;
inline constexpr detail::edge_count_fn size;


namespace detail
{
struct out_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires
      { { std::declval<G &&>().out_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>()) } -> edge_range<G>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires
      { { out_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()) } -> edge_range<G>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().out_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(out_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).out_edges( v);
    else
      return out_edges(std::forward<G>(g), v);
  }
};

} // namespace detail

inline constexpr
detail::out_edges_fn
out_edges;


namespace detail
{
struct in_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().in_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>()) } -> edge_range<G>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { in_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()) } -> edge_range<G>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().in_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(in_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).in_edges( v);
    else
      return in_edges(std::forward<G>(g), v);
  }
};

} // namespace detail

inline constexpr
detail::in_edges_fn
in_edges;


namespace detail
{
struct out_edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().out_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { out_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_out_edges_fn
  = requires { std::ranges::size(out_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>())); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_out_edges_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().out_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_adl_fn<G>)
      return noexcept(out_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
    {
      return noexcept(std::ranges::size(out_edges(
          std::declval<G &&>(),
          std::declval<vertex_t<std::remove_cvref_t<G>> &>())));
    }

  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_out_edges_fn<G>
  [[nodiscard]] constexpr
  auto
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).out_edge_count( v);
    else if constexpr (s_adl_fn<G>)
      return out_edge_count(std::forward<G>(g), v);
    else
      return std::ranges::size(out_edges(std::forward<G>(g), v));
  }
};

} // namespace detail

inline constexpr detail::out_edge_count_fn out_edge_count;
inline constexpr detail::out_edge_count_fn out_degree;


namespace detail
{
struct in_edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().in_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { in_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_in_edges_fn
  = requires { std::ranges::size(in_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>())); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_in_edges_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().in_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_adl_fn<G>)
      return noexcept(in_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(std::ranges::size(in_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>())));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_in_edges_fn<G>
  [[nodiscard]] constexpr
  auto
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).in_edge_count( v);
    else if constexpr (s_adl_fn<G>)
      return in_edge_count(std::forward<G>(g), v);
    else
      return std::ranges::size(in_edges(std::forward<G>(g), v));
  }
};

} // namespace detail

inline constexpr detail::in_edge_count_fn in_edge_count;
inline constexpr detail::in_edge_count_fn in_degree;


namespace detail
{
struct total_edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().total_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { total_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_undirected_out
  =  !directed<G>
   && requires { out_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_undirected_in
  =  !directed<G>
   && requires { in_edge_count (std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_directed_in_out
  =   directed<G>
   && requires { out_edge_count(std::declval<G &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); }
   && requires { in_edge_count (std::declval<G &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
        || s_undirected_out<G> || s_undirected_in<G> || s_directed_in_out<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().total_edge_count( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_adl_fn<G>)
      return noexcept(total_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_undirected_out<G>)
      return noexcept(out_edge_count(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_undirected_in<G>)
      return noexcept(in_edge_count (std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
    {
      return noexcept(
          out_edge_count(std::declval<G &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>())
        + in_edge_count (std::declval<G &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    }
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
        || s_undirected_out<G> || s_undirected_in<G> || s_directed_in_out<G>
  [[nodiscard]] constexpr
  auto
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).total_edge_count( v);
    else if constexpr (s_adl_fn<G>)
      return total_edge_count(std::forward<G>(g), v);
    else if constexpr (s_undirected_out<G>)
      return out_edge_count(std::forward<G>(g), v);
    else if constexpr (s_undirected_in<G>)
      return in_edge_count (std::forward<G>(g), v);
    else
      return in_edge_count(g, v) + out_edge_count(g, v);
  }
};

} // namespace detail

inline constexpr detail::total_edge_count_fn total_edge_count;
inline constexpr detail::total_edge_count_fn degree;


namespace detail
{
struct source_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().source( std::declval<edge_t<std::remove_cvref_t<G>> &>()) } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { source(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()) } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().source( std::declval<edge_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(source(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  vertex_t<std::remove_cvref_t<G>>
  operator()(G &&g, edge_t<std::remove_cvref_t<G>> e) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).source( e);
    else
      return source(std::forward<G>(g), e);
  }
};

} // namespace detail

inline constexpr
detail::source_fn
source;


namespace detail
{
struct target_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().target( std::declval<edge_t<std::remove_cvref_t<G>> &>()) } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { target(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()) } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().target( std::declval<edge_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(target(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  [[nodiscard]] constexpr
  vertex_t<std::remove_cvref_t<G>>
  operator()(G &&g, edge_t<std::remove_cvref_t<G>> e) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).target( e);
    else
      return target(std::forward<G>(g), e);
  }
};

} // namespace detail

inline constexpr
detail::target_fn
target;


namespace detail
{
struct endpoints_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires
  {
    { std::declval<G &&>().endpoints( std::declval<edge_t<std::remove_cvref_t<G>> &>()) }
    -> std::convertible_to<std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>>;
  };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires
  {
    { endpoints(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()) }
    -> std::convertible_to<std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>>;
  };

  template<typename G>
  static constexpr bool s_source_target
  = requires
  {
    std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>{
        source(std::declval<G &>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()),
        target(std::declval<G &>(), std::declval<edge_t<std::remove_cvref_t<G>> &>())};
  };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_source_target<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().endpoints( std::declval<edge_t<std::remove_cvref_t<G>> &>()));
    else if constexpr (s_adl_fn<G>)
      return noexcept(endpoints(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(
          std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>{
              source(std::declval<G &>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()),
              target(std::declval<G &>(), std::declval<edge_t<std::remove_cvref_t<G>> &>())});
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G> || s_source_target<G>
  [[nodiscard]] constexpr
  std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>
  operator()(G &&g, edge_t<std::remove_cvref_t<G>> e) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member_fn<G>)
      return std::forward<G>(g).endpoints( e);
    else if constexpr (s_adl_fn<G>)
      return endpoints(std::forward<G>(g), e);
    else
      return std::pair<vertex_t<std::remove_cvref_t<G>>, vertex_t<std::remove_cvref_t<G>>>{source(g, e), target(g, e)};
  }
};

} // namespace detail

inline constexpr
detail::endpoints_fn
endpoints;


namespace detail
{
struct place_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { { std::declval<G &&>().place_vertex() } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { { place_vertex(std::declval<G &&>()) } -> std::convertible_to<vertex_t<std::remove_cvref_t<G>>>; };

  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().place_vertex());
    else
      return noexcept(place_vertex(std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  vertex_t<std::remove_cvref_t<G>>
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).place_vertex();
    else
      return place_vertex(std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::place_vertex_fn
place_vertex;


namespace detail
{
struct place_edge_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires
  {
    { std::declval<G &&>().place_edge( std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()) }
    -> std::convertible_to<std::pair<edge_t<std::remove_cvref_t<G>>, bool>>;
  };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires
  {
    { place_edge(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()) }
    -> std::convertible_to<std::pair<edge_t<std::remove_cvref_t<G>>, bool>>;
  };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().place_edge( std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(place_edge(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  std::pair<edge_t<std::remove_cvref_t<G>>, bool>
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> u, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      return std::forward<G>(g).place_edge( u, v);
    else
      return place_edge(std::forward<G>(g), u, v);
  }
};

} // namespace detail

inline constexpr
detail::place_edge_fn
place_edge;


namespace detail
{
struct remove_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().remove_vertex( std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { remove_vertex(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().remove_vertex( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(remove_vertex(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  void
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      std::forward<G>(g).remove_vertex( v);
    else
      remove_vertex(std::forward<G>(g), v);
  }
};

} // namespace detail

inline constexpr
detail::remove_vertex_fn
remove_vertex;


namespace detail
{
struct remove_edge_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().remove_edge( std::declval<edge_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { remove_edge(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().remove_edge( std::declval<edge_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(remove_edge(std::declval<G &&>(), std::declval<edge_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  void
  operator()(G &&g, edge_t<std::remove_cvref_t<G>> e) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      std::forward<G>(g).remove_edge( e);
    else
      remove_edge(std::forward<G>(g), e);
  }
};

} // namespace detail

inline constexpr
detail::remove_edge_fn
remove_edge;


namespace detail
{
struct remove_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires
  { std::declval<G &&>().remove_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires
  { remove_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };


  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().remove_edges( std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(remove_edges(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  void
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> u, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      std::forward<G>(g).remove_edges( u, v);
    else
      remove_edges(std::forward<G>(g), u, v);
  }
};

} // namespace detail

inline constexpr
detail::remove_edges_fn
remove_edges;


namespace detail
{
struct clear_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member_fn
  = requires { std::declval<G &&>().clear_vertex( std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  static constexpr bool s_adl_fn
  = requires { clear_vertex(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()); };

  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<G>)
      return noexcept(std::declval<G &&>().clear_vertex( std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
    else
      return noexcept(clear_vertex(std::declval<G &&>(), std::declval<vertex_t<std::remove_cvref_t<G>> &>()));
  }

public:
  template<typename G>
  requires s_member_fn<G> || s_adl_fn<G>
  constexpr
  void
  operator()(G &&g, vertex_t<std::remove_cvref_t<G>> v) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member_fn<G>)
      std::forward<G>(g).clear_vertex( v);
    else
      clear_vertex(std::forward<G>(g), v);
  }
};

} // namespace detail

inline constexpr
detail::clear_vertex_fn
clear_vertex;

} // namespace heim::graphs

#endif // HEIM_GRAPH_STRUCTURE_PRIMITIVES_HPP
