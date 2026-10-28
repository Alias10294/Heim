#ifndef HEIM_TEST_COMMON_SPARSE_INDEX_SUPPORT_HPP
#define HEIM_TEST_COMMON_SPARSE_INDEX_SUPPORT_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <heim/common/sparse.hpp>

namespace heim_test::sparse_index
{
template<typename UInt = unsigned, std::size_t PageSize = 8, typename Alloc = std::allocator<UInt>>
using index_type = heim::detail::sparse_index<UInt, PageSize, Alloc>;

template<typename I>
using key_type = typename I::value_type;

template<typename I>
constexpr auto null_value = std::numeric_limits<key_type<I>>::max();

template<typename I>
void check(I const &idx, key_type<I> key, key_type<I> expected)
{
  auto const *value = idx.get(key);
  REQUIRE(value != nullptr);
  REQUIRE(*value == expected);
}

template<typename I>
void check_missing(I const &idx, key_type<I> key)
{
  REQUIRE(idx.get(key) == nullptr);
}

template<typename I>
void verify_against(I const &idx,
                    std::unordered_map<key_type<I>, key_type<I>> const &reference,
                    key_type<I> limit)
{
  for (std::size_t k = 0; k < static_cast < std::size_t > (limit); ++k)
  {
    auto const key = static_cast<key_type<I>>(k);
    auto const it = reference.find(key);
    auto const *value = idx.get(key);
    if (it == reference.end() || it->second == null_value<I>)
      REQUIRE(value == nullptr);
    else
    {
      REQUIRE(value != nullptr);
      REQUIRE(*value == it->second);
    }
  }
}

// Explicit deterministic generator: independent of standard-library distributions.
class rng
{
  std::uint32_t state_;
public:
  explicit rng(std::uint32_t seed) : state_(seed ? seed : 1u) { }
  std::uint32_t next() noexcept
  {
    auto x = state_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state_ = x;
  }
  std::uint32_t
  bounded(std::uint32_t bound) noexcept
  { return next() % bound; }
};

template<typename T>
struct is_page : std::false_type { };
template<typename UInt, std::size_t N>
struct is_page<std::array<UInt, N>> : std::bool_constant<std::is_unsigned_v<UInt>> { };

struct block
{
  std::size_t elements{};
  int owner{};
  bool page{};
};

struct allocation_log
{
  std::map<void *, block> outstanding;
  std::size_t allocations{};
  std::size_t deallocations{};
  std::size_t page_allocations{};
  std::size_t page_deallocations{};
  std::size_t invalid_deallocations{};
  bool fail_page_allocation{};
  bool fail_all_allocation{};
  std::size_t remaining = std::numeric_limits<std::size_t>::max();

  std::size_t live_pages() const
  {
    return static_cast<std::size_t>(std::ranges::count_if(
        outstanding, [](auto const &entry)
        {
          return entry.second.page;
        }));
  }
  bool balanced() const
  {
    return outstanding.empty() && invalid_deallocations == 0
        && allocations == deallocations && page_allocations == page_deallocations;
  }
};

// Stateful, rebind-safe allocator. Its log follows the storage through every rebind.
template<typename T, bool Propagate = false>
class tracked_allocator
{
public:
  using value_type = T;
  template<typename U>
  struct rebind
  { using other = tracked_allocator<U, Propagate>; };
  using propagate_on_container_copy_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_move_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_swap = std::bool_constant<Propagate>;
  using is_always_equal = std::false_type;

  std::shared_ptr<allocation_log> log;
  int owner{};

  tracked_allocator() : log{std::make_shared<allocation_log>()} { }
  explicit tracked_allocator(std::shared_ptr<allocation_log> state, int id = 0)
    : log{std::move(state)}, owner{id} { }

  template<typename U>
  tracked_allocator(tracked_allocator<U, Propagate> const &other) noexcept
    : log{other.log}, owner{other.owner} { }

  T *allocate(std::size_t count)
  {
    if (log->fail_all_allocation || (log->fail_page_allocation && is_page<T>::value)
        || log->remaining == 0)
      throw std::bad_alloc{};

    auto *p = std::allocator<T>{}.allocate(count);
    try
    {
      log->outstanding.emplace(static_cast<void *>(p), block{count, owner, is_page<T>::value});
    }
    catch (...)
    {
      std::allocator<T>{}.deallocate(p, count);
      throw;
    }
    if (log->remaining != std::numeric_limits<std::size_t>::max())
      --log->remaining;
    ++log->allocations;
    if constexpr (is_page<T>::value)
  ++log->page_allocations;
    return p;
  }

  void deallocate(T *p, std::size_t count) noexcept
  {
    auto const it = log->outstanding.find(static_cast<void *>(p));
    if (it == log->outstanding.end())
      ++log->invalid_deallocations;
    else
    {
      if (it->second.elements != count || it->second.owner != owner
          || it->second.page != is_page<T>::value)
        ++log->invalid_deallocations;
      log->outstanding.erase(it);
    }
    ++log->deallocations;
    if constexpr (is_page<T>::value)
  ++log->page_deallocations;
    std::allocator<T>{}.deallocate(p, count);
  }

  template<typename U, bool P>
  friend class tracked_allocator;

  template<typename U>
  bool operator == (tracked_allocator<U, Propagate> const &other) const noexcept
  { return log == other.log && owner == other.owner; }
};

template<std::size_t PageSize = 8, bool Propagate = false>
using observed_index = index_type<unsigned, PageSize, tracked_allocator<unsigned, Propagate>>;

// The implementation's get() deliberately treats UINT_MAX as the absent marker.
template<typename I>
void set_and_model(I &idx,
                   std::unordered_map<key_type<I>, key_type<I>> &model,
                   key_type<I> key, key_type<I> value)
{
  idx.assure(key) = value;
  model[key] = value;
}

}
#endif
