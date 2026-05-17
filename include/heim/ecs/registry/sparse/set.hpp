#ifndef HEIM_ECS_REGISTRY_SPARSE_SET_HPP
#define HEIM_ECS_REGISTRY_SPARSE_SET_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/identifier.hpp"
#include "heim/lib/unique_allocator_aware_ptr.hpp"

namespace heim::sparse
{
/*!
 * \brief
 *   Determines the default size for pages for the sparse container of sets and pools.
 *
 * \details
 *   Each set and pool implements the sparse set data structure, and uses pagination in the sparse container
 *   to avoid significant memory overhead.
 *
 * \note
 *   Using a value attribute of zero (0) will cause the containers to not be paginated. This can be
 *   an option to very slightly improve performance when used identifier values are low.
 */
template<typename = void>
struct default_page_size
  : std::integral_constant<std::size_t, 1024>
{ };

template<typename = void>
inline constexpr
std::size_t
default_page_size_v
= default_page_size<>::value;


namespace detail
{
template<
    typename    Identifier = default_identifier_t<>,
    std::size_t PageSize   = default_page_size_v<>,
    typename    Allocator  = std::allocator<Identifier>>
requires (
    identifier   <Identifier>
 && allocator_for<Allocator, Identifier>)
class set_sparse_container
{
public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static constexpr std::size_t page_size = PageSize;
  static constexpr bool        is_paged  = page_size != 0;

private:
  using id_traits    = identifier_traits<identifier_type>;
  using alloc_traits = std::allocator_traits<allocator_type>;

  using page
  = std::array<identifier_type, page_size>;

  using page_allocator    = alloc_traits::template rebind_alloc <page>;
  using page_alloc_traits = alloc_traits::template rebind_traits<page>;

  using page_pointer
  = unique_allocator_aware_ptr<page, page_allocator>;

  using page_pointer_allocator    = alloc_traits::template rebind_alloc <page_pointer>;
  using page_pointer_alloc_traits = alloc_traits::template rebind_traits<page_pointer>;

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
        container_type &&, allocator_type const &>;
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
  m_copy(container_type const &container)
  {
    m_container.reserve(container.capacity());

    for (page_pointer const &ptr : container)
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
    : m_container{std::move(other.m_container), alloc}
  { }

  constexpr
  set_sparse_container(set_sparse_container &&other)
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
  { m_container.swap(other.m_container); }

  friend constexpr
  void
  swap(set_sparse_container &lhs, set_sparse_container &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  constexpr
  void
  swap(identifier_type const lhs, identifier_type const rhs)
  noexcept
  { using std::swap; swap(position(lhs), position(rhs)); }

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
  position(identifier_type const id)
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
  position(identifier_type const id) const
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
  reserve_for(identifier_type const id)
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if constexpr (is_paged)
    {
      std::size_t const pg_idx{s_page_index(idx)};

      if (pg_idx >= m_container.size())
        m_container.reserve(pg_idx + 1);

      while (pg_idx >= m_container.size())
        m_container.emplace_back(page_pointer{});

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


template<
    typename Identifier = default_identifier_t<>,
    typename Allocator  = std::allocator<Identifier>>
requires (
    identifier   <Identifier>
 && allocator_for<Allocator, Identifier>)
class set_dense_container
{
public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

private:
  using container_type
  = std::vector<identifier_type, allocator_type>;

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
        container_type &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_type>; }

public:
  explicit constexpr
  set_dense_container(allocator_type const &alloc)
  noexcept
    : m_container{alloc}
  { }

  constexpr
  set_dense_container(set_dense_container const &other, allocator_type const &alloc)
    : m_container{other.m_container, alloc}
  { }

  constexpr
  set_dense_container(set_dense_container const &)
  = default;

  constexpr
  set_dense_container(set_dense_container &&other, allocator_type const &alloc)
    : m_container{std::move(other.m_container), alloc}
  { }

  constexpr
  set_dense_container(set_dense_container &&)
  = default;

  constexpr
  ~set_dense_container()
  = default;

  constexpr
  set_dense_container &
  operator=(set_dense_container const &)
  = default;

  constexpr
  set_dense_container &
  operator=(set_dense_container &&)
  = default;

  constexpr
  void
  swap(set_dense_container &other)
  noexcept(s_noexcept_swap())
  { m_container.swap(other.m_container); }

  friend constexpr
  void
  swap(set_dense_container &lhs, set_dense_container &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  constexpr
  void
  swap(std::size_t const lhs, std::size_t const rhs)
  noexcept
  { using std::swap; swap(m_container[lhs], m_container[rhs]); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_container.get_allocator(); }


  [[nodiscard]] constexpr
  auto
  begin()
  noexcept
  { return m_container.rbegin(); }

  [[nodiscard]] constexpr
  auto
  begin() const
  noexcept
  { return m_container.rbegin(); }

  [[nodiscard]] constexpr
  auto
  end()
  noexcept
  { return m_container.rend(); }

  [[nodiscard]] constexpr
  auto
  end() const
  noexcept
  { return m_container.rend(); }

  [[nodiscard]] constexpr
  auto
  cbegin() const
  noexcept
  { return m_container.crbegin(); }

  [[nodiscard]] constexpr
  auto
  cend() const
  noexcept
  { return m_container.crend(); }

  [[nodiscard]] constexpr
  std::size_t
  size() const
  noexcept
  { return m_container.size(); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return m_container.empty(); }

  [[nodiscard]] constexpr
  identifier_type
  back() const
  noexcept
  { return m_container.back(); }


  constexpr
  void
  insert(identifier_type const id)
  { m_container.emplace_back(id); }

  constexpr
  void
  overwrite_with_back(std::size_t const idx)
  noexcept
  { m_container[idx] = std::move(m_container.back()); }

  constexpr
  void
  pop_back()
  noexcept
  { m_container.pop_back(); }

  constexpr
  void
  clear()
  noexcept
  { m_container.clear(); }
};

} // namespace detail


/*!
 * \brief
 *   The main underlying container for identifiers.
 *
 * \details
 *   Implements a specialized sparse set data structure, that uses pagination to avoid significant memory
 *   overhead.
 *   This data structure allows for constant-time complexity insertion, removal and access to elements,
 *   as well as providing optimal iteration speed.
 *
 * \note
 *   Using a specializing page size of zero (0) will cause the container to not use pagination. This
 *   can be an option to very slightly improve performance when used identifier values are low.
 */
template<
    typename    Identifier = default_identifier_t<>,
    std::size_t PageSize   = default_page_size_v<>,
    typename    Allocator  = std::allocator<Identifier>>
class set
  : protected detail::set_dense_container <Identifier, Allocator>
  , protected detail::set_sparse_container<Identifier, PageSize, Allocator>
{
protected:
  using dense_container  = detail::set_dense_container <Identifier, Allocator>;
  using sparse_container = detail::set_sparse_container<Identifier, PageSize, Allocator>;

public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      is_identifier_v<identifier_type>,
      "heim::sparse::set: identifier_type must be an identifier type.");

  static constexpr std::size_t page_size
  = PageSize;

private:
  using id_traits
  = identifier_traits<identifier_type>;

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
            dense_container,
            dense_container  &&, allocator_type const &>
     && std::is_nothrow_constructible_v<
            sparse_container,
            sparse_container &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<dense_container>
        && std::is_nothrow_swappable_v<sparse_container>;
  }

public:
  explicit constexpr
  set(allocator_type const &alloc)
  noexcept
    : dense_container{alloc}, sparse_container{alloc}
  { }

  constexpr
  set()
  noexcept(s_noexcept_default_construct())
    : set{allocator_type{}}
  { }

  constexpr
  set(set const &other, allocator_type const &alloc)
    : dense_container {static_cast<dense_container  const &>(other), alloc}
    , sparse_container{static_cast<sparse_container const &>(other), alloc}
  { }

  constexpr
  set(set const &)
  = default;

  constexpr
  set(set &&other, allocator_type const &alloc)
    : dense_container {static_cast<dense_container  &&>(other), alloc}
    , sparse_container{static_cast<sparse_container &&>(other), alloc}
  { }

  constexpr
  set(set &&)
  = default;

  virtual constexpr
  ~set()
  = default;

  constexpr
  set &
  operator=(set const &)
  = default;

  constexpr
  set &
  operator=(set &&)
  = default;

  constexpr
  void
  swap(set &other)
  noexcept(s_noexcept_swap())
  {
    dense_container ::swap(static_cast<dense_container  &>(other));
    sparse_container::swap(static_cast<sparse_container &>(other));
  }

  friend constexpr
  void
  swap(set &lhs, set &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  constexpr
  void
  swap(identifier_type const lhs, identifier_type const rhs)
  noexcept
  {
    using std::swap;

    auto const lhs_idx{static_cast<std::size_t>(id_traits::index(sparse_container::position(lhs)))};
    auto const rhs_idx{static_cast<std::size_t>(id_traits::index(sparse_container::position(rhs)))};

    dense_container ::swap(lhs_idx, rhs_idx);
    sparse_container::swap(lhs    , rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(set const &lhs, set const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (identifier_type const id : lhs)
    {
      if (!rhs.contains(id))
        return false;
    }
    return true;
  }

  using dense_container::get_allocator;
  using dense_container::begin;
  using dense_container::end;
  using dense_container::cbegin;
  using dense_container::cend;
  using dense_container::size;
  using dense_container::empty;

  using sparse_container
      ::contains;

  [[nodiscard]] constexpr
  auto
  iterator_to(identifier_type const id)
  noexcept
  {
    auto const idx{static_cast<std::ptrdiff_t>(id_traits::index(sparse_container::position(id)) + 1)};
    return end() - idx;
  }

  [[nodiscard]] constexpr
  auto
  iterator_to(identifier_type const id) const
  noexcept
  {
    auto const idx{static_cast<std::ptrdiff_t>(id_traits::index(sparse_container::position(id)) + 1)};
    return end() - idx;
  }

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id)
  noexcept
  { return contains(id) ? iterator_to(id) : end(); }

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id) const
  noexcept
  { return contains(id) ? iterator_to(id) : end(); }


  template<typename ...Args>
  requires std::constructible_from<identifier_type, Args &&...>
  constexpr
  void
  emplace(Args&&... args)
  {
    using index_type
    = typename id_traits::index_type;

    identifier_type const id{std::forward<Args>(args)...};

    sparse_container::reserve_for(id);
    dense_container ::insert     (id);
    sparse_container::position   (id) = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
  }

  template<typename ...Args>
  requires std::constructible_from<identifier_type, Args &&...>
  constexpr
  bool
  try_emplace(Args&&... args)
  { return insert(identifier_type(std::forward<Args>(args)...)); }

  constexpr
  bool
  insert(identifier_type const id)
  {
    using index_type
    = typename id_traits::index_type;

    if (contains(id))
      return false;

    sparse_container::reserve_for(id);
    dense_container ::insert     (id);
    sparse_container::position   (id) = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
    return true;
  }

  virtual constexpr
  void
  erase(identifier_type const id)
  // no noexcept, as inheriting pools cannot provide the same guarantee
  {
    using index_type
    = typename id_traits::index_type;

    if (auto const idx{static_cast<std::size_t>(id_traits::index(sparse_container::position(id)))};
        idx != size() - 1)
    {
      identifier_type const back{dense_container::back()};

      dense_container ::overwrite_with_back(idx);
      sparse_container::position(back) = id_traits::from(static_cast<index_type>(idx), id_traits::generation(back));
    }

    dense_container ::pop_back();
    sparse_container::position(id) = id_traits::null;
  }

  virtual constexpr
  bool
  try_erase(identifier_type const id)
  // no noexcept, as inheriting pools cannot provide the same guarantee
  {
    if (!contains(id))
      return false;

    erase(id);
    return true;
  }

  constexpr
  void
  clear()
  noexcept
  {
    for (identifier_type const id : *this)
      sparse_container::position(id) = id_traits::null;

    dense_container::clear();
  }
};

} // namespace heim::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SET_HPP
