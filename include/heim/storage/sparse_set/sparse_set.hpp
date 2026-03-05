#ifndef HEIM_SPARSE_SET_HPP
#define HEIM_SPARSE_SET_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/allocator.hpp"
#include "heim/identifier.hpp"
#include "heim/utility.hpp"

namespace heim::sparse_set_based
{
/*!
 * @brief The base associative container optimized for its usage in the context of the entity-component-system
 *   pattern.
 *
 * @details Uses two containers — one contiguous "dense" container to hold the elements, and another
 *   "sparse" container to keep track of the position of each element.
 *   By default, this sparse array is paginated to avoid significant memory overhead in most use
 *   cases. This structure allows for constant-time insertion, removal, access to elements, as well
 *   as providing optimal iteration speed.
 *
 * @tparam Identifier The identifier type.
 * @tparam PageSize   The size of each internal page of positions.
 * @tparam Allocator  The allocator type.
 *
 * @note Specializing this container with a page size of zero causes the sparse container to not be
 *   paginated at all. This can be considered a good option if inserted identifiers are expected to
 *   have low enough index values.
 */
template<
    typename    Identifier = heim::identifier<>,
    std::size_t PageSize   = 1024,
    typename    Allocator  = std::allocator<Identifier>>
class sparse_set;

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
class sparse_set
{
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::sparse_set: identifier_type must be a specialization of heim::identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::sparse_set: allocator_type must pass as an allocator for identifier_type.");

  constexpr static
  std::size_t
  page_size
  = PageSize;


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using value_type      = identifier_type;
  using reference       = identifier_type &;
  using const_reference = identifier_type const &;
  using pointer         = identifier_type *;
  using const_pointer   = identifier_type const *;

private:
  using dense_container
  = std::vector<identifier_type, allocator_type>;


  class sparse_container
  {
  private:
    constexpr static bool            s_is_paged = page_size != 0;
    constexpr static identifier_type s_null_pos = identifier_type{};

    using alloc_traits
    = std::allocator_traits<allocator_type>;


    using page
    = std::array<identifier_type, page_size>;

    using page_allocator    = alloc_traits::template rebind_alloc <page>;
    using page_alloc_traits = alloc_traits::template rebind_traits<page>;

  private:
    class page_deleter
    {
    private:
      [[no_unique_address]]
      page_allocator
      m_allocator;

    public:
      explicit constexpr
      page_deleter(allocator_type &&)
      noexcept;

      constexpr
      ~page_deleter()
      = default;


      constexpr
      void
      operator()(page *)
      noexcept;
    };

  private:
    using page_pointer
    = std::unique_ptr<page, page_deleter>;

    using page_pointer_allocator    = alloc_traits::template rebind_alloc <page_pointer>;
    using page_pointer_alloc_traits = alloc_traits::template rebind_traits<page_pointer>;


    using container_type
    = std::conditional_t<
        s_is_paged,
        std::vector<page_pointer   , page_pointer_allocator>,
        std::vector<identifier_type, allocator_type        >>;

    using container_allocator    = typename container_type::allocator_type;
    using container_alloc_traits = std::allocator_traits<container_allocator>;

  private:
    container_type
    m_container;

  private:
    constexpr static bool s_noexcept_default_construct   () noexcept;
    constexpr static bool s_noexcept_move_alloc_construct() noexcept;
    constexpr static bool s_noexcept_swap() noexcept;


    template<typename ...Args>
    [[nodiscard]]
    constexpr
    page_pointer
    m_make_page_pointer(Args &&...args);

    [[nodiscard]]
    constexpr
    page_pointer
    m_make_page_pointer(std::nullptr_t);

    constexpr
    void
    m_copy_container(container_type const &from, container_type &into);


    constexpr sparse_container(sparse_container const &, allocator_type const &, bool_constant<true >);
    constexpr sparse_container(sparse_container const &, allocator_type const &, bool_constant<false>);

    constexpr sparse_container(sparse_container const &, bool_constant<true >);
    constexpr sparse_container(sparse_container const &, bool_constant<false>);


    constexpr static size_type s_page_index(size_type) noexcept;
    constexpr static size_type s_line_index(size_type) noexcept;

  public:
    constexpr explicit
    sparse_container(allocator_type const &)
    noexcept;

    constexpr
    sparse_container()
    noexcept(s_noexcept_default_construct());

    constexpr sparse_container(sparse_container const &, allocator_type const &);
    constexpr sparse_container(sparse_container const &);

    constexpr
    sparse_container(sparse_container &&, allocator_type const &)
    noexcept(s_noexcept_move_alloc_construct());

    constexpr
    sparse_container(sparse_container &&)
    = default;

    constexpr
    ~sparse_container()
    = default;

    constexpr sparse_container &operator=(sparse_container const &);
    constexpr sparse_container &operator=(sparse_container &&     ) = default;

    constexpr
    void
    swap(sparse_container &)
    noexcept(s_noexcept_swap());


    [[nodiscard]]
    constexpr
    bool
    contains(identifier_type) const
    noexcept;

    [[nodiscard]] constexpr identifier_type &operator[](identifier_type)       noexcept;
    [[nodiscard]] constexpr identifier_type  operator[](identifier_type) const noexcept;


    constexpr
    void
    prepare_for(identifier_type);

    constexpr
    void
    erase(identifier_type)
    noexcept;

    constexpr
    void
    clear()
    noexcept;


    friend constexpr
    void
    swap(sparse_container &lhs, sparse_container &rhs)
    noexcept(s_noexcept_swap())
    {
      lhs.swap(rhs);
    }
  };

public:
  using iterator       = typename dense_container::const_reverse_iterator;
  using const_iterator = typename dense_container::const_reverse_iterator;

  using reverse_iterator       = typename dense_container::const_iterator;
  using const_reverse_iterator = typename dense_container::const_iterator;

  using container_type
  = dense_container;

private:
  dense_container  m_dense;
  sparse_container m_sparse;

private:
  constexpr static bool s_noexcept_default_construct   () noexcept;
  constexpr static bool s_noexcept_move_alloc_construct() noexcept;
  constexpr static bool s_noexcept_swap() noexcept;


  constexpr
  void
  m_emplace(identifier_type);

public:
  constexpr explicit
  sparse_set(allocator_type const &)
  noexcept;

  constexpr
  sparse_set()
  noexcept(s_noexcept_default_construct());

  constexpr
  sparse_set(sparse_set const &, allocator_type const &);

  constexpr
  sparse_set(sparse_set const &)
  = default;

  constexpr
  sparse_set(sparse_set &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  sparse_set(sparse_set &&)
  = default;

  constexpr virtual
  ~sparse_set()
  = default;

  constexpr sparse_set &operator=(sparse_set const &) = default;
  constexpr sparse_set &operator=(sparse_set &&     ) = default;

  [[nodiscard]]
  constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(sparse_set &)
  noexcept(s_noexcept_swap());

  [[nodiscard]]
  constexpr
  container_type const &
  container() const
  noexcept;


  [[nodiscard]] constexpr iterator       begin ()       noexcept;
  [[nodiscard]] constexpr const_iterator begin () const noexcept;
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

  [[nodiscard]] constexpr iterator       end ()       noexcept;
  [[nodiscard]] constexpr const_iterator end () const noexcept;
  [[nodiscard]] constexpr const_iterator cend() const noexcept;

  [[nodiscard]] constexpr reverse_iterator       rbegin ()       noexcept;
  [[nodiscard]] constexpr const_reverse_iterator rbegin () const noexcept;
  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept;

  [[nodiscard]] constexpr reverse_iterator       rend ()       noexcept;
  [[nodiscard]] constexpr const_reverse_iterator rend () const noexcept;
  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept;

  [[nodiscard]] constexpr size_type size () const noexcept;
  [[nodiscard]] constexpr bool      empty() const noexcept;


  [[nodiscard]]
  constexpr
  bool
  contains(identifier_type) const
  noexcept;

  [[nodiscard]] constexpr iterator       iterate(identifier_type)       noexcept;
  [[nodiscard]] constexpr const_iterator iterate(identifier_type) const noexcept;

  [[nodiscard]] constexpr iterator       find(identifier_type)       noexcept;
  [[nodiscard]] constexpr const_iterator find(identifier_type) const noexcept;


  template<typename ...Args> constexpr std::pair<iterator, bool> emplace    (Args &&...);
  template<typename ...Args> constexpr std::pair<iterator, bool> try_emplace(Args &&...);

  constexpr std::pair<iterator, bool> insert(identifier_type const &);
  constexpr std::pair<iterator, bool> insert(identifier_type &&     );

  constexpr void     erase(identifier_type   ) noexcept;
  constexpr iterator erase(iterator          ) noexcept;
  constexpr iterator erase(iterator, iterator) noexcept;

  constexpr bool     try_erase(identifier_type   ) noexcept;
  constexpr iterator try_erase(iterator          ) noexcept;
  constexpr iterator try_erase(iterator, iterator) noexcept;

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(sparse_set &lhs, sparse_set &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]]
  friend constexpr
  bool
  operator==(sparse_set const &lhs, sparse_set const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto const id : lhs)
    {
      if (!rhs.contains(id))
        return false;
    }

    return true;
  }
};


template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::page_deleter
    ::page_deleter(allocator_type &&alloc)
noexcept
  : m_allocator(std::move(alloc))
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::page_deleter
    ::operator()(page *pg)
noexcept
{
  page_alloc_traits::destroy   (m_allocator, pg);
  page_alloc_traits::deallocate(m_allocator, pg, 1);
}


template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<
      container_type,
      container_type &&, container_allocator const &>;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<container_type>;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::page_pointer
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::m_make_page_pointer(Args &&...args)
{
  page_allocator alloc(m_container.get_allocator());
  page          *pg   (page_alloc_traits::allocate(alloc, 1));

  // strong exception safety guarantee
  try
  { page_alloc_traits::construct (alloc, pg, std::forward<Args>(args)...); }
  catch (...)
  { page_alloc_traits::deallocate(alloc, pg, 1); throw; }

  return page_pointer(pg, page_deleter(std::move(alloc)));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::page_pointer
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::m_make_page_pointer(std::nullptr_t np)
{
  return page_pointer(np, page_deleter(page_allocator(m_container.get_allocator())));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::m_copy_container(container_type const &from, container_type &into)
{
  into.reserve(from.capacity());

  for (page_pointer const &pg : from)
  {
    if (pg) into.emplace_back(m_make_page_pointer(*pg    ));
    else    into.emplace_back(m_make_page_pointer(nullptr));
  }
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, allocator_type const &alloc, bool_constant<true>)
  : m_container(alloc)
{
  m_copy_container(other.m_container, m_container);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, allocator_type const &alloc, bool_constant<false>)
  : m_container(other.m_container, alloc)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, bool_constant<true>)
  : m_container(container_alloc_traits::select_on_container_copy_construction(other.m_container.get_allocator()))
{
  m_copy_container(other.m_container, m_container);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, bool_constant<false>)
  : m_container(other.m_container)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::size_type
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::s_page_index(size_type idx)
noexcept
{
  return idx / page_size;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::size_type
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::s_line_index(size_type idx)
noexcept
{
  return idx % page_size;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(allocator_type const &alloc)
noexcept
  : m_container(container_allocator(alloc))
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container()
noexcept(s_noexcept_default_construct())
  : sparse_container(allocator_type{})
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, allocator_type const &alloc)
  : sparse_container(other, alloc, bool_constant<s_is_paged>{})
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other)
  : sparse_container(other, bool_constant<s_is_paged>{})
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_container(std::move(other.m_container), alloc)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container &
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::operator=(sparse_container const &other)
{
  if constexpr (s_is_paged)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
    {
      std::destroy_at  (&m_container);
      std::construct_at(&m_container, other.m_container.get_allocator());
    }
    else
      m_container.clear();

    m_copy_container(other.m_container, m_container);
  }
  else
    m_container = other.m_container;

  return *this;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::swap(sparse_container &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  swap(m_container, other.m_container);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::contains(identifier_type const id) const
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
  {
    size_type const page_idx = s_page_index(idx);
    if (page_idx >= m_container.size())
      return false;

    page const *pg = m_container[page_idx].get();
    if (!pg)
      return false;

    return (*pg)[s_line_index(idx)].generation() == id.generation();
  }
  else
  {
    return idx < m_container.size()
        && m_container[idx].generation() == id.generation();
  }
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::identifier_type &
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::operator[](identifier_type const id)
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
    return (*m_container[s_page_index(idx)])[s_line_index(idx)];
  else
    return m_container[idx];
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::identifier_type
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::operator[](identifier_type const id) const
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
    return (*m_container[s_page_index(idx)])[s_line_index(idx)];
  else
    return m_container[idx];
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::prepare_for(identifier_type const id)
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
  {
    size_type const pg_idx = s_page_index(idx);

    if (pg_idx >= m_container.size())
      m_container.resize(pg_idx + 1, m_make_page_pointer(nullptr));

    page_pointer &pg = m_container[pg_idx];
    if (!pg)
    {
      pg = m_make_page_pointer();
      pg ->fill(s_null_pos);
    }
  }
  else
  {
    if (idx >= m_container.size())
      m_container.resize(idx + 1, s_null_pos);
  }
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::erase(identifier_type const id)
noexcept
{
  operator[](id) = s_null_pos;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::clear()
noexcept
{
  if constexpr (s_is_paged)
  {
    for (auto const &pg : m_container)
    {
      if (auto * const p = pg.get())
        std::ranges::fill(*p, s_null_pos);
    }
  }
  else
    std::ranges::fill(m_container, s_null_pos);
}


template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::iterator(underlying_iterator const it)
noexcept
  : m_it(it)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator &
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator++()
noexcept
{
  --m_it;
  return *this;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator++(int)
noexcept
{
  iterator r(*this);
  ++*this;
  return r;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator &
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator--()
noexcept
{
  ++m_it;
  return *this;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator--(int)
noexcept
{
  iterator r(*this);
  --*this;
  return r;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator &
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator+=(difference_type const n)
noexcept
{
  m_it -= n;
  return *this;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator &
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator-=(difference_type const n)
noexcept
{
  m_it += n;
  return *this;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::reference
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator*() const
noexcept
{
  return *m_it;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::pointer
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator->() const
noexcept
{
  return m_it.operator->();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::reference
sparse_set<Identifier, PageSize, Allocator>
    ::iterator
    ::operator[](difference_type const n) const
noexcept
{
  return m_it[n];
}


template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::s_noexcept_move_alloc_construct()
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

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<dense_container >
      && std::is_nothrow_swappable_v<sparse_container>;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::m_emplace(identifier_type const id)
{
  m_sparse.prepare_for (id);
  m_dense .emplace_back(id);
  m_sparse[id] = identifier_type(m_dense.size() - 1, id.generation());
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_set(allocator_type const &alloc)
noexcept
  : m_dense (alloc),
    m_sparse(alloc)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_set()
noexcept(s_noexcept_default_construct())
  : sparse_set(allocator_type{})
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_set(sparse_set const &other, allocator_type const &alloc)
  : m_dense (other.m_dense , alloc),
    m_sparse(other.m_sparse, alloc)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_set(sparse_set &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_dense (std::move(other.m_dense ), alloc),
    m_sparse(std::move(other.m_sparse), alloc)
{ }

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::allocator_type
sparse_set<Identifier, PageSize, Allocator>
    ::get_allocator() const
noexcept
{
  return m_dense.get_allocator();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::swap(sparse_set &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  swap(m_dense , other.m_dense );
  swap(m_sparse, other.m_sparse);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::container_type const &
sparse_set<Identifier, PageSize, Allocator>
    ::container() const
noexcept
{
  return m_dense;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::begin()
noexcept
{
  return m_dense.rbegin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::begin() const
noexcept
{
  return m_dense.rbegin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::cbegin() const
noexcept
{
  return begin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::end()
noexcept
{
  return m_dense.rend();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::end() const
noexcept
{
  return m_dense.rend();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::cend() const
noexcept
{
  return end();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::rbegin()
noexcept
{
  return m_dense.begin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::rbegin() const
noexcept
{
  return m_dense.begin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::crbegin() const
noexcept
{
  return rbegin();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::rend()
noexcept
{
  return m_dense.end();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::rend() const
noexcept
{
  return m_dense.end();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_reverse_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::crend() const
noexcept
{
  return rend();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::size_type
sparse_set<Identifier, PageSize, Allocator>
    ::size() const
noexcept
{
  return m_dense.size();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier,PageSize, Allocator>
    ::empty() const
noexcept
{
  return m_dense.empty();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::contains(identifier_type const id) const
noexcept
{
  return m_sparse.contains(id);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id)
noexcept
{
  return rend() - m_sparse[id].index() - 1;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id) const
noexcept
{
  return rend() - m_sparse[id].index() - 1;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::find(identifier_type const id)
noexcept
{
  return contains(id) ? iterate(id) : end();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::find(identifier_type const id) const
noexcept
{
  return contains(id) ? iterate(id) : end();
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<
    typename sparse_set<Identifier, PageSize, Allocator>
        ::iterator,
    bool>
sparse_set<Identifier, PageSize, Allocator>
    ::emplace(Args &&...args)
{
  m_emplace(identifier_type(std::forward<Args>(args)...));
  return std::pair(begin(), true);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<
    typename sparse_set<Identifier, PageSize, Allocator>
        ::iterator,
    bool>
sparse_set<Identifier, PageSize, Allocator>
    ::try_emplace(Args &&...args)
{
  identifier_type id(std::forward<Args>(args)...);

  if (contains(id))
    return std::pair(iterate(id), false);

  m_emplace(id);
  return std::pair(begin(), true);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<
    typename sparse_set<Identifier, PageSize, Allocator>
        ::iterator,
    bool>
sparse_set<Identifier, PageSize, Allocator>
    ::insert(identifier_type const &id)
{
  if (contains(id))
    return std::pair(iterate(id), false);

  m_emplace(id);
  return std::pair(begin(), true);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<
    typename sparse_set<Identifier, PageSize, Allocator>
        ::iterator,
    bool>
sparse_set<Identifier, PageSize, Allocator>
    ::insert(identifier_type &&id)
{
  if (contains(id))
    return std::pair(iterate(id), false);

  m_emplace(std::move(id));
  return std::pair(begin(), true);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::erase(identifier_type const id)
noexcept
{
  identifier_type const pos = m_sparse[id];
  auto            const idx = static_cast<size_type>(pos.index());

  if (pos.index() != m_dense.size() - 1)
  {
    m_dense [idx]          = std::move(m_dense.back());
    m_sparse[m_dense[idx]] = pos;
  }

  m_dense .pop_back();
  m_sparse.erase(id);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier,PageSize, Allocator>
    ::erase(iterator it)
noexcept
{
  erase(*it);
  return it + 1;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier,PageSize, Allocator>
    ::erase(iterator first, iterator last)
noexcept
{
  while (first != last)
    first = erase(first);

  return first;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::try_erase(identifier_type const id)
noexcept
{
  if (!contains(id))
    return false;

  erase(id);
  return true;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::try_erase(iterator it)
noexcept
{
  try_erase(*it);
  return it + 1;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::try_erase(iterator first, iterator last)
noexcept
{
  while (first != last)
    first = try_erase(first);

  return first;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::clear()
noexcept
{
  m_dense .clear();
  m_sparse.clear();
}


/*!
 * @brief Determines whether the given type is a specialization of sparse_set.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_sparse_set;

template<typename T>
struct specializes_sparse_set
  : bool_constant<false>
{ };

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
struct specializes_sparse_set<sparse_set<Identifier, PageSize, Allocator>>
  : bool_constant<true>
{ };


}

#endif // HEIM_SPARSE_SET_HPP