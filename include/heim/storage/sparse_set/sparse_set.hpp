#ifndef HEIM_SPARSE_SET_HPP
#define HEIM_SPARSE_SET_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/allocator.hpp"
#include "heim/identifier.hpp"
#include "heim/utility.hpp"

namespace heim::sparse_set_based
{
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
public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static constexpr std::size_t page_size = PageSize;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::sparse_set: identifier_type must be a specialization of identifier_type.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::sparse_set: allocator_type must pass as an allocator of identifier_type.");


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  using dense_container
  = std::vector<identifier_type, allocator_type>;


  class sparse_container
  {
  private:
    using alloc_traits = std::allocator_traits<allocator_type>;


    static constexpr bool            s_is_paged = PageSize != 0;
    static constexpr identifier_type s_null_pos = identifier_type{};

    using page              = std::array<identifier_type, page_size>;
    using page_allocator    = alloc_traits::template rebind_alloc <page>;
    using page_alloc_traits = alloc_traits::template rebind_traits<page>;

    class page_deleter
    {
    private:
      [[no_unique_address]]
      page_allocator m_allocator;

    public:
      explicit constexpr
      page_deleter(allocator_type)
      noexcept;

      constexpr
      ~page_deleter()
      = default;


      constexpr
      void
      operator()(page *)
      noexcept;
    };

    using page_pointer              = std::unique_ptr<page, page_deleter>;
    using page_pointer_allocator    = alloc_traits::template rebind_alloc <page_pointer>;
    using page_pointer_alloc_traits = alloc_traits::template rebind_traits<page_pointer>;

    using container_type
    = std::conditional_t<
        s_is_paged,
        std::vector<page_pointer   , page_pointer_allocator>,
        std::vector<identifier_type, allocator_type        >>;

    using container_allocator
    = typename container_type::allocator_type;

  private:
    container_type m_container;

  private:
    constexpr static
    bool
    s_noexcept_default_construct()
    noexcept;

    constexpr static
    bool
    s_noexcept_move_alloc_construct()
    noexcept;

    constexpr static
    bool
    s_noexcept_swap()
    noexcept;

    template<typename ...Args>
    [[nodiscard]]
    constexpr
    page_pointer
    m_make_page_pointer(Args &&...);

    [[nodiscard]]
    constexpr
    page_pointer
    m_make_page_pointer(std::nullptr_t);

    constexpr
    void
    m_copy_container(container_type const &, container_type &);

    constexpr
    sparse_container(sparse_container const &, allocator_type const &, bool_constant<true >);

    constexpr
    sparse_container(sparse_container const &, allocator_type const &, bool_constant<false>);

    constexpr
    sparse_container(sparse_container const &, bool_constant<true >);

    constexpr
    sparse_container(sparse_container const &, bool_constant<false>);

    constexpr static
    size_type
    s_page_nb(size_type)
    noexcept;

    constexpr static
    size_type
    s_line_nb(size_type)
    noexcept;

  public:
    constexpr explicit
    sparse_container(allocator_type const &)
    noexcept;

    constexpr
    sparse_container()
    noexcept(s_noexcept_default_construct());

    constexpr
    sparse_container(sparse_container const &, allocator_type const &);

    constexpr
    sparse_container(sparse_container const &);

    constexpr
    sparse_container(sparse_container &&, allocator_type const &)
    noexcept(s_noexcept_move_alloc_construct());

    constexpr
    sparse_container(sparse_container &&)
    = default;

    constexpr
    ~sparse_container()
    = default;

    constexpr
    sparse_container &
    operator=(sparse_container const &)
    = default;

    constexpr
    sparse_container &
    operator=(sparse_container &&)
    = default;

    constexpr
    void
    swap(sparse_container &)
    noexcept(s_noexcept_swap());


    [[nodiscard]]
    constexpr
    bool
    contains(identifier_type) const
    noexcept;

    [[nodiscard]]
    constexpr
    identifier_type &
    operator[](identifier_type)
    noexcept;

    [[nodiscard]]
    constexpr
    identifier_type
    operator[](identifier_type) const
    noexcept;


    constexpr
    void
    prepare(identifier_type);

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
  using value_type      = typename dense_container::value_type;
  using reference       = typename dense_container::reference;
  using const_reference = typename dense_container::const_reference;
  using pointer         = typename dense_container::pointer;
  using const_pointer   = typename dense_container::const_pointer;

  using iterator       = typename dense_container::iterator;
  using const_iterator = typename dense_container::const_iterator;

  using reverse_iterator       = typename dense_container::reverse_iterator;
  using const_reverse_iterator = typename dense_container::const_reverse_iterator;

private:
  dense_container  m_dense;
  sparse_container m_sparse;

private:
  constexpr static
  bool
  s_noexcept_default_construct()
  noexcept;

  constexpr static
  bool
  s_noexcept_move_alloc_construct()
  noexcept;

  constexpr static
  bool
  s_noexcept_swap()
  noexcept;

protected:
  constexpr
  void
  m_emplace(identifier_type);

public:
  explicit constexpr
  sparse_set(allocator_type const &)
  noexcept;

  constexpr
  sparse_set()
  noexcept(s_noexcept_default_construct());

  constexpr
  sparse_set(sparse_set const &, allocator_type const &);

  constexpr
  sparse_set(sparse_set &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  sparse_set(sparse_set const &)
  = default;

  constexpr
  sparse_set(sparse_set &&)
  = default;

  constexpr virtual
  ~sparse_set()
  = default;

  constexpr
  sparse_set &
  operator=(sparse_set const &)
  = default;

  constexpr
  sparse_set &
  operator=(sparse_set &&)
  = default;

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
  size_type
  size() const
  noexcept;

  [[nodiscard]]
  constexpr
  bool
  empty() const
  noexcept;


  [[nodiscard]]
  constexpr
  iterator
  begin()
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  begin() const
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  cbegin() const
  noexcept;

  [[nodiscard]]
  constexpr
  iterator
  end()
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  end() const
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  cend() const
  noexcept;

  [[nodiscard]]
  constexpr
  reverse_iterator
  rbegin()
  noexcept;

  [[nodiscard]]
  constexpr
  const_reverse_iterator
  rbegin() const
  noexcept;

  [[nodiscard]]
  constexpr
  const_reverse_iterator
  crbegin() const
  noexcept;

  [[nodiscard]]
  constexpr
  reverse_iterator
  rend()
  noexcept;

  [[nodiscard]]
  constexpr
  const_reverse_iterator
  rend() const
  noexcept;

  [[nodiscard]]
  constexpr
  const_reverse_iterator
  crend() const
  noexcept;


  [[nodiscard]]
  constexpr
  bool
  contains(identifier_type) const
  noexcept;

  [[nodiscard]]
  constexpr
  iterator
  iterate(identifier_type)
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  iterate(identifier_type) const
  noexcept;

  [[nodiscard]]
  constexpr
  iterator
  find(identifier_type)
  noexcept;

  [[nodiscard]]
  constexpr
  const_iterator
  find(identifier_type) const
  noexcept;


  template<typename ...Args>
  constexpr
  void
  emplace(Args &&...);

  template<typename ...Args>
  constexpr
  bool
  try_emplace(Args &&...);

  constexpr
  bool
  insert(value_type const &);

  constexpr
  bool
  insert(value_type &&);

  template<
      typename Iterator,
      typename Sentinel>
  constexpr
  void
  insert_range(Iterator, Sentinel);

  template<typename Range>
  constexpr
  void
  insert_range(Range &&);


  constexpr
  void
  erase(identifier_type)
  noexcept;

  constexpr
  iterator
  erase(iterator)
  noexcept;

  constexpr
  iterator
  erase(const_iterator)
  noexcept;

  constexpr
  iterator
  erase(iterator, iterator)
  noexcept;

  constexpr
  iterator
  erase(const_iterator, const_iterator)
  noexcept;

  constexpr
  bool
  try_erase(identifier_type)
  noexcept;

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
    if (lhs.size() == rhs.size())
      return true;

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
    ::page_deleter(allocator_type alloc)
noexcept
  : m_allocator(alloc)
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
    ::operator()(page *p)
noexcept
{
  page_alloc_traits::destroy   (m_allocator, p);
  page_alloc_traits::deallocate(m_allocator, p, 1);
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
      container_type &&, allocator_type const &>;
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
  page          *p    (page_alloc_traits::allocate(alloc, 1));

  // strong exception safety guarantee
  try
  { page_alloc_traits::construct (alloc, p, std::forward<Args>(args)...); }
  catch (...)
  { page_alloc_traits::deallocate(alloc, p, 1); throw; }

  return page_pointer(p, page_deleter(std::move(alloc)));
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
    ::m_make_page_pointer(std::nullptr_t p)
{
  return page_pointer(p, page_deleter(page_allocator(m_container.get_allocator())));
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
  for (page_pointer &p : from)
  {
    if (p) into.emplace_back(m_make_page_pointer(*p     ));
    else   into.emplace_back(m_make_page_pointer(nullptr));
  }
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
sparse_set<Identifier, PageSize, Allocator>
    ::sparse_container
    ::sparse_container(sparse_container const &other, allocator_type const &alloc, bool_constant<true >)
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
    ::sparse_container(sparse_container const &other, bool_constant<true >)
  : m_container(
        std::allocator_traits<container_allocator>
            ::select_on_container_copy_construction(other.m_container.get_allocator()))
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
    ::s_page_nb(size_type idx)
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
    ::s_line_nb(size_type idx)
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
  : sparse_container(allocator_type())
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
    ::contains(identifier_type id) const
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
  {
    size_type const page_nb = s_page_nb(idx);
    if (page_nb >= m_container.size())
      return false;

    page const *p = m_container[page_nb].get();
    if (!p)
      return false;

    return (*p)[s_line_nb(idx)].generation() == id.generation();
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
    ::operator[](identifier_type id)
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
    return (*m_container[s_page_nb(idx)])[s_line_nb(idx)];
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
    ::operator[](identifier_type id) const
noexcept
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
    return (*m_container[s_page_nb(idx)])[s_line_nb(idx)];
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
    ::prepare(identifier_type id)
{
  size_type const idx = id.index();

  if constexpr (s_is_paged)
  {
    size_type const pg_nb = s_page_nb(idx);

    if (pg_nb >= m_container.size())
      m_container.reserve(pg_nb + 1);

    while (pg_nb >= m_container.size())
      m_container.emplace_back(m_make_page_pointer(nullptr));

    page_pointer &pg = m_container[pg_nb];
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
    ::erase(identifier_type id)
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
        std::fill(p->begin(), p->end(), s_null_pos);
    }
  }
  else
    std::fill(m_container.begin(), m_container.end(), s_null_pos);
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
          sparse_container,
          sparse_container &&, allocator_type const &>
   && std::is_nothrow_constructible_v<
          dense_container ,
          dense_container  &&, allocator_type const &>;
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
  return std::is_nothrow_swappable_v<sparse_container>
      && std::is_nothrow_swappable_v<dense_container >;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::m_emplace(identifier_type id)
{
  m_sparse.prepare(id);

  m_dense .emplace_back(id);
  m_sparse[id] = identifier_type(--m_dense.size(), id.generation());
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
    typename Allocator>
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

  swap(m_sparse, other.m_sparse);
  swap(m_dense , other.m_dense );
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
sparse_set<Identifier, PageSize, Allocator>
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
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::begin()
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
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::begin() const
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
  return m_dense.end();
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
  return m_dense.end();
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
  return m_dense.rbegin();
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
  return m_dense.rbegin();
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
  return m_dense.rend();
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
  return m_dense.rend();
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
bool
sparse_set<Identifier, PageSize, Allocator>
    ::contains(identifier_type id) const
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
    ::iterate(identifier_type id)
noexcept
{
  return begin() + m_sparse[id];
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::const_iterator
sparse_set<Identifier, PageSize, Allocator>
    ::iterate(identifier_type id) const
noexcept
{
  return begin() + m_sparse[id];
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::find(identifier_type id)
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
    ::find(identifier_type id) const
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
void
sparse_set<Identifier, PageSize, Allocator>
    ::emplace(Args &&...args)
{
  m_emplace(identifier_type(std::forward<Args>(args)...));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::try_emplace(Args &&...args)
{
  identifier_type const id(std::forward<Args>(args)...);

  if (contains(id))
    return false;

  m_emplace(id);
  return true;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::insert(value_type const &id)
{
  if (contains(id))
    return false;

  m_emplace(id);
  return true;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::insert(value_type &&id)
{
  return try_emplace(std::move(id));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<
    typename Iterator,
    typename Sentinel>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::insert_range(Iterator first, Sentinel last)
{
  static_assert(
      std::input_iterator<Iterator>
   && std::convertible_to<std::iter_reference_t<Iterator>, identifier_type>,
      "heim::sparse_set_based::sparse_set::insert_range: Iterator must be an input iterator dereferenceable "
      "to a type convertible to identifier_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "heim::sparse_set_based::sparse_set::insert_range: Sentinel must be a sentinel for Iterator.");

  for (; first != last; ++first)
    insert(*first);
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename Range>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::insert_range(Range &&r)
{
  static_assert(
      std::ranges::input_range<Range>
   && std::convertible_to<std::ranges::range_reference_t<Range>, identifier_type>,
      "heim::sparse_set_based::sparse_set::insert_range: Range must be an input range with an iterator "
      "dereferenceable to a type convertible to identifier_type.");

  insert_range(std::ranges::begin(r), std::ranges::end(r));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
sparse_set<Identifier, PageSize, Allocator>
    ::erase(identifier_type id)
noexcept
{
  if (identifier_type pos = m_sparse[id]; pos.index() != size() - 1)
  {
    m_dense [pos.index()]          = std::move(m_dense.back());
    m_sparse[m_dense[pos.index()]] = pos;
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
sparse_set<Identifier, PageSize, Allocator>
    ::erase(iterator it)
noexcept
{
  erase(*it);
  return it;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::erase(const_iterator it)
noexcept
{
  return erase(begin() + (it - cbegin()));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::erase(iterator first, iterator last)
noexcept
{
  while (first != erase(--last));
  return last;
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename sparse_set<Identifier, PageSize, Allocator>
    ::iterator
sparse_set<Identifier, PageSize, Allocator>
    ::erase(const_iterator first, const_iterator last)
noexcept
{
  return erase(begin() + (first - cbegin()), begin() + (last - cbegin()));
}

template<
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
sparse_set<Identifier, PageSize, Allocator>
    ::try_erase(identifier_type id)
noexcept
{
  if (contains(id))
    return false;

  erase(id);
  return true;
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


}

#endif // HEIM_SPARSE_SET_HPP