#ifndef HEIM_ECS_REGISTRY_SPARSE_SET_HPP
#define HEIM_ECS_REGISTRY_SPARSE_SET_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/identifier.hpp"
#include "heim/lib/unique_allocator_aware_ptr.hpp"

namespace heim::ecs::sparse
{
template<typename = void>
struct default_page_size
  : std::integral_constant<std::size_t, 1024>
{ };

inline constexpr std::size_t default_page_size_v
= default_page_size<>;


namespace detail
{
template<
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
requires (std::unsigned_integral<Id> && allocator_for<Alloc, Id>)
class set_sparse_container
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

  static constexpr std::size_t page_size = PageSz;
  static constexpr bool        is_paged  = page_size != 0;

private:
  using id_traits    = identifier_traits<identifier_type>;
  using alloc_traits = std::allocator_traits<allocator_type>;

  using page
  = std::array<identifier_type, page_size>;

  using page_allocator    = typename alloc_traits::template rebind_alloc <page>;
  using page_alloc_traits = typename alloc_traits::template rebind_traits<page>;

  using page_pointer
  = unique_allocator_aware_ptr<page, page_allocator>;

  using page_pointer_allocator    = typename alloc_traits::template rebind_alloc <page_pointer>;
  using page_pointer_alloc_traits = typename alloc_traits::template rebind_traits<page_pointer>;

  using container_type
  = std::conditional_t<
      is_paged,
      std::vector<page_pointer   , page_pointer_allocator>,
      std::vector<identifier_type, allocator_type        >>;

  using container_allocator    = typename container_type::allocator_type;
  using container_alloc_traits = std::allocator_traits<container_allocator>;

private:
  container_type m_container;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        container_type,
        container_type &&, container_allocator const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_type>; }

  static constexpr
  std::size_t
  s_page_index(std::size_t const idx)
  noexcept
  { return idx / page_size; }

  static constexpr
  std::size_t
  s_line_index(std::size_t const idx)
  noexcept
  { return idx % page_size; }


  constexpr
  void
  m_copy(container_type const &cont)
  {
    m_container.reserve(cont.size());

    for (page_pointer const &ptr : cont)
    {
      if (ptr)
        m_container.emplace_back(make_unique_allocator_aware<page>(page_allocator{m_container.get_allocator()}, *ptr));
      else
        m_container.emplace_back(page_pointer{});
    }
  }


  constexpr
  set_sparse_container(set_sparse_container const &other, allocator_type const &alloc, std::bool_constant<true>)
    : m_container{container_allocator(alloc)}
  { m_copy(other.m_container); }

  constexpr
  set_sparse_container(set_sparse_container const &other, allocator_type const &alloc, std::bool_constant<false>)
    : m_container{other.m_container, container_allocator{alloc}}
  { }

  constexpr
  set_sparse_container(set_sparse_container const &other, std::bool_constant<true>)
    : m_container{container_alloc_traits::select_on_container_copy_construction(other.m_container.get_allocator())}
  { m_copy(other.m_container); }

  constexpr
  set_sparse_container(set_sparse_container const &other, std::bool_constant<false>)
    : m_container{other.m_container}
  { }

public:
  explicit constexpr
  set_sparse_container(allocator_type const &alloc)
  noexcept
    : m_container{container_allocator{alloc}}
  { }

  constexpr
  set_sparse_container(set_sparse_container const &other, allocator_type const &alloc)
    : set_sparse_container{other, alloc, std::bool_constant<is_paged>{}}
  { }

  constexpr
  set_sparse_container(set_sparse_container const &other)
    : set_sparse_container{other, std::bool_constant<is_paged>{}}
  { }

  constexpr
  set_sparse_container(set_sparse_container &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_container{std::move(other.m_container), container_allocator{alloc}}
  { }

  constexpr
  set_sparse_container(set_sparse_container &&)
  = default;

  constexpr
  ~set_sparse_container()
  = default;

  constexpr
  set_sparse_container &
  operator=(set_sparse_container const &other)
  {
    if constexpr (is_paged)
    {
      if (this == std::addressof(other))
        return *this;

      if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
      {
        container_type *ptr{std::addressof(m_container)};

        std::destroy_at  (ptr);
        std::construct_at(ptr, other.m_container.get_allocator());
      }
      else
        m_container.clear();

      m_copy(other.m_container);
    }
    else
      m_container = other.m_container;

    return *this;
  }

  constexpr
  set_sparse_container &
  operator=(set_sparse_container &&)
  = default;

  constexpr
  void
  swap(set_sparse_container &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_container, other.m_container);
  }

  friend constexpr
  void
  swap(set_sparse_container &lhs, set_sparse_container &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }


  [[nodiscard]] constexpr
  bool
  contains(identifier_type const id) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if constexpr (is_paged)
    {
      std::size_t const pg_idx{s_page_index(idx)};

      if (pg_idx >= m_container.size())
        return false;

      page_pointer const &ptr{m_container[pg_idx]};

      if (!ptr)
        return false;

      return id_traits::generation((*ptr)[s_line_index(idx)])
          == id_traits::generation(id);
    }
    else
    {
      return idx < m_container.size()
          && id_traits::generation(m_container[idx]) == id_traits::generation(id);
    }
  }

  [[nodiscard]] constexpr
  identifier_type &
  operator[](identifier_type const id)
  noexcept
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if constexpr (is_paged)
      return (*m_container[s_page_index(idx)])[s_line_index(idx)];
    else
      return m_container[idx];
  }

  [[nodiscard]] constexpr
  identifier_type
  operator[](identifier_type const id) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if constexpr (is_paged)
      return (*m_container[s_page_index(idx)])[s_line_index(idx)];
    else
      return m_container[idx];
  }

  constexpr
  void
  assure(identifier_type const id)
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if constexpr (is_paged)
    {
      std::size_t const pg_idx{s_page_index(idx)};

      if (pg_idx >= m_container.size())
        m_container.resize(pg_idx + 1);

      if (page_pointer &ptr{m_container[pg_idx]};
          !ptr)
      {
        ptr = make_unique_allocator_aware<page>(page_allocator(m_container.get_allocator()));
        ptr ->fill(id_traits::null);
      }
    }
    else
    {
      if (idx >= m_container.size())
        m_container.resize(idx + 1, id_traits::null);
    }
  }
};

} // namespace detail


template<
    typename    Id     = default_identifier_t,
    std::size_t PageSz = default_page_size_v,
    typename    Alloc  = std::allocator<Id>>
requires (std::unsigned_integral<Id> && allocator_for<Alloc, Id>)
class generic_set
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

  static constexpr std::size_t page_size
  = PageSz;

protected:
  using sparse_container = detail::set_sparse_container<Id, PageSz, Alloc>;
  using dense_container  = std::vector<Id, Alloc>;

  using id_traits
  = identifier_traits<Id>;

protected:
  sparse_container m_sparse;
  dense_container  m_dense;

protected:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<sparse_container, sparse_container &&, Alloc const &>
        && std::is_nothrow_constructible_v<dense_container , dense_container  &&, Alloc const &>;
  }

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<sparse_container>
        && std::is_nothrow_swappable_v<dense_container>;
  }

public:
  explicit constexpr
  generic_set(allocator_type const &alloc)
  noexcept
    : m_sparse{alloc}
    , m_dense {alloc}
  { }

  constexpr
  generic_set(generic_set const &other, allocator_type const &alloc)
    : m_sparse{other.m_sparse, alloc}
    , m_dense {other.m_dense , alloc}
  { }

  constexpr
  generic_set(generic_set &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_sparse{std::move(other.m_sparse), alloc}
    , m_dense {std::move(other.m_dense ), alloc}
  { }

  constexpr
  generic_set()
  noexcept(s_noexcept_default_construct())
    : generic_set{allocator_type{}}
  { }

  constexpr
  generic_set(generic_set const &)
  = default;

  constexpr
  generic_set(generic_set &&)
  = default;

  virtual constexpr
  ~generic_set()
  = default;

  constexpr
  generic_set &
  operator=(generic_set const &)
  = default;

  constexpr
  generic_set &
  operator=(generic_set &&)
  = default;

  constexpr
  void
  swap(generic_set &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_sparse, other.m_sparse);
    swap(m_dense , other.m_dense);
  }

  friend constexpr
  void
  swap(generic_set &lhs, generic_set &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_dense.get_allocator(); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_set const &lhs, generic_set const &rhs)
  noexcept
  {
    if (lhs.size() == rhs.size())
      return std::ranges::all_of(lhs, [&rhs](auto const id){ return rhs.contains(id); });

    return false;
  }


  [[nodiscard]] constexpr auto begin() noexcept       { return m_dense.rbegin(); }
  [[nodiscard]] constexpr auto begin() const noexcept { return m_dense.rbegin(); }

  [[nodiscard]] constexpr auto end() noexcept       { return m_dense.rend(); }
  [[nodiscard]] constexpr auto end() const noexcept { return m_dense.rend(); }

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end(); }

  [[nodiscard]] constexpr auto rbegin() noexcept       { return m_dense.begin(); }
  [[nodiscard]] constexpr auto rbegin() const noexcept { return m_dense.begin(); }

  [[nodiscard]] constexpr auto rend() noexcept       { return m_dense.end(); }
  [[nodiscard]] constexpr auto rend() const noexcept { return m_dense.end(); }

  [[nodiscard]] constexpr auto crbegin() const noexcept { return rbegin(); }
  [[nodiscard]] constexpr auto crend  () const noexcept { return rend(); }

  [[nodiscard]] constexpr auto size () const noexcept { return m_dense.size(); }
  [[nodiscard]] constexpr bool empty() const noexcept { return m_dense.empty(); }

  [[nodiscard]] constexpr
  bool
  contains(identifier_type const id) const
  noexcept
  { return m_sparse.contains(id); }

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id)
  noexcept
  {
    if (contains(id))
      return end() - 1 - id_traits::index(m_sparse[id]);

    return end();
  }

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id) const
  noexcept
  {
    if (contains(id))
      return end() - 1 - id_traits::index(m_sparse[id]);

    return end();
  }


  template<typename ...Args>
  constexpr
  void
  emplace(Args &&...args)
  {
    using index_type
    = typename id_traits::index_type;

    identifier_type const id{std::forward<Args>(args)...};

    m_sparse.assure      (id);
    m_dense .emplace_back(id);
    m_sparse[id] = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
  }

  template<typename ...Args>
  constexpr
  bool
  try_emplace(Args &&...args)
  { return insert(identifier_type{std::forward<Args>(args)...}); }

  constexpr
  bool
  insert(identifier_type const id)
  {
    using index_type
    = typename id_traits::index_type;

    if (contains(id))
      return false;

    m_sparse.assure      (id);
    m_dense .emplace_back(id);
    m_sparse[id] = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
    return true;
  }

  virtual constexpr
  void
  erase(identifier_type const id)
  {
    using index_type
    = typename id_traits::index_type;

    if (auto const idx{static_cast<std::size_t>(id_traits::index(m_sparse[id]))};
        idx != static_cast<std::size_t>(size() - 1))
    {
      identifier_type const back{m_dense.back()};

      m_dense [idx]  = back;
      m_sparse[back] = id_traits::from(static_cast<index_type>(idx), id_traits::generation(back));
    }

    m_dense.pop_back();
    m_sparse[id] = id_traits::null;
  }

  virtual constexpr
  bool
  try_erase(identifier_type const id)
  {
    if (!contains(id))
      return false;

    erase(id);
    return true;
  }

  virtual constexpr
  void
  clear()
  noexcept
  {
    for (identifier_type const id : m_dense)
      m_sparse[id] = id_traits::null;

    m_dense.clear();
  }
};


using set
= generic_set<>;

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SET_HPP
