#ifndef HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
#define HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/lib/utility.hpp"

namespace heim::graphs
{
namespace detail
{
template<typename T>
struct adjacency_list_vertex_descriptor
{
  T value;


  friend constexpr bool operator== (adjacency_list_vertex_descriptor, adjacency_list_vertex_descriptor) noexcept = default;
  friend constexpr auto operator<=>(adjacency_list_vertex_descriptor, adjacency_list_vertex_descriptor) noexcept = default;
};


template<typename T, bool IsDirected>
struct adjacency_list_edge_descriptor
{ };

template<typename T>
struct adjacency_list_edge_descriptor<T, false>
{
  T value, tail, head;


  friend constexpr bool operator== (adjacency_list_edge_descriptor, adjacency_list_edge_descriptor) noexcept = default;
  friend constexpr auto operator<=>(adjacency_list_edge_descriptor, adjacency_list_edge_descriptor) noexcept = default;
};

template<typename T>
struct adjacency_list_edge_descriptor<T, true>
{
  T value;


  friend constexpr bool operator== (adjacency_list_edge_descriptor, adjacency_list_edge_descriptor) noexcept = default;
  friend constexpr auto operator<=>(adjacency_list_edge_descriptor, adjacency_list_edge_descriptor) noexcept = default;
};


template<
    typename T,
    bool     IsOutIncident,
    bool     IsInIncident>
struct adjacency_list_incident_edge_descriptor
{ };

template<typename T>
struct adjacency_list_incident_edge_descriptor<T, true , false>
{
  T value, head;


  friend constexpr
  bool
  operator== (adjacency_list_incident_edge_descriptor const &, adjacency_list_incident_edge_descriptor const &)
  noexcept
  = default;

  friend constexpr
  auto
  operator<=>(adjacency_list_incident_edge_descriptor const &, adjacency_list_incident_edge_descriptor const &)
  noexcept
  = default;
};

template<typename T>
struct adjacency_list_incident_edge_descriptor<T, false, true>
{
  T value, tail;


  friend constexpr
  bool
  operator== (adjacency_list_incident_edge_descriptor const &, adjacency_list_incident_edge_descriptor const &)
  noexcept
  = default;

  friend constexpr
  auto
  operator<=>(adjacency_list_incident_edge_descriptor const &, adjacency_list_incident_edge_descriptor const &)
  noexcept
  = default;
};


template<
    typename T,
    bool     IsOutIncident,
    bool     IsInIncident,
    typename Allocator>
struct adjacency_list_incident_edge_container
{ };

template<typename T, typename Allocator>
struct adjacency_list_incident_edge_container<T, true, false, Allocator>
{
private:
  using container_type
  = std::vector<
      adjacency_list_incident_edge_descriptor<T, true, false>,
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          adjacency_list_incident_edge_descriptor<T, true, false>>>;

public:
  container_type out;


  friend constexpr
  bool
  operator== (adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;

  friend constexpr
  auto
  operator<=>(adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;
};

template<typename T, typename Allocator>
struct adjacency_list_incident_edge_container<T, false, true, Allocator>
{
private:
  using container_type
  = std::vector<
      adjacency_list_incident_edge_descriptor<T, false, true>,
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          adjacency_list_incident_edge_descriptor<T, false, true>>>;

public:
  container_type in;


  friend constexpr
  bool
  operator== (adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;

  friend constexpr
  auto
  operator<=>(adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;
};

template<typename T, typename Allocator>
struct adjacency_list_incident_edge_container<T, true, true, Allocator>
{
private:
  using out_container_type
  = std::vector<
      adjacency_list_incident_edge_descriptor<T, true, false>,
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          adjacency_list_incident_edge_descriptor<T, true, false>>>;

  using in_container_type
  = std::vector<
      adjacency_list_incident_edge_descriptor<T, false, true>,
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          adjacency_list_incident_edge_descriptor<T, false, true>>>;

public:
  out_container_type out;
  in_container_type  in;


  friend constexpr
  bool
  operator== (adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;

  friend constexpr
  auto
  operator<=>(adjacency_list_incident_edge_container const &, adjacency_list_incident_edge_container const &)
  noexcept
  = default;
};


template<
    typename T,
    bool     IsOutIncident,
    bool     IsInIncident,
    typename Allocator>
using adjacency_list_vertex_container
= std::vector<
    adjacency_list_incident_edge_container<T, IsOutIncident, IsInIncident, Allocator>,
    typename std::allocator_traits<Allocator>::template rebind_alloc<
        adjacency_list_incident_edge_container<T, IsOutIncident, IsInIncident, Allocator>>>;


template<typename T, typename Allocator>
using adjacency_list_edge_container
= std::vector<
    std::pair<adjacency_list_vertex_descriptor<T>, adjacency_list_vertex_descriptor<T>>,
    typename std::allocator_traits<Allocator>::template rebind_alloc<
        std::pair<adjacency_list_vertex_descriptor<T>, adjacency_list_vertex_descriptor<T>>>>;

} // namespace detail


/*!
 * \brief
 *   ...
 */
template<
    bool     IsDirected,
    bool     IsMultigraph,
    bool     IsOutIncident = true,
    bool     IsInIncident  = true,
    typename T             = std::size_t,
    typename Allocator     = std::allocator<T>>
requires std::unsigned_integral<T>
      && allocator_for         <Allocator, T>
      && (IsOutIncident || IsInIncident)
class adjacency_list
{
public:
  static constexpr bool is_directed     = IsDirected;
  static constexpr bool is_multigraph   = IsMultigraph;
  static constexpr bool is_out_incident = IsOutIncident;
  static constexpr bool is_in_incident  = IsInIncident;

  using value_type     = T;
  using allocator_type = Allocator;

  using vertex_descriptor = detail::adjacency_list_vertex_descriptor<value_type>;
  using edge_descriptor   = detail::adjacency_list_edge_descriptor  <value_type, is_directed>;

private:
  using vertex_container = detail::adjacency_list_vertex_container<value_type, is_out_incident, is_in_incident, allocator_type>;
  using edge_container   = detail::adjacency_list_edge_container  <value_type, allocator_type>;

  using vertex_allocator = typename vertex_container::allocator_type;
  using edge_allocator   = typename edge_container  ::allocator_type;

private:
  vertex_container m_vertices;
  edge_container   m_edges;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_default_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return
        std::is_nothrow_constructible_v<
            vertex_container,
            vertex_container &&, vertex_allocator const &>
     && std::is_nothrow_constructible_v<
            edge_container,
            edge_container   &&, edge_allocator   const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<vertex_container>
        && std::is_nothrow_swappable_v<edge_container>;
  }

public:
  explicit constexpr
  adjacency_list(allocator_type const &alloc)
  noexcept
    : m_vertices{vertex_allocator{alloc}}
    , m_edges   {edge_allocator  {alloc}}
  { }

  constexpr
  adjacency_list()
  noexcept(s_noexcept_default_construct())
    : adjacency_list{allocator_type{}}
  { }

  constexpr
  adjacency_list(adjacency_list const &other, allocator_type const &alloc)
    : m_vertices{other.m_vertices, vertex_allocator{alloc}}
    , m_edges   {other.m_edges   , edge_allocator  {alloc}}
  { }

  constexpr
  adjacency_list(adjacency_list const &other)
  = default;

  constexpr
  adjacency_list(adjacency_list &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_vertices{std::move(other.m_vertices), vertex_allocator{alloc}}
    , m_edges   {std::move(other.m_edges   ), edge_allocator  {alloc}}
  { }

  constexpr
  adjacency_list(adjacency_list &&other)
  = default;

  constexpr
  ~adjacency_list()
  = default;

  constexpr
  adjacency_list &
  operator=(adjacency_list const &other)
  = default;

  constexpr
  adjacency_list &
  operator=(adjacency_list &&other)
  = default;

  constexpr
  void
  swap(adjacency_list &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;
    swap(m_vertices, other.m_vertices);
    swap(m_edges   , other.m_edges);
  }

  friend constexpr
  void
  swap(adjacency_list &lhs, adjacency_list &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(adjacency_list const &, adjacency_list const &)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{m_vertices.get_allocator()}; }
};

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
