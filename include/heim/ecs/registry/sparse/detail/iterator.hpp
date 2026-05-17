#ifndef HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP
#define HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <utility>
#include "heim/ecs/entity.hpp"

namespace heim::sparse::detail
{
template<typename Registry>
class registry_iterator
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
  registry_iterator()
  noexcept
    : m_registry{}
    , m_iterator{}
  { }

  constexpr
  registry_iterator(registry_iterator const &)
  = default;

  constexpr
  registry_iterator(registry_iterator &&)
  = default;

  constexpr
  registry_iterator(registry_type &registry, iterator_type const iterator)
  noexcept
    : m_registry{&registry}
    , m_iterator{iterator}
  { }

  constexpr
  ~registry_iterator()
  = default;

  constexpr
  registry_iterator &
  operator=(registry_iterator const &)
  = default;

  constexpr
  registry_iterator &
  operator=(registry_iterator &&)
  = default;

  constexpr
  void
  swap(registry_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_iterator, other.m_iterator);
  }

  friend constexpr
  void
  swap(registry_iterator &lhs, registry_iterator &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_iterator const &, registry_iterator const &)
  = default;

  [[nodiscard]] friend constexpr
  auto
  operator<=>(registry_iterator const &, registry_iterator const &)
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
  registry_iterator &
  operator++()
  noexcept
  { ++m_iterator; return *this; }

  constexpr
  registry_iterator &
  operator--()
  noexcept
  { --m_iterator; return *this; }

  constexpr
  registry_iterator
  operator++(int)
  noexcept
  {
    registry_iterator tmp{*this};
    ++*this;
    return tmp;
  }

  constexpr
  registry_iterator
  operator--(int)
  noexcept
  {
    registry_iterator tmp{*this};
    --*this;
    return tmp;
  }

  constexpr
  registry_iterator &
  operator+=(difference_type const n)
  { m_iterator += n; return *this; }

  constexpr
  registry_iterator &
  operator-=(difference_type const n)
  { m_iterator -= n; return *this; }

  friend constexpr
  registry_iterator
  operator+(registry_iterator it, difference_type const n)
  noexcept
  { it += n; return it; }

  friend constexpr
  registry_iterator
  operator+(difference_type const n, registry_iterator it)
  noexcept
  { it += n; return it; }

  friend constexpr
  registry_iterator
  operator-(registry_iterator it, difference_type const n)
  noexcept
  { it -= n; return it; }

  friend constexpr
  difference_type
  operator-(registry_iterator const lhs, registry_iterator const rhs)
  noexcept
  { return lhs.m_iterator - rhs.m_iterator; }
};

} // namespace heim::sparse::detail

#endif // HEIM_ECS_REGISTRY_SPARSE_DETAIL_ITERATOR_HPP
