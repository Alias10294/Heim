#ifndef HEIM_LIB_GRAPH_INTERFACE_INTERFACE_HPP
#define HEIM_LIB_GRAPH_INTERFACE_INTERFACE_HPP

#include <concepts>
#include <ranges>
#include <utility>
#include "primitives.hpp"

namespace heim::graphs
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
  [[nodiscard]] constexpr
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
  [[nodiscard]] constexpr
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
struct vertex_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().vertex_count(); };

  template<typename G>
  static constexpr bool s_adl
  = requires { vertex_count(std::declval<G &&>() ); };

  template<typename G>
  static constexpr bool s_vertices
  = requires { std::ranges::size(vertices(std::declval<G &&>())); };


  template<typename G>
  requires s_member  <G> || s_adl<G>
        || s_vertices<G>
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
  requires s_member  <G> || s_adl<G>
        || s_vertices<G>
  [[nodiscard]] constexpr
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
struct edge_count_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().edge_count(); };

  template<typename G>
  static constexpr bool s_adl
  = requires { edge_count(std::declval<G &&>() ); };

  template<typename G>
  static constexpr bool s_edges
  = requires { std::ranges::size(edges(std::declval<G &&>())); };


  template<typename G>
  requires s_member<G> || s_adl<G>
        || s_edges <G>
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
  requires s_member<G> || s_adl<G>
        || s_edges <G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).edge_count();
    else if constexpr (s_adl   <G>)
      return edge_count(std::forward<G>(g) );
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
    { std::declval<G &&>().out_edges(std::declval<vertex_descriptor_t<G>>() ) }
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
      return noexcept(std::declval<G &&>().out_edges(std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).out_edges(v );
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
struct in_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().in_edges(std::declval<vertex_descriptor_t<G>>() ) }
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
      return noexcept(std::declval<G &&>().in_edges(std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).in_edges(v );
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
struct out_degree_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().out_degree(std::declval<vertex_descriptor_t<G>>() ); };

  template<typename G>
  static constexpr bool s_adl
  = requires { out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_out_edges
  = requires { std::ranges::size(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())); };


  template<typename G>
  requires s_member   <G> || s_adl<G>
        || s_out_edges<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().out_degree(std::declval<vertex_descriptor_t<G>>() ));
    else if constexpr (s_adl   <G>)
      return noexcept(out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else
      return noexcept(std::ranges::size(out_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member   <G> || s_adl<G>
        || s_out_edges<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).out_degree(v);
    else if constexpr (s_adl   <G>)
      return out_degree(v, std::forward<G>(g) );
    else
      return std::ranges::size(out_edges(v, std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr
detail::out_degree_fn
out_degree{};


namespace detail
{
struct in_degree_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().in_degree(std::declval<vertex_descriptor_t<G>>() ); };

  template<typename G>
  static constexpr bool s_adl
  = requires { in_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_in_edges
  = requires { std::ranges::size(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())); };


  template<typename G>
  requires s_member  <G> || s_adl<G>
        || s_in_edges<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().in_degree(std::declval<vertex_descriptor_t<G>>() ));
    else if constexpr (s_adl   <G>)
      return noexcept(in_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else
      return noexcept(std::ranges::size(in_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>())));
  }

public:
  template<typename G>
  requires s_member  <G> || s_adl<G>
        || s_in_edges<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member<G>)
      return std::forward<G>(g).in_degree(v);
    else if constexpr (s_adl   <G>)
      return in_degree(v, std::forward<G>(g) );
    else
      return std::ranges::size(in_edges(v, std::forward<G>(g)));
  }
};

} // namespace detail

inline constexpr
detail::in_degree_fn
in_degree{};


namespace detail
{
struct degree_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().degree(std::declval<vertex_descriptor_t<G>>() ); };

  template<typename G>
  static constexpr bool s_adl
  = requires { degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_undirected_out
  =  !is_directed_v<G>
   && requires { out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_undirected_in
  =  !is_directed_v<G>
   && requires { in_degree (std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };

  template<typename G>
  static constexpr bool s_directed_in_out
  =   is_directed_v<G>
   && requires { out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &>()); }
   && requires { in_degree (std::declval<vertex_descriptor_t<G>>(), std::declval<G &>()); };


  template<typename G>
  requires s_member         <G> || s_adl          <G>
        || s_undirected_out <G> || s_undirected_in<G>
        || s_directed_in_out<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if      constexpr (s_member        <G>)
      return noexcept(std::declval<G &&>().degree(std::declval<vertex_descriptor_t<G>>() ));
    else if constexpr (s_adl           <G>)
      return noexcept(degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else if constexpr (s_undirected_out<G>)
      return noexcept(out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else if constexpr (s_undirected_in <G>)
      return noexcept(in_degree (std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
    else
    {
      return noexcept(
          out_degree(std::declval<vertex_descriptor_t<G>>(), std::declval<G &>())
        + in_degree (std::declval<vertex_descriptor_t<G>>(), std::declval<G &>()));
    }
  }

public:
  template<typename G>
  requires s_member         <G> || s_adl          <G>
        || s_undirected_out <G> || s_undirected_in<G>
        || s_directed_in_out<G>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if      constexpr (s_member        <G>)
      return std::forward<G>(g).degree(v );
    else if constexpr (s_adl           <G>)
      return degree(v, std::forward<G>(g));
    else if constexpr (s_undirected_out<G>)
      return out_degree(v, std::forward<G>(g));
    else if constexpr (s_undirected_in <G>)
      return in_degree (v, std::forward<G>(g));
    else
      return out_degree(v, g) + in_degree(v, g);
  }
};

} // namespace detail

inline constexpr
detail::degree_fn
degree{};


namespace detail
{
struct source_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().source(std::declval<edge_descriptor_t<G>>() ) }
    -> std::same_as<vertex_descriptor_t<G>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { source(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
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
      return noexcept(std::declval<G &&>().source(std::declval<edge_descriptor_t<G>>() ));
    else
      return noexcept(source(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  vertex_descriptor_t<G>
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).source(e );
    else
      return source(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::source_fn
source{};


namespace detail
{
struct target_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().target(std::declval<edge_descriptor_t<G>>() ) }
    -> std::same_as<vertex_descriptor_t<G>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { target(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
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
      return noexcept(std::declval<G &&>().target(std::declval<edge_descriptor_t<G>>() ));
    else
      return noexcept(target(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  vertex_descriptor_t<G>
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).target(e );
    else
      return target(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::target_fn
target{};


namespace detail
{
struct endpoints_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().endpoints(std::declval<edge_descriptor_t<G>>() ) }
    -> std::same_as<std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { endpoints(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
    -> std::same_as<std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().endpoints(std::declval<edge_descriptor_t<G>>() ));
    else
      return noexcept(endpoints(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  std::pair<vertex_descriptor_t<G>, vertex_descriptor_t<G>>
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).endpoints(e );
    else
      return endpoints(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::endpoints_fn
endpoints{};


namespace detail
{
struct place_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().place_vertex() }
    -> std::same_as<std::pair<vertex_descriptor_t<G>, bool>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { place_vertex(std::declval<G &&>() ) }
    -> std::same_as<std::pair<vertex_descriptor_t<G>, bool>>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().place_vertex());
    else
      return noexcept(place_vertex(std::declval<G &&>() ));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  std::pair<vertex_descriptor_t<G>, bool>
  operator()(G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).place_vertex();
    else
      return place_vertex(std::forward<G>(g) );
  }
};

} // namespace detail

inline constexpr
detail::place_vertex_fn
place_vertex{};


namespace detail
{
struct place_edge_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().place_edge(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>() ) }
    -> std::same_as<std::pair<edge_descriptor_t<G>, bool>>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { place_edge(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }
    -> std::same_as<std::pair<edge_descriptor_t<G>, bool>>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().place_edge(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(place_edge(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  std::pair<edge_descriptor_t<G>, bool>
  operator()(vertex_descriptor_t<G> u, vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).place_edge(u, v );
    else
      return place_edge(u, v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::place_edge_fn
place_edge{};


namespace detail
{
struct remove_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().remove_vertex(std::declval<vertex_descriptor_t<G>>() ) }
    -> std::same_as<bool>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { remove_vertex(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()) }
    -> std::same_as<bool>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().remove_vertex(std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(remove_vertex(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  bool
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).remove_vertex(v );
    else
      return remove_vertex(v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::remove_vertex_fn
remove_vertex{};


namespace detail
{
struct remove_edge_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires
  {
    { std::declval<G &&>().remove_edge(std::declval<edge_descriptor_t<G>>() ) }
    -> std::same_as<bool>;
  };

  template<typename G>
  static constexpr bool s_adl
  = requires
  {
    { remove_edge(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()) }
    -> std::same_as<bool>;
  };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().remove_edge(std::declval<edge_descriptor_t<G>>() ));
    else
      return noexcept(remove_edge(std::declval<edge_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  [[nodiscard]] constexpr
  bool
  operator()(edge_descriptor_t<G> e, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      return std::forward<G>(g).remove_edge(e );
    else
      return remove_edge(e, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::remove_edge_fn
remove_edge{};


namespace detail
{
struct remove_edges_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().remove_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>() ); };

  template<typename G>
  static constexpr bool s_adl
  = requires { remove_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().remove_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(remove_edges(std::declval<vertex_descriptor_t<G>>(), std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  constexpr
  void
  operator()(vertex_descriptor_t<G> u, vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      std::forward<G>(g).remove_edges(u, v );
    else
      remove_edges(u, v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::remove_edges_fn
remove_edges{};


namespace detail
{
struct clear_vertex_fn
{
private:
  template<typename G>
  static constexpr bool s_member
  = requires { std::declval<G &&>().clear_vertex(std::declval<vertex_descriptor_t<G>>() ); };

  template<typename G>
  static constexpr bool s_adl
  = requires { clear_vertex(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()); };


  template<typename G>
  requires s_member<G> || s_adl<G>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member<G>)
      return noexcept(std::declval<G &&>().clear_vertex(std::declval<vertex_descriptor_t<G>>() ));
    else
      return noexcept(clear_vertex(std::declval<vertex_descriptor_t<G>>(), std::declval<G &&>()));
  }

public:
  template<typename G>
  requires s_member<G> || s_adl<G>
  constexpr
  void
  operator()(vertex_descriptor_t<G> v, G &&g) const
  noexcept(s_noexcept<G>())
  {
    if constexpr (s_member<G>)
      std::forward<G>(g).clear_vertex(v );
    else
      clear_vertex(v, std::forward<G>(g));
  }
};

} // namespace detail

inline constexpr
detail::clear_vertex_fn
clear_vertex{};

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_INTERFACE_INTERFACE_HPP
