#ifndef HEIM_LIB_GRAPH_INTERFACE_PROPERTIES_HPP
#define HEIM_LIB_GRAPH_INTERFACE_PROPERTIES_HPP

#include <concepts>
#include <ranges>
#include "traits.hpp"

namespace heim::graph
{
namespace detail
{
template<typename R, typename G>
concept vertex_descriptor_range
=   std::ranges::range<R>
 && std::same_as<std::ranges::range_value_t<R>, vertex_descriptor_t<G>>;

struct vertices_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().vertices() } -> vertex_descriptor_range<G>; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { vertices(std::declval<G &&>() ) } -> vertex_descriptor_range<G>; };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().vertices());
    else
      return noexcept(vertices(std::declval<G &&>() ));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).vertices();
    else
      return vertices(std::forward<G>(g) );
  }
};

} // namespace detail

inline constexpr
detail::vertices_fn
vertices{};


namespace detail
{
struct vertex_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().vertex_count() }; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { vertex_count(std::declval<G &&>() ) }; };

  template<typename G>
  static constexpr bool s_range
  = requires { { std::ranges::size(vertices(std::declval<G &&>())) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().vertex_count());
    else if constexpr (s_adl   <G>)
      return noexcept(vertex_count(std::declval<G &&>() ));
    else
      return noexcept(std::ranges::size(vertices(std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).vertex_count();
    else if constexpr (s_adl   <G>)
      return vertex_count(std::forward<G>(g) );
    else
      return std::ranges::size(vertices(std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::vertex_count_fn vertex_count{};
inline constexpr detail::vertex_count_fn order       {};


namespace detail
{
template<typename R, typename G>
concept edge_descriptor_range
=   std::ranges::range<R>
 && std::same_as<std::ranges::range_value_t<R>, edge_descriptor_t<G>>;

struct edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().edges() } -> edge_descriptor_range<G>; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { edges(std::declval<G &&>() ) } -> edge_descriptor_range<G>; };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().edges());
    else
      return noexcept(edges(std::declval<G &&>() ));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).edges();
    else
      return edges(std::forward<G>(g) );
  }
};

} // namespace detail

inline constexpr
detail::edges_fn
edges{};


namespace detail
{
struct edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().edge_count() }; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { edge_count(std::declval<G &&>() ) }; };

  template<typename G>
  static constexpr bool s_range
  = requires { { std::ranges::size(edges(std::declval<G &&>())) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().edge_count());
    else if constexpr (s_adl   <G>)
      return noexcept(edge_count(std::declval<G &&>() ));
    else
      return noexcept(std::ranges::size(edges(std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).edge_count();
    else if constexpr (s_adl   <G>)
      return edge_count(std::forward<G>(g));
    else
      return std::ranges::size(edges(std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::edge_count_fn edge_count{};
inline constexpr detail::edge_count_fn size      {};


namespace detail
{
struct out_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
      {
        { std::declval<G &&>().out_edges(std::declval<vertex_descriptor_t<G>>()) }
        -> edge_descriptor_range<G>;
      };

  template<typename G>
  static constexpr bool s_adl
  = requires
      {
        { out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }
        -> edge_descriptor_range<G>;
      };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().out_edges(std::declval<vertex_descriptor_t<G>>()));
    else
      return noexcept(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).out_edges(v);
    else
      return out_edges(v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::out_edges_fn
out_edges{};


namespace detail
{
struct out_edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().out_edge_count(std::declval<vertex_descriptor_t<G>>()) }; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { out_edge_count(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }; };

  template<typename G>
  static constexpr bool s_range
  = requires { { std::ranges::size(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().out_edge_count(std::declval<vertex_descriptor_t<G>>()));
    else if constexpr (s_adl   <G>)
      return noexcept(out_edge_count(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>() ));
    else
      return noexcept(std::ranges::size(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).out_edge_count(v);
    else if constexpr (s_adl   <G>)
      return out_edge_count(v, std::forward<G>(g));
    else
      return std::ranges::size(out_edges(v, std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::out_edge_count_fn out_edge_count{};
inline constexpr detail::out_edge_count_fn out_degree    {};


namespace detail
{
struct in_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
      {
        { std::declval<G &&>().in_edges(std::declval<vertex_descriptor_t<G>>()) }
        -> edge_descriptor_range<G>;
      };

  template<typename G>
  static constexpr bool s_adl
  = requires
      {
        { in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }
        -> edge_descriptor_range<G>;
      };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().in_edges(std::declval<vertex_descriptor_t<G>>()));
    else
      return noexcept(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).in_edges(v);
    else
      return in_edges(v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::in_edges_fn
in_edges{};


namespace detail
{
struct in_edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { { std::declval<G &&>().in_edge_count(std::declval<vertex_descriptor_t<G>>()) }; };

  template<typename G>
  static constexpr bool s_adl
  = requires { { in_edge_count(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>() ) }; };

  template<typename G>
  static constexpr bool s_range
  = requires { { std::ranges::size(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().out_edge_count(std::declval<vertex_descriptor_t<G>>()));
    else if constexpr (s_adl   <G>)
      return noexcept(in_edge_count(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>() ));
    else
      return noexcept(std::ranges::size(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_range<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).out_edge_count(v);
    else if constexpr (s_adl   <G>)
      return in_edge_count(v, std::forward<G>(g));
    else
      return std::ranges::size(in_edges(v, std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr detail::in_edge_count_fn in_edge_count{};
inline constexpr detail::in_edge_count_fn in_degree    {};


namespace detail
{
struct tail_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
      {
        { std::declval<G &&>().tail(std::declval<edge_descriptor_t<G>>()) }
        -> std::same_as<vertex_descriptor_t<G>>;
      };

  template<typename G>
  static constexpr bool s_adl
  = requires
      {
        { tail(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
        -> std::same_as<vertex_descriptor_t<G>>;
      };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().tail(std::declval<edge_descriptor_t<G>>()));
    else
      return noexcept(tail(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  vertex_descriptor_t<G>
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).tail(e);
    else
      return tail(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::tail_fn
tail{};


namespace detail
{
struct head_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().head(std::declval<edge_descriptor_t<G>>()) }
    -> std::same_as<vertex_descriptor_t<G>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { head(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
    -> std::same_as<vertex_descriptor_t<G>>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().head(std::declval<edge_descriptor_t<G>>()));
    else
      return noexcept(head(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]]
  constexpr
  vertex_descriptor_t<G>
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).head(e);
    else
      return head(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::head_fn
head{};


namespace detail
{
struct ends_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
      {
        { std::declval<G &&>().ends( std::declval<edge_descriptor_t<G>>()) }
        -> std::same_as<std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>>;
      };

  template<typename G>
  static constexpr bool s_adl
  = requires
      {
        { ends(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
        -> std::same_as<std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>>;
      };

  template<typename G>
  static constexpr bool s_tail_head
  =   requires { { tail(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }; }
   && requires { { head(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_tail_head<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().ends( std::declval<edge_descriptor_t<G>>()));
    else if constexpr (s_adl   <G>)
      return noexcept(ends(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
    else
    {
      return noexcept(tail(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()))
          && noexcept(head(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
    }
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_tail_head<G>
  [[nodiscard]]
  constexpr
  std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>
  operator()(G&& g, edge_descriptor_t<G> e) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).ends( e);
    else if constexpr (s_adl   <G>)
      return ends(e, std::forward<G>(g));
    else
      return std::pair{tail(e, g), head(e, g)};
  }
};

} // namespace detail

inline constexpr
detail::ends_fn
ends{};


namespace detail
{
struct adjacent_vertices_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
      {
        { std::declval<G &&>().adjacent_vertices(std::declval<vertex_descriptor_t<G>>()) }
        -> vertex_descriptor_range<G>;
      };

  template<typename G>
  static constexpr bool s_adl
  = requires
      {
        { adjacent_vertices(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }
        -> vertex_descriptor_range<G>;
      };

  template<typename G>
  static constexpr bool s_out_edges
  =   requires { { out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &>()) }; }
   && requires { { head     (std::declval<edge_descriptor_t  <G>>(), std::declval<G &>()) }; };


  template<typename G>
  requires s_member<G> || s_adl<G> || s_out_edges<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().adjacent_vertices(std::declval<vertex_descriptor_t<G>>()));
    else if constexpr (s_adl   <G>)
      return noexcept(adjacent_vertices(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else
      return noexcept(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G> || s_out_edges<G>
  [[nodiscard]]
  constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).adjacent_vertices(v);
    else if constexpr (s_adl   <G>)
      return adjacent_vertices(v, std::forward<G>(g));
    else
    {
      return out_edges(v, g)
           | std::views::transform(
                 [v, g](edge_descriptor_t<G> e)
                 {
                   return head(e, g);
                 });
    }
  }
};

} // namespace detail

} // namespace heim::graph

#endif // HEIM_LIB_GRAPH_INTERFACE_PROPERTIES_HPP
