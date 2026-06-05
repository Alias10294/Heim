#ifndef HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
#define HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP

#include <cstddef>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include "heim/lib/graph/interface/primitives.hpp"

namespace heim::graphs
{
template<
    typename Direction,
    typename UInt      = std::size_t,
    typename Allocator = std::allocator<UInt>>
class adjacency_list
{ };


template<
    typename UInt,
    typename Allocator>
class adjacency_list<
    undirected_graph_tag,
    UInt,
    Allocator>
{
public:
  using direction_type = undirected_graph_tag;
  using unsigned_type  = UInt;
  using allocator_type = Allocator;

  struct vertex_descriptor { friend adjacency_list; private: unsigned_type value; };
  struct edge_descriptor   { friend adjacency_list; private: unsigned_type value, tail, head; };

private:
  using alloc_traits
  = std::allocator_traits<allocator_type>;


  struct out_edge_descriptor { friend adjacency_list; private: unsigned_type value, head; };

  using out_edge_allocator = typename alloc_traits::template rebind_alloc<out_edge_descriptor>;
  using out_edge_container = std::vector<out_edge_descriptor, out_edge_allocator>;

  using vertex_allocator = typename alloc_traits::template rebind_alloc<out_edge_container>;
  using vertex_container = std::vector<out_edge_container, vertex_allocator>;

  using vertex_pair    = std::pair<unsigned_type, unsigned_type>;
  using edge_allocator = typename alloc_traits::template rebind_alloc<vertex_pair>;
  using edge_container = std::vector<vertex_pair, edge_allocator>;

private:
  vertex_container m_vertices;
  edge_container   m_edges;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  {
    return std::is_nothrow_default_constructible_v<vertex_allocator>
        && std::is_nothrow_default_constructible_v<edge_allocator>;
  }

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return
        std::is_nothrow_constructible_v<
            vertex_container,
            vertex_container &&, vertex_allocator>
     && std::is_nothrow_constructible_v<
            edge_container,
            edge_container &&, edge_allocator>;
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

  [[nodiscard]]
  constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{m_vertices.get_allocator()}; }

  constexpr
  void
  swap(adjacency_list &other)
  noexcept(s_noexcept_swap())
  {
    std::swap(m_vertices, other.m_vertices);
    std::swap(m_edges   , other.m_edges);
  }

  friend constexpr
  void
  swap(adjacency_list &lhs, adjacency_list &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }


  [[nodiscard]]
  constexpr
  auto
  vertices() const
  noexcept
  {
    return std::views::iota(
               unsigned_type{},
               unsigned_type{m_vertices.size()})
         | std::views::transform(
               [](unsigned_type const val)
               {
                 return vertex_descriptor{val};
               });
  }

  [[nodiscard]]
  constexpr
  auto
  edges() const
  noexcept
  {
    return std::views::iota(
               unsigned_type{},
               unsigned_type{m_edges.size()})
         | std::views::transform(
               [this](unsigned_type const val)
               {
                 auto   ends           {m_edges[val]};
                 return edge_descriptor{val, ends.first, ends.second};
               });
  }

  [[nodiscard]]
  constexpr
  auto
  out_edges(vertex_descriptor const v) const
  noexcept
  {
    unsigned_type tail{v.value};

    return m_vertices[tail]
         | std::views::transform(
               [tail](out_edge_descriptor const e)
               {
                 return edge_descriptor{e.value, tail, e.head};
               });
  }

  [[nodiscard]]
  constexpr
  vertex_descriptor
  head(edge_descriptor const e) const
  noexcept
  { return vertex_descriptor{e.head}; }

  [[nodiscard]]
  constexpr
  vertex_descriptor
  tail(edge_descriptor const e) const
  noexcept
  { return vertex_descriptor{e.tail}; }
};

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_STRUCTURE_ADJACENCY_LIST_HPP
