#ifndef HEIM_GRAPH_STRUCTURE_IMPLEMENTATION_ADJACENCY_LIST_HPP
#define HEIM_GRAPH_STRUCTURE_IMPLEMENTATION_ADJACENCY_LIST_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <inplace_vector>
#include <functional>
#include <memory>
#include <ranges>
#include <scoped_allocator>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/common/sparse.hpp"

namespace heim
{
namespace detail
{
template<
    bool        IsDirected,
    bool        IsMultigraph,
    bool        IsInIncident,
    bool        IsOutIncident,
    typename    UInt,
    std::size_t PageSize,
    typename    Allocator>
class adjacency_list_vertex_context
{
public:
  static constexpr bool        is_directed     = IsDirected;
  static constexpr bool        is_multigraph   = IsMultigraph;
  static constexpr bool        is_in_incident  = IsInIncident;
  static constexpr bool        is_out_incident = IsOutIncident;
  static constexpr std::size_t page_size       = PageSize;

  using id_type        = UInt;
  using allocator_type = Allocator;
  using size_type      = std::size_t;

  class incident_edge
  {
  private:
    id_type m_id;
    id_type m_v_id;

  public:
    constexpr
    incident_edge()
    = default;

    constexpr
    incident_edge(id_type id, id_type v_id)
    noexcept
      : m_id{id}, m_v_id{v_id}
    { }

    [[nodiscard]] friend constexpr
    bool
    operator==(incident_edge const &, incident_edge const &)
    = default;


    [[nodiscard]] constexpr
    id_type
    id() const
    noexcept
    { return m_id; }

    [[nodiscard]] constexpr
    id_type
    vertex_id() const
    noexcept
    { return m_v_id; }
  };

private:
  template<typename T>
  using rebind_alloc
  = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

  using incident_allocator = rebind_alloc<incident_edge>;
  using incident_container = std::vector<incident_edge, incident_allocator>;


  static constexpr bool has_distinct_in_storage
  = is_directed && is_out_incident && is_in_incident;

  using storage_type
  = std::conditional_t<
      has_distinct_in_storage,
      std::tuple<incident_container, incident_container>,
      std::tuple<incident_container>>;

private:
  storage_type m_storage;

private:
  [[nodiscard]] constexpr
  incident_container &
  m_out_edges()
  noexcept
  requires is_out_incident
  { return std::get<0>(this->m_storage); }

  [[nodiscard]] constexpr
  incident_container const &
  m_out_edges() const
  noexcept
  requires is_out_incident
  { return std::get<0>(this->m_storage); }

  [[nodiscard]] constexpr
  incident_container &
  m_in_edges()
  noexcept
  requires is_in_incident
  {
    if constexpr (has_distinct_in_storage)
      return std::get<1>(this->m_storage);
    else
      return std::get<0>(this->m_storage);
  }

  [[nodiscard]] constexpr
  incident_container const &
  m_in_edges() const
  noexcept
  requires is_in_incident
  {
    if constexpr (has_distinct_in_storage)
      return std::get<1>(this->m_storage);
    else
      return std::get<0>(this->m_storage);
  }

  static constexpr
  bool
  s_contains_vertex_id(incident_container const &cont, id_type v_id)
  noexcept
  {
    return std::ranges::any_of(
        cont,
        [v_id]
            (auto const ie)
            { return ie.vertex_id() == v_id; });
  }


  static constexpr
  void
  s_erase_edge_id(incident_container &cont, id_type e_id)
  noexcept
  {
    if (e_id != cont.back().id())
      *std::ranges::find(cont, e_id, &incident_edge::id) = std::move(cont.back());

    cont.pop_back();
  }

  template<typename OnErase>
  static constexpr
  void
  s_erase_vertex_id(incident_container &cont, id_type v_id, OnErase &&on_erase)
  {
    for (size_type pos{cont.size()}; pos != 0;)
    {
      --pos;

      incident_edge &ie{cont[pos]};

      if (ie.vertex_id() != v_id)
        continue;

      std::invoke(on_erase, ie.id());

      if (pos + 1 != cont.size())
        ie = std::move(cont.back());

      cont.pop_back();
    }
  }

public:
  constexpr
  adjacency_list_vertex_context()
    : adjacency_list_vertex_context{allocator_type{}}
  { }

  constexpr
  adjacency_list_vertex_context(adjacency_list_vertex_context const &)
  = default;

  constexpr
  adjacency_list_vertex_context(adjacency_list_vertex_context &&)
  = default;

  explicit constexpr
  adjacency_list_vertex_context(allocator_type const &alloc)
  noexcept
    : m_storage{std::allocator_arg, alloc}
  { }

  constexpr
  adjacency_list_vertex_context(adjacency_list_vertex_context const &other, allocator_type const &alloc)
    : m_storage{std::allocator_arg, alloc, other.m_storage}
  { }

  constexpr
  adjacency_list_vertex_context(adjacency_list_vertex_context &&other, allocator_type const &alloc)
    : m_storage{std::allocator_arg, alloc, std::move(other.m_storage)}
  { }

  constexpr
  ~adjacency_list_vertex_context()
  = default;

  constexpr
  adjacency_list_vertex_context &
  operator=(adjacency_list_vertex_context const &)
  = default;

  constexpr
  adjacency_list_vertex_context &
  operator=(adjacency_list_vertex_context &&)
  = default;

  constexpr
  void
  swap(adjacency_list_vertex_context &other)
  { std::ranges::swap(this->m_storage, other.m_storage); }

  friend constexpr
  void
  swap(adjacency_list_vertex_context &lhs, adjacency_list_vertex_context &rhs)
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(adjacency_list_vertex_context const &, adjacency_list_vertex_context const &)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{std::get<0>(this->m_storage).get_allocator()}; }


  [[nodiscard]] constexpr
  auto
  out_edges() const
  noexcept
  requires is_out_incident
  { return this->m_out_edges() | std::views::all; }

  [[nodiscard]] constexpr
  auto
  in_edges() const
  noexcept
  requires is_in_incident
  { return this->m_in_edges() | std::views::all; }

  [[nodiscard]] constexpr
  auto
  out_edge_count() const
  noexcept
  requires is_out_incident
  { return std::ranges::size(this->m_out_edges()); }

  [[nodiscard]] constexpr
  auto
  in_edge_count() const
  noexcept
  requires is_in_incident
  { return std::ranges::size(this->m_in_edges()); }

  [[nodiscard]] constexpr
  auto
  total_edge_count() const
  noexcept
  requires is_out_incident && is_in_incident
  {
    if constexpr (is_directed)
      return std::ranges::size(this->m_out_edges()) + std::ranges::size(this->m_in_edges());
    else
      return std::ranges::size(std::get<0>(this->m_storage));
  }

  [[nodiscard]] constexpr
  id_type
  out_back_edge_id() const
  noexcept
  requires is_out_incident
  { return this->m_out_edges().back().id(); }

  [[nodiscard]] constexpr
  id_type
  in_back_edge_id() const
  noexcept
  requires is_in_incident
  { return this->m_in_edges().back().id(); }

  [[nodiscard]] constexpr
  bool
  out_contains_vertex_id(id_type v_id) const
  noexcept
  requires is_out_incident
  { return this->s_contains_vertex_id(this->m_out_edges(), v_id); }

  [[nodiscard]] constexpr
  bool
  in_contains_vertex_id(id_type v_id) const
  noexcept
  requires is_in_incident
  { return this->s_contains_vertex_id(this->m_in_edges(), v_id); }


  constexpr
  void
  out_emplace_back(id_type e_id, id_type v_id)
  requires is_out_incident
  { this->m_out_edges().emplace_back(e_id, v_id); }

  constexpr
  void
  in_emplace_back(id_type e_id, id_type v_id)
  requires is_in_incident
  { this->m_in_edges().emplace_back(e_id, v_id); }

  constexpr
  void
  out_pop_back()
  noexcept
  requires is_out_incident
  { this->m_out_edges().pop_back(); }

  constexpr
  void
  in_pop_back()
  noexcept
  requires is_in_incident
  { this->m_in_edges().pop_back(); }

  constexpr
  void
  out_erase_edge_id(id_type e_id)
  noexcept
  requires is_out_incident
  { this->s_erase_edge_id(this->m_out_edges(), e_id); }

  constexpr
  void
  in_erase_edge_id(id_type e_id)
  noexcept
  requires is_in_incident
  { this->s_erase_edge_id(this->m_in_edges(), e_id); }

  template<typename OnErase>
  constexpr
  void
  out_erase_vertex_id(id_type v_id, OnErase &&on_erase)
  requires is_out_incident
  { return this->s_erase_vertex_id(this->m_out_edges(), v_id, std::forward<OnErase>(on_erase)); }

  constexpr
  void
  out_erase_vertex_id(id_type v_id)
  requires is_out_incident
  { return this->s_erase_vertex_id(this->m_out_edges(), v_id, [](id_type){}); }

  template<typename OnErase>
  constexpr
  void
  in_erase_vertex_id(id_type v_id, OnErase &&on_erase)
  requires is_in_incident
  { return this->s_erase_vertex_id(this->m_in_edges(), v_id, std::forward<OnErase>(on_erase)); }

  constexpr
  void
  in_erase_vertex_id(id_type v_id)
  requires is_in_incident
  { return this->s_erase_vertex_id(this->m_in_edges(), v_id, [](id_type){}); }

  template<typename OnErase>
  constexpr
  void
  undirected_erase_vertex_id(id_type v_id, OnErase &&on_erase)
  requires (!is_directed)
  { return this->s_erase_vertex_id(std::get<0>(this->m_storage), v_id, std::forward<OnErase>(on_erase)); }

  constexpr
  void
  undirected_erase_vertex_id(id_type v_id)
  requires (!is_directed)
  { return this->s_erase_vertex_id(std::get<0>(this->m_storage), v_id, [](id_type){}); }
};


template<typename Graph, typename UInt>
class adjacency_list_vertex
{
  friend Graph;

private:
  UInt m_id;

public:
  constexpr
  adjacency_list_vertex()
  = default;

  constexpr
  adjacency_list_vertex(UInt id)
  noexcept
    : m_id{id}
  { }

  [[nodiscard]] friend constexpr
  bool
  operator==(adjacency_list_vertex const &, adjacency_list_vertex const &)
  = default;

  [[nodiscard]] friend constexpr
  auto
  operator<=>(adjacency_list_vertex const &, adjacency_list_vertex const &)
  = default;


  constexpr
  operator UInt() const
  noexcept
  { return this->m_id; }

  [[nodiscard]] constexpr
  UInt
  id() const
  noexcept
  { return this->m_id; }


  friend constexpr
  std::ostream &
  operator<<(std::ostream &os, adjacency_list_vertex const &v)
  { return os << v.m_id; }
};


template<typename Graph, typename UInt>
class adjacency_list_edge
{

  friend Graph;

private:
  UInt m_id, m_src, m_tgt;

private:
  constexpr
  adjacency_list_edge(UInt id, UInt src, UInt tgt)
  noexcept
    : m_id{id}, m_src{src}, m_tgt{tgt}
  { }

public:
  constexpr
  adjacency_list_edge()
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(adjacency_list_edge const &lhs, adjacency_list_edge const &rhs)
  noexcept
  { return lhs.m_id == rhs.m_id; }

  [[nodiscard]] friend constexpr
  auto
  operator<=>(adjacency_list_edge const &lhs, adjacency_list_edge const &rhs)
  noexcept
  { return lhs.m_id <=> rhs.m_id; }


  constexpr
  operator UInt() const
  noexcept
  { return this->m_id; }

  [[nodiscard]] constexpr
  UInt
  id() const
  noexcept
  { return this->m_id; }

  [[nodiscard]] constexpr
  UInt
  source_id() const
  noexcept
  { return this->m_src; }

  [[nodiscard]] constexpr
  UInt
  target_id() const
  noexcept
  { return this->m_tgt; }


  friend constexpr
  std::ostream &
  operator<<(std::ostream &os, adjacency_list_edge const &e)
  { return os << e.m_id; }
};

} // namespace detail


template<
    bool        IsDirected    = true,
    bool        IsMultigraph  = true,
    bool        IsInIncident  = true,
    bool        IsOutIncident = true,
    typename    UInt          = unsigned,
    std::size_t PageSize      = default_page_size_v,
    typename    Allocator     = std::allocator<UInt>>
requires std::unsigned_integral<UInt>
      && std::same_as<UInt, typename std::allocator_traits<Allocator>::value_type>
      && (IsDirected
           ? IsOutIncident || IsInIncident
           : IsOutIncident && IsInIncident)
class adjacency_list
{
public:
  static constexpr bool        is_directed     = IsDirected;
  static constexpr bool        is_multigraph   = IsMultigraph;
  static constexpr bool        is_in_incident  = IsInIncident;
  static constexpr bool        is_out_incident = IsOutIncident;
  static constexpr std::size_t page_size       = PageSize;

  using id_type        = UInt;
  using allocator_type = Allocator;
  using size_type      = std::size_t;

  using vertex_type = detail::adjacency_list_vertex<adjacency_list, id_type>;
  using edge_type   = detail::adjacency_list_edge  <adjacency_list, id_type>;

private:
  using vertex_context
  = detail::adjacency_list_vertex_context<
      is_directed,
      is_multigraph,
      is_in_incident,
      is_out_incident,
      id_type,
      page_size,
      allocator_type>;

  using edge_context
  = std::pair<id_type, id_type>;


  template<typename T>
  using rebind_alloc
  = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

  using id_allocator                 = rebind_alloc<id_type>;
  using outer_v_context_allocator    = rebind_alloc<vertex_context>;
  using v_context_allocator          = std::scoped_allocator_adaptor<outer_v_context_allocator, allocator_type>;
  using outer_id_v_context_allocator = rebind_alloc<std::pair<id_type, vertex_context>>;
  using id_v_context_allocator       = std::scoped_allocator_adaptor<outer_id_v_context_allocator, allocator_type>;
  using e_context_allocator          = rebind_alloc<edge_context>;
  using id_e_context_allocator       = rebind_alloc<std::pair<id_type, edge_context>>;


  using vertex_container
  = sparse_map<
      id_type,
      vertex_context,
      page_size,
      std::vector<id_type       , id_allocator>,
      std::vector<vertex_context, v_context_allocator>,
      id_v_context_allocator>;

  using edge_container
  = sparse_map<
      id_type,
      edge_context,
      page_size,
      std::vector<id_type     , id_allocator>,
      std::vector<edge_context, e_context_allocator>,
      id_e_context_allocator>;

private:
  edge_container   m_edges;
  vertex_container m_vertices;
  id_type          m_next_edge;
  id_type          m_next_vertex;

private:
  template<typename InputIt>
  constexpr
  void
  m_assign_from_edge_list(InputIt first, InputIt last)
  {
    auto assure_vertex
    = [this]
        (id_type const v_id)
        {
          if (this->m_vertices.contains(v_id))
            return;

          this->m_vertices.emplace(v_id);
          if (v_id >= this->m_next_vertex)
            this->m_next_vertex = v_id + 1;
        };

    for (; first != last; ++first)
    {
      auto [u_id, v_id]{*first};

      assure_vertex(u_id);
      assure_vertex(v_id);
      this->place_edge(vertex_type{u_id}, vertex_type{v_id});
    }
  }

  [[nodiscard]] constexpr
  bool
  m_contains_edge(id_type u_id, id_type v_id) const
  noexcept
  {
    if constexpr (is_out_incident)
      return this->m_vertices[u_id].out_contains_vertex_id(v_id);
    else
      return this->m_vertices[v_id].in_contains_vertex_id (u_id);
  }


  constexpr
  void
  m_clear_vertex(id_type v_id)
  {
    auto &v_context{this->m_vertices[v_id]};

    if      constexpr (!is_directed)
    {
      while (v_context.out_edge_count() != 0)
        this->m_remove_edge(v_context.out_back_edge_id());
    }
    else if constexpr (is_out_incident && is_in_incident)
    {
      while (v_context.out_edge_count() != 0)
        this->m_remove_edge(v_context.out_back_edge_id());

      while (v_context.in_edge_count () != 0)
        this->m_remove_edge(v_context.in_back_edge_id ());
    }
    else if constexpr (is_out_incident)
    {
      while (v_context.out_edge_count() != 0)
        this->m_remove_edge(v_context.out_back_edge_id());

      for (auto [id, context] : this->m_vertices)
      {
        if (id == v_id)
          continue;

        context.out_erase_vertex_id(
            v_id,
            [this]
                (id_type e_id)
                { this->m_edges.erase(e_id); });
      }
    }
    else
    {
      while (v_context.in_edge_count() != 0)
        this->m_remove_edge(v_context.in_back_edge_id());

      for (auto [id, context] : this->m_vertices)
      {
        if (id == v_id)
          continue;

        context.in_erase_vertex_id(
            v_id,
            [this]
                (id_type e_id)
                { this->m_edges.erase(e_id); });
      }
    }
  }

  constexpr
  void
  m_remove_edge(id_type e_id)
  noexcept
  {
    auto [u_id, v_id]{this->m_edges[e_id]};

    this->m_edges.erase(e_id);

    if constexpr (is_out_incident)
      this->m_vertices[u_id].out_erase_edge_id(e_id);
    if constexpr (is_in_incident)
      this->m_vertices[v_id].in_erase_edge_id (e_id);
  }

public:
  constexpr
  adjacency_list()
    : adjacency_list{allocator_type{}}
  { }

  constexpr
  adjacency_list(adjacency_list const &)
  = default;

  constexpr
  adjacency_list(adjacency_list &&)
  = default;

  constexpr
  adjacency_list(allocator_type const &alloc)
    : m_edges      {id_e_context_allocator{alloc}}
    , m_vertices   {id_v_context_allocator{outer_id_v_context_allocator{alloc}, alloc}}
    , m_next_edge  {}
    , m_next_vertex{}
  { }

  constexpr
  adjacency_list(adjacency_list const &other, allocator_type const &alloc)
    : m_edges      {other.m_edges   , id_e_context_allocator{alloc}}
    , m_vertices   {other.m_vertices, id_v_context_allocator{outer_id_v_context_allocator{alloc}, alloc}}
    , m_next_edge  {other.m_next_edge}
    , m_next_vertex{other.m_next_vertex}
  { }

  constexpr
  adjacency_list(adjacency_list &&other, allocator_type const &alloc)
    : m_edges      {std::move(other.m_edges   ), id_e_context_allocator{alloc}}
    , m_vertices   {std::move(other.m_vertices), id_v_context_allocator{outer_id_v_context_allocator{alloc}, alloc}}
    , m_next_edge  {std::move(other.m_next_edge  )}
    , m_next_vertex{std::move(other.m_next_vertex)}
  { }

  template<typename InputIt>
  constexpr
  adjacency_list(InputIt first, InputIt last)
  requires std::input_iterator<InputIt>
    : adjacency_list{first, last, allocator_type{}}
  { }

  template<typename R>
  constexpr
  adjacency_list(std::from_range_t, R &&rg)
  requires std::ranges::input_range<R>
    : adjacency_list{std::ranges::begin(rg), std::ranges::end(rg)}
  { }

  constexpr
  adjacency_list(std::initializer_list<std::pair<id_type, id_type>> init)
    : adjacency_list{std::ranges::begin(init), std::ranges::end(init)}
  { }

  template<typename InputIt>
  constexpr
  adjacency_list(InputIt first, InputIt last, allocator_type const &alloc)
  requires std::input_iterator<InputIt>
    : adjacency_list{alloc}
  { this->m_assign_from_edge_list(first, last); }

  template<typename R>
  constexpr
  adjacency_list(std::from_range_t, R &&rg, allocator_type const &alloc)
  requires std::ranges::input_range<R>
    : adjacency_list{std::ranges::begin(rg), std::ranges::end(rg), alloc}
  { }

  constexpr
  adjacency_list(std::initializer_list<std::pair<id_type, id_type>> init, allocator_type const &alloc)
    : adjacency_list{std::ranges::begin(init), std::ranges::end(init), alloc}
  { }


  constexpr
  ~adjacency_list()
  = default;

  constexpr
  adjacency_list &
  operator=(adjacency_list const &)
  = default;

  constexpr
  adjacency_list &
  operator=(adjacency_list &&)
  = default;

  constexpr
  adjacency_list &
  operator=(std::initializer_list<std::pair<id_type, id_type>> ilist)
  {
    this->m_vertices.clear();
    this->m_edges   .clear();
    this->m_next_vertex = {};
    this->m_next_edge   = {};

    this->m_assign_from_edge_list(ilist.begin(), ilist.end());
    return *this;
  }

  constexpr
  void
  swap(adjacency_list &other)
  {
    std::ranges::swap(this->m_vertices   , other.m_vertices);
    std::ranges::swap(this->m_edges      , other.m_edges);
    std::ranges::swap(this->m_next_vertex, other.m_next_vertex);
    std::ranges::swap(this->m_next_edge  , other.m_next_edge);
  }

  friend constexpr
  void
  swap(adjacency_list &lhs, adjacency_list &rhs)
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{this->m_vertices.get_allocator().inner_allocator()}; }

  [[nodiscard]] friend constexpr
  bool
  operator==(adjacency_list const &, adjacency_list const &)
  noexcept
  = default;


  [[nodiscard]] constexpr
  auto
  vertices() const
  noexcept
  {
    return this->m_vertices
         | std::views::transform(
               []
                   (auto const &p)
                   { return vertex_type{p.first}; });
  }

  [[nodiscard]] constexpr
  auto
  edges() const
  noexcept
  {
    return this->m_edges
         | std::views::transform(
               []
                   (auto const &p)
                   { return edge_type{p.first, p.second.first, p.second.second}; });
  }

  [[nodiscard]] constexpr
  size_type
  vertex_count() const
  noexcept
  { return this->m_vertices.size(); }

  [[nodiscard]] constexpr
  size_type
  edge_count() const
  noexcept
  { return this->m_edges.size(); }

  [[nodiscard]] constexpr
  auto
  out_edges(vertex_type v) const
  noexcept
  requires is_out_incident
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    return this->m_vertices[v_id].out_edges()
         | std::views::transform(
               [v_id]
                   (auto const &oe)
                   { return edge_type{oe.id(), v_id, oe.vertex_id()}; });
  }

  [[nodiscard]] constexpr
  auto
  in_edges(vertex_type v) const
  noexcept
  requires is_in_incident
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    return this->m_vertices[v_id].in_edges()
         | std::views::transform(
               [v_id]
                   (auto const &ie)
                   { return edge_type{ie.id(), ie.vertex_id(), v_id}; });
  }

  [[nodiscard]] constexpr
  size_type
  out_edge_count(vertex_type v) const
  noexcept
  requires is_out_incident
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    return static_cast<size_type>(this->m_vertices[v_id].out_edge_count());
  }

  [[nodiscard]] constexpr
  size_type
  in_edge_count(vertex_type v) const
  noexcept
  requires is_in_incident
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    return static_cast<size_type>(this->m_vertices[v_id].in_edge_count());
  }

  [[nodiscard]] constexpr
  size_type
  total_edge_count(vertex_type v) const
  noexcept
  requires is_in_incident && is_out_incident
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    return static_cast<size_type>(this->m_vertices[v_id].total_edge_count());
  }


  [[nodiscard]] constexpr
  vertex_type
  source(edge_type e) const
  noexcept
  {
    contract_assert(this->m_edges.contains(e.id()));

    return vertex_type{e.source_id()};
  }

  [[nodiscard]] constexpr
  vertex_type
  target(edge_type e) const
  noexcept
  {
    contract_assert(this->m_edges.contains(e.id()));

    return vertex_type{e.target_id()};
  }

  [[nodiscard]] constexpr
  std::pair<vertex_type, vertex_type>
  endpoints(edge_type e) const
  noexcept
  {
    id_type e_id{e.id()};

    contract_assert(this->m_edges.contains(e_id));

    auto const [u_id, v_id]{this->m_edges[e_id]};

    return std::pair{vertex_type{u_id}, vertex_type{v_id}};
  }


  constexpr
  vertex_type
  place_vertex()
  {
    this->m_vertices.emplace(this->m_next_vertex);

    return vertex_type{this->m_next_vertex++};
  }

  constexpr
  std::pair<edge_type, bool>
  place_edge(vertex_type u, vertex_type v)
  {
    id_type const e_id{this->m_next_edge};
    id_type const u_id{u.id()};
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(u_id));
    contract_assert(this->m_vertices.contains(v_id));

    if constexpr (!is_multigraph)
    {
      if (this->m_contains_edge(u_id, v_id))
        return std::pair{edge_type{}, false};
    }

    bool out_emplaced{false};
    bool in_emplaced {false};

    try
    {
      if constexpr (is_out_incident)
      { this->m_vertices[u_id].out_emplace_back(e_id, v_id); out_emplaced = true; }

      if constexpr (is_in_incident)
      { this->m_vertices[v_id].in_emplace_back (e_id, u_id); in_emplaced  = true; }

      this->m_edges.emplace(e_id, u_id, v_id);
    }
    catch (...)
    {
      if constexpr (is_in_incident)
      { if (in_emplaced ) this->m_vertices[v_id].in_pop_back(); }

      if constexpr (is_out_incident)
      { if (out_emplaced) this->m_vertices[u_id].out_pop_back(); }

      throw;
    }

    ++this->m_next_edge;
    return std::pair{edge_type{e_id, u_id, v_id}, true};
  }

  constexpr
  void
  remove_vertex(vertex_type v)
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    this->m_clear_vertex(v_id);
    this->m_vertices.erase(v_id);
  }

  constexpr
  void
  remove_edge(edge_type e)
  noexcept
  {
    id_type e_id{e.id()};

    contract_assert(this->m_edges.contains(e_id));

    this->m_remove_edge(e_id);
  }

  constexpr
  void
  remove_edges(vertex_type u, vertex_type v)
  {
    id_type const u_id{u.id()};
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(u_id));
    contract_assert(this->m_vertices.contains(v_id));

    if constexpr (is_directed)
    {
      if      constexpr (is_out_incident && is_in_incident)
      {
        this->m_vertices[u_id].out_erase_vertex_id(v_id);
        this->m_vertices[v_id].in_erase_vertex_id (u_id, [this](id_type e_id){ this->m_edges.erase(e_id); });
      }
      else if constexpr (is_out_incident)
        this->m_vertices[u_id].out_erase_vertex_id(v_id, [this](id_type e_id){ this->m_edges.erase(e_id); });
      else if constexpr (is_in_incident)
        this->m_vertices[v_id].in_erase_vertex_id (u_id, [this](id_type e_id){ this->m_edges.erase(e_id); });
    }
    else
    {
      if (u_id == v_id)
        this->m_vertices[u_id].undirected_erase_vertex_id(v_id, [this](id_type e_id){ this->m_edges.try_erase(e_id); });
      else
      {
        this->m_vertices[u_id].undirected_erase_vertex_id(v_id);
        this->m_vertices[v_id].undirected_erase_vertex_id(u_id, [this](id_type e_id){ this->m_edges.erase(e_id); });
      }
    }
  }

  constexpr
  void
  clear_vertex(vertex_type v)
  {
    id_type const v_id{v.id()};

    contract_assert(this->m_vertices.contains(v_id));

    this->m_clear_vertex(v.id());
  }

  constexpr
  void
  clear()
  noexcept
  {
    this->m_vertices.clear();
    this->m_edges   .clear();
  }
};

} // namespace heim


namespace std
{
template<typename Graph, typename UInt>
struct hash<heim::detail::adjacency_list_vertex<Graph, UInt>>
{
  [[nodiscard]] constexpr
  std::size_t
  operator()(heim::detail::adjacency_list_vertex<Graph, UInt> v) const
  noexcept(noexcept(std::hash<UInt>{}(v.id())))
  { return std::hash<UInt>{}(v.id()); }
};

template<typename Graph, typename UInt>
struct hash<heim::detail::adjacency_list_edge<Graph, UInt>>
{
  [[nodiscard]] constexpr
  std::size_t
  operator()(heim::detail::adjacency_list_edge<Graph, UInt> e) const
  noexcept(noexcept(std::hash<UInt>{}(e.id())))
  { return std::hash<UInt>{}(e.id()); }
};


template<typename Graph, typename UInt>
struct formatter<heim::detail::adjacency_list_vertex<Graph, UInt>>
{
  constexpr
  auto
  parse(format_parse_context &ctx) const
  { return ctx.begin(); }

  constexpr
  auto
  format(heim::detail::adjacency_list_vertex<Graph, UInt> const &v, format_context &ctx) const
  { return format_to(ctx.out(), "{}", v.id()); }
};

template<typename Graph, typename UInt>
struct formatter<heim::detail::adjacency_list_edge<Graph, UInt>>
{
  constexpr
  auto
  parse(format_parse_context &ctx) const
  { return ctx.begin(); }

  constexpr
  auto
  format(heim::detail::adjacency_list_edge<Graph, UInt> const &e, format_context &ctx) const
  { return format_to(ctx.out(), "{}", e.id()); }
};

} // namespace std

#endif // HEIM_GRAPH_STRUCTURE_IMPLEMENTATION_ADJACENCY_LIST_HPP
