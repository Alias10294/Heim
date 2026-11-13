#ifndef HEIM_COMMON_SPARSE_INDEX_HPP
#define HEIM_COMMON_SPARSE_INDEX_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/common/unique_allocated_ptr.hpp"

namespace heim
{
template<typename = void>
struct default_page_size
  : std::integral_constant<std::size_t, 1024>
{ };

inline constexpr
std::size_t
default_page_size_v
= default_page_size<>::value;


namespace detail
{
template<
    typename    UInt,
    std::size_t PageSize  = default_page_size_v,
    typename    Allocator = std::allocator<UInt>>
requires std::unsigned_integral<UInt>
      && std::same_as          <UInt, typename std::allocator_traits<Allocator>::value_type>
class sparse_index
{
public:
  static constexpr std::size_t page_size = PageSize;
  static constexpr bool        is_paged  = page_size != 0;

  using value_type     = UInt;
  using allocator_type = Allocator;

private:
  using alloc_traits
  = std::allocator_traits<allocator_type>;

  using page
  = std::array<value_type, page_size>;

  using page_allocator    = typename alloc_traits::template rebind_alloc <page>;
  using page_alloc_traits = typename alloc_traits::template rebind_traits<page>;

  using page_pointer
  = unique_allocated_ptr<page, page_allocator, false>;

  using page_pointer_allocator    = typename alloc_traits::template rebind_alloc <page_pointer>;
  using page_pointer_alloc_traits = typename alloc_traits::template rebind_traits<page_pointer>;

  using container_type
  = std::conditional_t<
      is_paged,
      std::vector<page_pointer, page_pointer_allocator>,
      std::vector<value_type  , allocator_type        >>;

  using container_allocator    = typename container_type::allocator_type;
  using container_alloc_traits = std::allocator_traits<container_allocator>;

private:
  container_type m_cont;

private:
  constexpr
  sparse_index(sparse_index const &other, std::bool_constant<true>)
    : m_cont{container_alloc_traits::select_on_container_copy_construction(other.m_cont.get_allocator())}
  { this->m_copy(other.m_cont); }

  constexpr
  sparse_index(sparse_index const &other, std::bool_constant<false>)
    : m_cont{other.m_cont}
  { }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc, std::bool_constant<true>)
    : sparse_index{alloc}
  { this->m_copy(other.m_cont); }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc, std::bool_constant<false>)
    : m_cont{other.m_cont, container_allocator{alloc}}
  { }

  constexpr
  page_pointer
  m_null_page() const
  {
    return page_pointer{
      typename page_pointer::pointer     {},
      typename page_pointer::deleter_type{page_allocator{this->m_cont.get_allocator()}}};
  }

  constexpr
  void
  m_copy(container_type const &from)
  {
    this->m_cont.reserve(from.size());

    for (page_pointer const &ptr : from)
    {
      if (ptr)
        this->m_cont.emplace_back(allocate_unique<page>(page_allocator{this->m_cont.get_allocator()}, *ptr));
      else
        this->m_cont.emplace_back(m_null_page());
    }
  }

  template<typename R>
  constexpr
  void
  m_index(R &&rg)
  {
    value_type idx{};

    for (value_type val : rg)
      this->assure(val) = idx++;
  }

public:
  constexpr
  sparse_index(sparse_index const &other)
    : sparse_index{other, std::bool_constant<is_paged>{}}
  { }

  constexpr
  sparse_index(sparse_index &&)
  = default;

  explicit constexpr
  sparse_index(allocator_type const &alloc)
    : m_cont{container_allocator{alloc}}
  { }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc)
    : sparse_index{other, alloc, std::bool_constant<is_paged>{}}
  { }

  constexpr
  sparse_index(sparse_index &&other, allocator_type const &alloc)
    : m_cont{std::move(other.m_cont), container_allocator{alloc}}
  { }

  template<typename R>
  explicit constexpr
  sparse_index(R &&rg, allocator_type const &alloc)
  requires std::ranges::input_range<R>
    : sparse_index{alloc}
  { this->m_index(std::forward<R>(rg)); }

  template<typename R>
  explicit constexpr
  sparse_index(R &&rg)
  requires std::ranges::input_range<R>
    : sparse_index{std::forward<R>(rg), allocator_type{}}
  { }

  constexpr
  ~sparse_index()
  = default;

  constexpr
  sparse_index &
  operator=(sparse_index const &other)
  {
    if constexpr (is_paged)
    {
      if (this == std::addressof(other))
        return *this;

      if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
      {
        auto * const ptr{std::addressof(this->m_cont)};

        std::destroy_at  (ptr);
        std::construct_at(ptr, other.m_cont.get_allocator());
      }
      else
        this->m_cont.clear();

      this->m_copy(other.m_cont);
    }
    else
      this->m_cont = other.m_cont;

    return *this;
  }

  constexpr
  sparse_index &
  operator=(sparse_index &&other)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (!is_paged || container_alloc_traits::propagate_on_container_move_assignment::value)
      this->m_cont = std::move(other.m_cont);
    else
    {
      if (this->m_cont.get_allocator() == other.m_cont.get_allocator())
      {
        container_type moved{std::move(other.m_cont), this->m_cont.get_allocator()};
        this->m_cont.swap(moved);
      }
      else
      {
        this->m_cont.clear  ();
        this->m_cont.reserve(other.m_cont.size());

        for (auto &ptr : other.m_cont)
          this->m_cont.emplace_back(std::move(ptr));

        other.m_cont.clear();
      }
    }

    return *this;
  }

  constexpr
  void
  swap(sparse_index &other)
  {
    using std::swap;

    swap(this->m_cont, other.m_cont);
  }

  friend constexpr
  void
  swap(sparse_index &lhs, sparse_index &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  { lhs.swap(rhs); }

  constexpr
  allocator_type
  get_allocator() const
  noexcept
  {
    if constexpr (is_paged)
      return allocator_type{this->m_cont.get_allocator()};
    else
      return this->m_cont.get_allocator();
  }


  constexpr
  auto
  max_size() const
  noexcept
  {
    constexpr
    std::size_t
    value_limit
    = []
    {
      if constexpr (std::numeric_limits<value_type>::digits >= std::numeric_limits<std::size_t>::digits)
        return std::numeric_limits<std::size_t>::max();
      else
        return static_cast<std::size_t>(std::numeric_limits<value_type>::max());
    }();

    if constexpr (is_paged)
    {
      auto const nb_pages{this->m_cont.max_size()};

      return nb_pages > value_limit / page_size
           ? value_limit
           : std::min(nb_pages * page_size, value_limit);
    }
    else
      return std::min(this->m_cont.max_size(), value_limit);
  }


  constexpr
  value_type &
  operator[](value_type val)
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
      return (*this->m_cont[idx / page_size])[idx % page_size];
    else
      return this->m_cont[idx];
  }

  constexpr
  value_type
  operator[](value_type val) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
      return (*this->m_cont[idx / page_size])[idx % page_size];
    else
      return this->m_cont[idx];
  }

  constexpr
  auto
  get(value_type val)
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size() || !this->m_cont[page_idx])
        return static_cast<value_type *>(nullptr);

      auto &value{(*this->m_cont[page_idx])[idx % page_size]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
    else
    {
      if (idx >= this->m_cont.size())
        return static_cast<value_type *>(nullptr);

      auto &value{this->m_cont[idx]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
  }

  constexpr
  auto
  get(value_type val) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size() || !this->m_cont[page_idx])
        return static_cast<value_type const *>(nullptr);

      auto const &value{(*this->m_cont[page_idx])[idx % page_size]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
    else
    {
      if (idx >= this->m_cont.size())
        return static_cast<value_type const *>(nullptr);

      auto const &value{this->m_cont[idx]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
  }

  constexpr
  value_type &
  assure(value_type val)
  {
    constexpr auto null{std::numeric_limits<value_type>::max()};
    auto const     idx {static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size())
      {
        this->m_cont.reserve(page_idx + 1);

        while (this->m_cont.size() <= page_idx)
          this->m_cont.emplace_back(this->m_null_page());
      }

      auto &ptr{this->m_cont[page_idx]};

      if (!ptr)
      {
        auto allocated{allocate_unique<page>(page_allocator{this->m_cont.get_allocator()})};

        std::destroy_at  (std::addressof(ptr));
        std::construct_at(std::addressof(ptr), std::move(allocated));
        ptr->fill(null);
      }

      return (*ptr)[idx % page_size];
    }
    else
    {
      if (idx >= this->m_cont.size())
        this->m_cont.resize(idx + 1, null);

      return this->m_cont[idx];
    }
  }

  constexpr
  void
  clear()
  noexcept
  { this->m_cont.clear(); }

  template<typename R>
  constexpr
  void
  reset(R &&rg)
  {
    this->clear  ();
    this->m_index(std::forward<R>(rg));
  }
};


template<typename Sparse>
struct clear_guard
{
  Sparse *cont;


  constexpr
  ~clear_guard()
  noexcept
  {
    if (this->cont)
      this->cont->clear();
  }


  constexpr
  void
  disable()
  noexcept
  { this->cont = nullptr; }
};

} // namespace detail


struct unique_t
{ };

inline constexpr
unique_t
unique;

} // namespace heim

#endif // HEIM_COMMON_SPARSE_INDEX_HPP
