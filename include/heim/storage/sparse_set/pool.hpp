#ifndef HEIM_POOL_HPP
#define HEIM_POOL_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/allocator.hpp"
#include "heim/identifier.hpp"
#include "heim/utility.hpp"
#include "sparse_set.hpp"

namespace heim::sparse_set_based
{
/*!
 * @brief An associative container optimized for usage in the context of the entity-component-system
 *   pattern.
 *
 * @details Derives from the heim::sparse_set_based::sparse_set container to provide constant-time insertion,
 *   removal, access to elements, as well as providing optimal iteration speed.
 *   Also, because of the structure-of-array (SoA) nature of the container, iterators expose a pair
 *   of references rather than a reference to a pair.
 *
 * @tparam Component  The component type.
 * @tparam Identifier The identifier type.
 * @tparam PageSize   The size of each internal page of positions.
 * @tparam Allocator  The allocator type.
 *
 * @note Specializing this container with a page size of zero causes the sparse container to not be
 *   paginated at all. This can be considered a good option if inserted identifiers are expected to
 *   have low enough index values.
 */
template<
    typename    Component,
    typename    Identifier = heim::identifier<>,
    std::size_t PageSize   = 1024,
    typename    Allocator  = std::allocator<Identifier>>
class pool;

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
class pool
final
  : public sparse_set<Identifier, PageSize, Allocator>
{
public:
  using component_type  = Component;
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::pool: identifier_type must be a specialization of heim::identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::pool: allocator_type must pass as an allocator for identifier_type.");

  constexpr static
  std::size_t
  page_size
  = PageSize;

private:
  using base_type
  = sparse_set<Identifier, PageSize, Allocator>;

  using alloc_traits        = std::allocator_traits<allocator_type>;
  using component_allocator = typename alloc_traits::template rebind_alloc<component_type>;

public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using value_type      = std::pair<identifier_type        , component_type        >;
  using reference       = std::pair<identifier_type const &, component_type &      >;
  using const_reference = std::pair<identifier_type const &, component_type const &>;
  using pointer         = std::pair<identifier_type const *, component_type *      >;
  using const_pointer   = std::pair<identifier_type const *, component_type const *>;

  using identifier_container_type = typename base_type::container_type;
  using component_container_type  = std::vector<component_type, component_allocator>;

private:
  template<bool IsConst>
  class generic_iterator
  {
  public:
    friend pool;

  public:
    constexpr static
    bool
    is_const
    = IsConst;


    using difference_type
    = std::ptrdiff_t;

    using value_type = component_type;
    using reference  = std::conditional_t<is_const, pool::const_reference, pool::reference>;

    class pointer
    {
    private:
      reference
      m_ref;

    public:
      constexpr explicit
      pointer(reference &&)
      noexcept;

      constexpr
      ~pointer()
      = default;


      constexpr
      reference *
      operator->() const
      noexcept;
    };

    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::random_access_iterator_tag;

  private:
    using identifier_iterator
    = typename base_type::const_iterator;

    using component_iterator
    = std::conditional_t<
        is_const,
        typename component_container_type::const_reverse_iterator,
        typename component_container_type::reverse_iterator      >;

  private:
    identifier_iterator m_identifier_it;
    component_iterator  m_component_it;

  private:
    constexpr explicit
    generic_iterator(identifier_iterator, component_iterator)
    noexcept;

  public:
    constexpr generic_iterator()                         = default;
    constexpr generic_iterator(generic_iterator const &) = default;
    constexpr generic_iterator(generic_iterator &&     ) = default;

    constexpr
    ~generic_iterator()
    = default;

    constexpr generic_iterator &operator=(generic_iterator const &) = default;
    constexpr generic_iterator &operator=(generic_iterator &&     ) = default;


    constexpr generic_iterator &operator++()    noexcept;
    constexpr generic_iterator  operator++(int) noexcept;

    constexpr generic_iterator &operator--()    noexcept;
    constexpr generic_iterator  operator--(int) noexcept;

    constexpr generic_iterator &operator+=(difference_type) noexcept;
    constexpr generic_iterator &operator-=(difference_type) noexcept;


    [[nodiscard]]
    constexpr
    reference
    operator*() const
    noexcept;

    constexpr
    pointer
    operator->() const
    noexcept;

    [[nodiscard]]
    constexpr
    reference
    operator[](difference_type) const
    noexcept;


    friend constexpr
    generic_iterator
    operator+(generic_iterator const it, difference_type const n)
    noexcept
    {
      return generic_iterator(it.m_identifier_it + n, it.m_component_it + n);
    }

    friend constexpr
    generic_iterator
    operator+(difference_type const n, generic_iterator const it)
    noexcept
    {
      return generic_iterator(n + it.m_identifier_it, n + it.m_component_it);
    }

    friend constexpr
    generic_iterator
    operator-(generic_iterator const it, difference_type const n)
    noexcept
    {
      return generic_iterator(it.m_identifier_it - n, it.m_component_it - n);
    }

    friend constexpr
    difference_type
    operator-(generic_iterator const lhs, generic_iterator const rhs)
    noexcept
    {
      return lhs.m_identifier_it - rhs.m_identifier_it;
    }

    [[nodiscard]]
    friend constexpr
    bool
    operator==(generic_iterator, generic_iterator)
    = default;

    [[nodiscard]]
    friend constexpr
    auto
    operator<=>(generic_iterator const lhs, generic_iterator const rhs)
    noexcept
    {
      return lhs.m_identifier_it <=> rhs.m_identifier_it;
    }
  };

public:
  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  component_container_type
  m_components;

private:
  constexpr static bool s_noexcept_default_construct   () noexcept;
  constexpr static bool s_noexcept_move_alloc_construct() noexcept;
  constexpr static bool s_noexcept_swap        () noexcept;
  constexpr static bool s_noexcept_swap_entries() noexcept;
  constexpr static bool s_noexcept_erase       () noexcept;


  using base_type::m_dense;
  using base_type::m_sparse;

public:
  constexpr explicit
  pool(allocator_type const &)
  noexcept;

  constexpr
  pool()
  noexcept(s_noexcept_default_construct());

  constexpr
  pool(pool const &, allocator_type const &);

  constexpr
  pool(pool const &)
  = default;

  constexpr
  pool(pool &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  pool(pool &&)
  = default;

  constexpr
  ~pool()
  override
  = default;

  constexpr pool &operator=(pool const &) = default;
  constexpr pool &operator=(pool &&     ) = default;

  using base_type
      ::get_allocator;

  constexpr void swap(pool &)                           noexcept(s_noexcept_swap());
  constexpr void swap(identifier_type, identifier_type) noexcept(s_noexcept_swap_entries());


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

  using base_type::size;
  using base_type::empty;
  using base_type::contains;

  [[nodiscard]] constexpr iterator       iterate(identifier_type)       noexcept;
  [[nodiscard]] constexpr const_iterator iterate(identifier_type) const noexcept;

  [[nodiscard]] constexpr iterator       find(identifier_type)       noexcept;
  [[nodiscard]] constexpr const_iterator find(identifier_type) const noexcept;

  [[nodiscard]] constexpr component_type       &operator[](identifier_type)       noexcept;
  [[nodiscard]] constexpr component_type const &operator[](identifier_type) const noexcept;

  [[nodiscard]] constexpr component_type       &at(identifier_type);
  [[nodiscard]] constexpr component_type const &at(identifier_type) const;


  template<typename ...Args> constexpr std::pair<iterator, bool> emplace    (identifier_type, Args &&...);
  template<typename ...Args> constexpr std::pair<iterator, bool> try_emplace(identifier_type, Args &&...);

  constexpr std::pair<iterator, bool> insert(identifier_type, component_type const &);
  constexpr std::pair<iterator, bool> insert(identifier_type, component_type &&     );

  constexpr std::pair<iterator, bool> insert_or_assign(identifier_type, component_type const &);
  constexpr std::pair<iterator, bool> insert_or_assign(identifier_type, component_type &&     );

  constexpr void     erase    (identifier_type)                noexcept(s_noexcept_erase()) override;
  constexpr iterator erase    (iterator       )                noexcept(s_noexcept_erase());
  constexpr iterator erase    (const_iterator )                noexcept(s_noexcept_erase());
  constexpr iterator erase    (iterator      , iterator      ) noexcept(s_noexcept_erase());
  constexpr iterator erase    (const_iterator, const_iterator) noexcept(s_noexcept_erase());

  constexpr bool     try_erase(identifier_type)                noexcept(s_noexcept_erase()) override;
  constexpr iterator try_erase(iterator       )                noexcept(s_noexcept_erase());
  constexpr iterator try_erase(const_iterator )                noexcept(s_noexcept_erase());
  constexpr iterator try_erase(iterator      , iterator      ) noexcept(s_noexcept_erase());
  constexpr iterator try_erase(const_iterator, const_iterator) noexcept(s_noexcept_erase());

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(pool &lhs, pool &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]]
  friend constexpr
  bool
  operator==(pool const &lhs, pool const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto const id : lhs.m_dense)
    {
      if (!rhs.contains(id))
        return false;

      if (lhs[id] != rhs[id])
        return false;
    }
    return true;
  }
};


template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::pointer
    ::pointer(reference &&ref)
noexcept
  : m_ref(std::move(ref))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference *
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::pointer
    ::operator->() const
noexcept
{
  return std::addressof(m_ref);
}


template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::generic_iterator(
        identifier_iterator const identifier_it,
        component_iterator  const component_it )
noexcept
  : m_identifier_it(identifier_it),
    m_component_it (component_it )
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator++()
noexcept
{
  ++m_identifier_it;
  ++m_component_it;
  return *this;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator++(int)
noexcept
{
  generic_iterator r(*this);
  ++*this;
  return r;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator--()
noexcept
{
  --m_identifier_it;
  --m_component_it;
  return *this;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator--(int)
noexcept
{
  generic_iterator r(*this);
  --*this;
  return r;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator+=(difference_type const n)
noexcept
{
  m_identifier_it += n;
  m_component_it  += n;
  return *this;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator-=(difference_type const n)
noexcept
{
  m_identifier_it -= n;
  m_component_it  -= n;
  return *this;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator*() const
noexcept
{
  return reference(*m_identifier_it, *m_component_it);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::pointer
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator->() const
noexcept
{
  return pointer(**this);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<bool IsConst>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference
pool<Component, Identifier, PageSize, Allocator>
    ::generic_iterator<IsConst>
    ::operator[](difference_type const n) const
noexcept
{
  return *(*this + n);
}


template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return
      base_type::s_noexcept_move_alloc_construct()
   && std::is_nothrow_constructible_v<
          component_container_type,
          component_container_type &&, component_allocator const &>;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::s_noexcept_swap()
noexcept
{
  return base_type::s_noexcept_swap()
      && std::is_nothrow_swappable_v<component_container_type>;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::s_noexcept_swap_entries()
noexcept
{
  return std::is_nothrow_swappable_v<component_type>;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::s_noexcept_erase()
noexcept
{
  return std::is_nothrow_move_assignable_v<component_type>;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::pool(allocator_type const &alloc)
noexcept
  : base_type   (alloc),
    m_components(component_allocator(alloc))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::pool()
noexcept(s_noexcept_default_construct())
  : pool(allocator_type{})
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::pool(pool const &other, allocator_type const &alloc)
  : base_type   (static_cast<base_type const &>(other), alloc),
    m_components(other.m_components, component_allocator(alloc))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
pool<Component, Identifier, PageSize, Allocator>
    ::pool(pool &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : base_type   (std::move(static_cast<base_type &&>(other)), alloc),
    m_components(std::move(other.m_components), component_allocator(alloc))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
pool<Component, Identifier, PageSize, Allocator>
    ::swap(pool &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  swap(m_components, other.m_components);
  base_type::swap(static_cast<base_type &>(other));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
pool<Component, Identifier, PageSize, Allocator>
    ::swap(identifier_type const lhs, identifier_type const rhs)
noexcept(s_noexcept_swap_entries())
{
  using std::swap;

  identifier_type &lhs_pos = m_sparse[lhs];
  identifier_type &rhs_pos = m_sparse[rhs];

  auto const lhs_idx = static_cast<size_type>(lhs_pos.idx());
  auto const rhs_idx = static_cast<size_type>(rhs_pos.idx());

  swap(m_components[lhs_idx], m_components[rhs_idx]);
  swap(m_dense     [lhs_idx], m_dense     [rhs_idx]);
  swap(lhs_pos              , rhs_pos              );
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::begin()
noexcept
{
  return iterator(base_type::begin(), m_components.rbegin());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::begin() const
noexcept
{
  return const_iterator(base_type::begin(), m_components.rbegin());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::cbegin() const
noexcept
{
  return begin();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::end()
noexcept
{
  return iterator(base_type::end(), m_components.rend());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::end() const
noexcept
{
  return const_iterator(base_type::end(), m_components.rend());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::cend() const
noexcept
{
  return end();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::rbegin()
noexcept
{
  return std::make_reverse_iterator(end());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::rbegin() const
noexcept
{
  return std::make_reverse_iterator(end());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::crbegin() const
noexcept
{
  return rbegin();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::rend()
noexcept
{
  return std::make_reverse_iterator(begin());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::rend() const
noexcept
{
  return std::make_reverse_iterator(begin());
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::crend() const
noexcept
{
  return rend();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id)
noexcept
{
  return iterator(base_type::iterate(id), m_components.rend() - m_sparse[id].index() - 1);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id) const
noexcept
{
  return const_iterator(base_type::iterate(id), m_components.rend() - m_sparse[id].index() - 1);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::find(identifier_type const id)
noexcept
{
  return contains(id) ? iterate(id) : end();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
pool<Component, Identifier, PageSize, Allocator>
    ::find(identifier_type const id) const
noexcept
{
  return contains(id) ? iterate(id) : end();
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::component_type &
pool<Component, Identifier, PageSize, Allocator>
    ::operator[](identifier_type const id)
noexcept
{
  return m_components[m_sparse[id].index()];
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::component_type const &
pool<Component, Identifier, PageSize, Allocator>
    ::operator[](identifier_type const id) const
noexcept
{
  return m_components[m_sparse[id].index()];
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::component_type &
pool<Component, Identifier, PageSize, Allocator>
    ::at(identifier_type const id)
{
  if (contains(id))
    return operator[](id);

  throw std::out_of_range("heim::sparse_set_based::pool::at");
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::component_type const &
pool<Component, Identifier, PageSize, Allocator>
    ::at(identifier_type const id) const
{
  if (contains(id))
    return operator[](id);

  throw std::out_of_range("heim::sparse_set_based::pool::at");
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::emplace(identifier_type const id, Args &&...args)
{
  m_sparse.prepare_for (id);
  m_dense .emplace_back(id);

  // strong exception safety guarantee
  try
  { m_components.emplace_back(std::forward<Args>(args)...); }
  catch (...)
  { m_dense.pop_back(); throw; }

  m_sparse[id] = identifier_type(m_dense.size() - 1, id.generation());

  return std::pair(begin(), true);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::try_emplace(identifier_type const id, Args &&...args)
{
  if (contains(id))
    return std::pair(iterate(id), false);

  return emplace(id, std::forward<Args>(args)...);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::insert(identifier_type const id, component_type const &c)
{
  return try_emplace(id, c);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::insert(identifier_type const id, component_type &&c)
{
  return try_emplace(id, std::move(c));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::insert_or_assign(identifier_type const id, component_type const &c)
{
  if (contains(id))
  {
    m_components[m_sparse[id].index()] = c;
    return std::pair(iterate(id), false);
  }

  return emplace(id, c);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
std::pair<typename pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
pool<Component, Identifier, PageSize, Allocator>
    ::insert_or_assign(identifier_type const id, component_type &&c)
{
  if (contains(id))
  {
    m_components[m_sparse[id].index()] = std::move(c);
    return std::pair(iterate(id), false);
  }

  return emplace(id, std::move(c));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
pool<Component, Identifier, PageSize, Allocator>
    ::erase(identifier_type const id)
noexcept(s_noexcept_erase())
{
  identifier_type const pos = m_sparse[id];
  auto            const idx = pos.index();

  if (idx != m_dense.size() - 1)
  {
    auto const
    size_idx
    = static_cast<size_type>(idx);

    m_components[idx]  = std::move(m_components.back());
    m_dense     [idx]  = std::move(m_dense     .back());
    m_sparse[m_dense[idx]] = identifier_type(idx, m_dense[size_idx].generation());
  }

  m_components.pop_back();
  m_dense     .pop_back();
  m_sparse.erase(id);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::erase(iterator const it)
noexcept(s_noexcept_erase())
{
  erase((*it).first);
  return it + 1;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::erase(const_iterator it)
noexcept(s_noexcept_erase())
{
  return erase(begin() + (it - cbegin()));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::erase(iterator first, iterator last)
noexcept(s_noexcept_erase())
{
  while (first != last)
    first = erase(first);

  return first;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::erase(const_iterator first, const_iterator last)
noexcept(s_noexcept_erase())
{
  return erase(begin() + (first - cbegin()), begin() + (last - cbegin()));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
bool
pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(identifier_type const id)
noexcept(s_noexcept_erase())
{
  if (!contains(id))
    return false;

  erase(id);
  return true;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(iterator const it)
noexcept(s_noexcept_erase())
{
  try_erase((*it).first);
  return it + 1;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(const_iterator it)
noexcept(s_noexcept_erase())
{
  return try_erase(begin() + (it - cbegin()));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(iterator first, iterator last)
noexcept(s_noexcept_erase())
{
  while (first != last)
    first = try_erase(first);

  return first;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename pool<Component, Identifier, PageSize, Allocator>
    ::iterator
pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(const_iterator first, const_iterator last)
noexcept(s_noexcept_erase())
{
  return try_erase(begin() + (first - cbegin()), begin() + (last - cbegin()));
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
pool<Component, Identifier, PageSize, Allocator>
    ::clear()
noexcept
{
  m_components.clear();
  base_type  ::clear();
}


/*!
 * @brief Determines whether the given type is a specialization of pool.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_pool;

template<typename T>
struct specializes_pool
  : bool_constant<false>
{ };

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
struct specializes_pool<
    pool<Component, Identifier, PageSize, Allocator>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
specializes_pool_v
= specializes_pool<T>::value;


}

#endif // HEIM_POOL_HPP