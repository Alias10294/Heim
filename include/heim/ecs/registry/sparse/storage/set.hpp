#ifndef HEIM_ECS_REGISTRY_SPARSE_SET_HPP
#define HEIM_ECS_REGISTRY_SPARSE_SET_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace heim::ecs::sparse
{
template<
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
class generic_set
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

  static constexpr std::size_t page_size
  = PageSz;

private:
  using sparse_container = ;
  using dense_container  = ;

private:
  sparse_container m_sparse;
  dense_container  m_dense;

private:
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


  [[nodiscard]] constexpr auto begin() noexcept;
  [[nodiscard]] constexpr auto begin() const noexcept;

  [[nodiscard]] constexpr auto end() noexcept;
  [[nodiscard]] constexpr auto end() const noexcept;

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end(); }

  [[nodiscard]] constexpr auto rbegin() noexcept;
  [[nodiscard]] constexpr auto rbegin() const noexcept;

  [[nodiscard]] constexpr auto rend() noexcept;
  [[nodiscard]] constexpr auto rend() const noexcept;

  [[nodiscard]] constexpr auto crbegin() const noexcept { return rbegin(); }
  [[nodiscard]] constexpr auto crend  () const noexcept { return rend(); }

  [[nodiscard]] constexpr auto size () const noexcept { return std::ranges::size (m_dense); }
  [[nodiscard]] constexpr bool empty() const noexcept { return std::ranges::empty(m_dense); }

  [[nodiscard]] constexpr
  bool
  contains(identifier_type const id) const
  noexcept;

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id)
  noexcept;

  [[nodiscard]] constexpr
  auto
  find(identifier_type const id) const
  noexcept;


  template<typename ...Args>
  constexpr
  void
  emplace(Args && ...args);

  template<typename ...Args>
  constexpr
  bool
  try_emplace(Args && ...args);

  constexpr
  bool
  insert(identifier_type const id);

  virtual constexpr
  void
  erase(identifier_type const id);

  virtual constexpr
  bool
  try_erase(identifier_type const id);

  constexpr
  void
  clear()
  noexcept;
};

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SET_HPP
