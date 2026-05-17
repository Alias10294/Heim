#ifndef HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP
#define HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <utility>
#include "heim/ecs/entity.hpp"

namespace heim::sparse::detail
{
template<typename Registry>
class generic_static_registry_iterator
{
public:
  using registry_type
  = Registry;

  using difference_type  = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using value_type       = entity<registry_type>;
  using reference        = entity<registry_type>;

private:
  using iterator_type
  = typename registry_type::manager_type::const_iterator;

private:
  registry_type *m_registry;
  iterator_type  m_iterator;

public:
  constexpr
  generic_static_registry_iterator()
  noexcept
    : m_registry{}
    , m_iterator{}
  { }

  constexpr
  generic_static_registry_iterator(generic_static_registry_iterator const &)
  = default;

  constexpr
  generic_static_registry_iterator(generic_static_registry_iterator &&)
  = default;

  constexpr
  generic_static_registry_iterator(registry_type &registry, iterator_type const iterator)
  noexcept
    : m_registry{&registry}
    , m_iterator{iterator}
  { }

  constexpr
  ~generic_static_registry_iterator()
  = default;

  constexpr
  generic_static_registry_iterator &
  operator=(generic_static_registry_iterator const &)
  = default;

  constexpr
  generic_static_registry_iterator &
  operator=(generic_static_registry_iterator &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_iterator, other.m_iterator);
  }

  friend constexpr
  void
  swap(generic_static_registry_iterator &lhs, generic_static_registry_iterator &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_iterator const &, generic_static_registry_iterator const &)
  = default;

  [[nodiscard]] friend constexpr
  auto
  operator<=>(generic_static_registry_iterator const &, generic_static_registry_iterator const &)
  = default;


  [[nodiscard]] constexpr
  reference
  operator*() const
  noexcept
  { return reference{*m_registry, *m_iterator}; }

  [[nodiscard]] constexpr
  reference
  operator[](difference_type const n) const
  noexcept
  { return *(*this + n); }


  constexpr
  generic_static_registry_iterator &
  operator++()
  noexcept
  { ++m_iterator; return *this; }

  constexpr
  generic_static_registry_iterator &
  operator--()
  noexcept
  { --m_iterator; return *this; }

  constexpr
  generic_static_registry_iterator
  operator++(int)
  noexcept
  {
    generic_static_registry_iterator tmp{*this};
    ++*this;
    return tmp;
  }

  constexpr
  generic_static_registry_iterator
  operator--(int)
  noexcept
  {
    generic_static_registry_iterator tmp{*this};
    --*this;
    return tmp;
  }

  constexpr
  generic_static_registry_iterator &
  operator+=(difference_type const n)
  { m_iterator += n; return *this; }

  constexpr
  generic_static_registry_iterator &
  operator-=(difference_type const n)
  { m_iterator -= n; return *this; }

  friend constexpr
  generic_static_registry_iterator
  operator+(generic_static_registry_iterator it, difference_type const n)
  noexcept
  { it += n; return it; }

  friend constexpr
  generic_static_registry_iterator
  operator+(difference_type const n, generic_static_registry_iterator it)
  noexcept
  { it += n; return it; }

  friend constexpr
  generic_static_registry_iterator
  operator-(generic_static_registry_iterator it, difference_type const n)
  noexcept
  { it -= n; return it; }

  friend constexpr
  difference_type
  operator-(generic_static_registry_iterator const lhs, generic_static_registry_iterator const rhs)
  noexcept
  { return lhs.m_iterator - rhs.m_iterator; }
};

} // namespace heim::sparse::detail

#endif // HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP
