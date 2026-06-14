#ifndef HEIM_ECS_REGISTRY_SPARSE_STORAGE_HPP
#define HEIM_ECS_REGISTRY_SPARSE_STORAGE_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/lib/utility.hpp"

namespace heim::ecs::sparse
{
template<
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
class automatic_storage
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

  static constexpr std::size_t page_size
  = PageSz;

private:
  using alloc_traits
  = std::allocator_traits<Alloc>;

  using set_type      = set<Id, PageSz, Alloc>;
  using set_allocator = typename alloc_traits::template rebind_alloc<set_type>;

  using set_pointer           = std::shared_ptr<set_type>;
  using set_pointer_allocator = typename alloc_traits::template rebind_alloc<set_pointer>;
  using set_pointer_container = std::vector<set_pointer, set_pointer_allocator>;

  template<typename C>
  using pool_for_type
  = pool<C, Id, PageSz, Alloc>;

private:
  set_pointer_container m_container;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        set_pointer_container,
        set_pointer_container, set_pointer_allocator>;
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
  { return std::is_nothrow_swappable_v<set_pointer_container>; }


  static constexpr
  std::size_t
  s_next_index()
  noexcept
  {
    static std::atomic_size_t counter{0};

    return counter.fetch_add(std::size_t{1}, std::memory_order_relaxed);
  }

  template<typename C>
  requires (!qualified<C>)
  static constexpr
  std::size_t
  s_index()
  noexcept
  {
    static std::size_t const idx{s_next_index()};

    return idx;
  }

public:
  explicit constexpr
  automatic_storage(allocator_type const alloc)
  noexcept
    : m_container{alloc}
  { }

  constexpr
  automatic_storage(automatic_storage const &other, allocator_type const alloc)
    : m_container{other.m_container, alloc}
  { }

  constexpr
  automatic_storage(automatic_storage &&other, allocator_type const alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_container{std::move(other.m_container), alloc}
  { }

  constexpr
  automatic_storage()
  noexcept(s_noexcept_default_construct())
    : automatic_storage{allocator_type{}}
  { }

  constexpr
  automatic_storage(automatic_storage const &)
  = default;

  constexpr
  automatic_storage(automatic_storage &&)
  = default;

  constexpr
  ~automatic_storage()
  = default;

  constexpr
  automatic_storage &
  operator=(automatic_storage const &)
  = default;

  constexpr
  automatic_storage &
  operator=(automatic_storage &&)
  = default;

  constexpr
  void
  swap(automatic_storage &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_container, other.m_container);
  }

  friend constexpr
  void
  swap(automatic_storage &lhs, automatic_storage &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{m_container.get_allocator()}; }

  [[nodiscard]] friend constexpr
  bool
  operator==(automatic_storage const &, automatic_storage const &)
  = default;


  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> &
  assure()
  {
    std::size_t const idx{s_index<C>()};

    if (m_container.size() <= idx) [[unlikely]]
      m_container.resize(idx + 1);

    if (allocator_type alloc{get_allocator()}; !m_container[idx]) [[unlikely]]
      m_container[idx] = std::allocate_shared<pool_for_type<C>>(alloc, alloc);

    return static_cast<pool_for_type<C> &>(*m_container[idx]);
  }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> &
  pool()
  noexcept
  { return static_cast<pool_for_type<C> &>(*m_container[s_index<C>()]); }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> const &
  pool() const
  noexcept
  { return static_cast<pool_for_type<C> &>(*m_container[s_index<C>()]); }


  constexpr
  void
  clear(identifier_type const id)
  { for (auto &pool : m_container) pool->try_erase(id); }

  constexpr
  void
  clear()
  { for (auto &pool : m_container) pool->clear(); }
};

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_STORAGE_HPP
