#ifndef HEIM_NEW_POOL_HPP
#define HEIM_NEW_POOL_HPP

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
 * @brief ...
 *
 * @details ...
 *
 * @tparam Component  The component type.
 * @tparam Identifier The identifier type.
 * @tparam PageSize   The size of each internal page of positions.
 * @tparam Allocator  The allocator type.
 */
template<
    typename    Component,
    typename    Identifier = heim::identifier<>,
    std::size_t PageSize   = 1024,
    typename    Allocator  = std::allocator<Identifier>>
class new_pool;

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
class new_pool final
  : public sparse_set<Identifier, PageSize, Allocator>
{
public:
  using component_type  = Component;
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::new_pool: identifier_type must be a specialization of heim::identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::new_pool: allocator_type must pass as an allocator for identifier_type.");

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
    friend new_pool;

  public:
    constexpr static
    bool
    is_const
    = IsConst;


    using difference_type
    = std::ptrdiff_t;

    using value_type = component_type;
    using reference  = std::conditional_t<is_const, new_pool::const_reference, new_pool::reference>;

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
  constexpr static bool s_noexcept_swap () noexcept;
  constexpr static bool s_noexcept_erase() noexcept;

  [[nodiscard]] constexpr typename base_type::sparse_container       &m_sparse()       noexcept;
  [[nodiscard]] constexpr typename base_type::sparse_container const &m_sparse() const noexcept;
  [[nodiscard]] constexpr typename base_type::dense_container        &m_dense ()       noexcept;
  [[nodiscard]] constexpr typename base_type::dense_container  const &m_dense () const noexcept;

public:
  constexpr explicit
  new_pool(allocator_type const &)
  noexcept;

  constexpr
  new_pool()
  noexcept(s_noexcept_default_construct());

  constexpr
  new_pool(new_pool const &, allocator_type const &);

  constexpr
  new_pool(new_pool const &)
  = default;

  constexpr
  new_pool(new_pool &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  new_pool(new_pool &&)
  = default;

  constexpr
  ~new_pool()
  override
  = default;

  constexpr new_pool &operator=(new_pool const &) = default;
  constexpr new_pool &operator=(new_pool &&     ) = default;

  using base_type
      ::get_allocator;

  constexpr
  void
  swap(new_pool &)
  noexcept(s_noexcept_swap());


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

  constexpr void     erase(identifier_type) noexcept(s_noexcept_erase());
  constexpr iterator erase(iterator       ) noexcept(s_noexcept_erase());
  constexpr iterator erase(const_iterator ) noexcept(s_noexcept_erase());
  constexpr iterator erase(iterator      , iterator      ) noexcept(s_noexcept_erase());
  constexpr iterator erase(const_iterator, const_iterator) noexcept(s_noexcept_erase());

  constexpr bool     try_erase(identifier_type) noexcept(s_noexcept_erase());
  constexpr iterator try_erase(iterator       ) noexcept(s_noexcept_erase());
  constexpr iterator try_erase(const_iterator ) noexcept(s_noexcept_erase());
  constexpr iterator try_erase(iterator      , iterator      ) noexcept(s_noexcept_erase());
  constexpr iterator try_erase(const_iterator, const_iterator) noexcept(s_noexcept_erase());

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(new_pool &lhs, new_pool &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]]
  friend constexpr
  bool
  operator==(new_pool const &lhs, new_pool const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto const id : lhs.m_dense())
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
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference *
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst> &
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::pointer
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::template generic_iterator<IsConst>
    ::reference
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::base_type
    ::sparse_container &
new_pool<Component, Identifier, PageSize, Allocator>
    ::m_sparse()
noexcept
{
  return this->m_sparse;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::base_type
    ::sparse_container const &
new_pool<Component, Identifier, PageSize, Allocator>
    ::m_sparse() const
noexcept
{
  return this->m_sparse;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::base_type
    ::dense_container &
new_pool<Component, Identifier, PageSize, Allocator>
    ::m_dense()
noexcept
{
  return this->m_dense;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::base_type
    ::dense_container const &
new_pool<Component, Identifier, PageSize, Allocator>
    ::m_dense() const
noexcept
{
  return this->m_dense;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
new_pool<Component, Identifier, PageSize, Allocator>
    ::new_pool(allocator_type const &alloc)
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
new_pool<Component, Identifier, PageSize, Allocator>
    ::new_pool()
noexcept(s_noexcept_default_construct())
  : new_pool(allocator_type{})
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
new_pool<Component, Identifier, PageSize, Allocator>
    ::new_pool(new_pool const &other, allocator_type const &alloc)
  : base_type   (static_cast<base_type const &>(other)),
    m_components(other.m_components, component_allocator(alloc))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
new_pool<Component, Identifier, PageSize, Allocator>
    ::new_pool(new_pool &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : base_type   (std::move(static_cast<base_type &&>(other))),
    m_components(std::move(other.m_components), component_allocator(alloc))
{ }

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void
new_pool<Component, Identifier, PageSize, Allocator>
    ::swap(new_pool &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  base_type::swap(static_cast<base_type &>(other));
  swap(m_components, other.m_components);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_reverse_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id)
noexcept
{
  return iterator(base_type::iterate(id), m_components.rend() - m_sparse()[id].index() - 1);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
    ::iterate(identifier_type const id) const
noexcept
{
  return iterator(base_type::iterate(id), m_components.rend() - m_sparse()[id].index() - 1);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::const_iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::component_type &
new_pool<Component, Identifier, PageSize, Allocator>
    ::operator[](identifier_type const id)
noexcept
{
  return m_components[m_sparse()[id].index()];
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::component_type const &
new_pool<Component, Identifier, PageSize, Allocator>
    ::operator[](identifier_type const id) const
noexcept
{
  return m_components[m_sparse()[id].index()];
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::component_type &
new_pool<Component, Identifier, PageSize, Allocator>
    ::at(identifier_type const id)
{
  if (contains(id))
    return operator[](id);

  throw std::out_of_range("heim::sparse_set_based::new_pool::at");
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::component_type const &
new_pool<Component, Identifier, PageSize, Allocator>
    ::at(identifier_type const id) const
{
  if (contains(id))
    return operator[](id);

  throw std::out_of_range("heim::sparse_set_based::new_pool::at");
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
    ::emplace(identifier_type const id, Args &&...args)
{
  typename base_type::sparse_container &sparse = m_sparse();
  typename base_type::dense_container  &dense  = m_dense ();

  sparse.prepare_for (id);
  dense .emplace_back(id);

  // strong exception safety guarantee
  try
  { m_components.emplace_back(std::forward<Args>(args)...); }
  catch (...)
  { dense.pop_back(); throw; }

  sparse[id] = identifier_type(dense.size() - 1, id.generation());

  return std::pair(begin(), true);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
template<typename ...Args>
constexpr
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
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
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
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
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
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
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
    ::insert_or_assign(identifier_type const id, component_type const &c)
{
  if (contains(id))
  {
    m_components[m_sparse()[id].index()] = c;
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
std::pair<typename new_pool<Component, Identifier, PageSize, Allocator>::iterator, bool>
new_pool<Component, Identifier, PageSize, Allocator>
    ::insert_or_assign(identifier_type const id, component_type &&c)
{
  if (contains(id))
  {
    m_components[m_sparse()[id].index()] = std::move(c);
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
new_pool<Component, Identifier, PageSize, Allocator>
    ::erase(identifier_type const id)
noexcept(s_noexcept_erase())
{
  typename base_type::sparse_container &sparse = m_sparse();
  typename base_type::dense_container  &dense  = m_dense ();

  identifier_type const pos = sparse[id];
  auto            const idx = static_cast<size_type>(pos.index());

  if (pos.index() != dense.size() - 1)
  {
    m_components[idx]  = std::move(m_components.back());
    dense       [idx]  = std::move(dense       .back());
    sparse[dense[idx]] = pos;
  }

  m_components.pop_back();
  dense       .pop_back();
  sparse.erase(id);
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
    ::try_erase(iterator const it)
noexcept(s_noexcept_erase())
{
  try_erase(*it);
  return it + 1;
}

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
constexpr
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
typename new_pool<Component, Identifier, PageSize, Allocator>
    ::iterator
new_pool<Component, Identifier, PageSize, Allocator>
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
new_pool<Component, Identifier, PageSize, Allocator>
    ::clear()
noexcept
{
  m_components.clear();
  base_type  ::clear();
}


}

#endif // HEIM_NEW_POOL_HPP