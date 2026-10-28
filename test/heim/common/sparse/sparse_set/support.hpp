#ifndef HEIM_TEST_COMMON_SPARSE_SET_SUPPORT_HPP
#define HEIM_TEST_COMMON_SPARSE_SET_SUPPORT_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <ranges>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <heim/common/sparse.hpp>

namespace heim_test::sparse_set
{
template<typename UInt = unsigned, std::size_t Page = 8,
         typename Alloc = std::allocator<UInt>>
using set = heim::sparse_set<UInt, Page, std::vector<UInt, Alloc>, Alloc>;

template<typename Set>
using key_type = typename Set::key_type;

template<typename Set>
void verify(Set const &actual,
            std::unordered_set<key_type<Set>> const &expected,
            std::size_t probe_limit = 256,
            std::initializer_list<key_type<Set>> extra_probes = {})
{
  REQUIRE(actual.size() == expected.size());
  REQUIRE(actual.empty() == expected.empty());
  REQUIRE(actual.size() <= actual.max_size());

  std::unordered_set<key_type<Set>> seen;
  for (auto key : actual)
  {
    INFO("iterated key=" << static_cast<std::uint64_t>(key));
    REQUIRE(expected.contains(key));
    REQUIRE(seen.insert(key).second);
    REQUIRE(actual.contains(key));
    auto pos = actual.find(key);
    REQUIRE(pos != actual.end());
    REQUIRE(*pos == key);
  }
  REQUIRE(seen == expected);

  for (auto key : expected)
  {
    INFO("expected key=" << static_cast<std::uint64_t>(key));
    REQUIRE(actual.contains(key));
    REQUIRE(actual.find(key) != actual.end());
  }

  // Iterate a fixed domain to catch stale sparse indices that are not present
  // in the dense sequence (e.g. after erase, move, or a failed insertion).
  const auto maximum_probe = std::min<std::size_t>(
      probe_limit, static_cast<std::size_t>(std::numeric_limits<key_type<Set>>::max()));
  for (std::size_t raw = 0; raw <= maximum_probe && raw < probe_limit; ++raw)
  {
    auto key = static_cast<key_type<Set>>(raw);
    bool present = expected.contains(key);
    INFO("probe=" << raw);
    REQUIRE(actual.contains(key) == present);
    if (present || raw % 17 == 0)
      REQUIRE((actual.find(key) != actual.end()) == present);
  }
  for (auto key : extra_probes)
  {
    INFO("explicitly probed key=" << static_cast<std::uint64_t>(key));
    bool const present = expected.contains(key);
    REQUIRE(actual.contains(key) == present);
    REQUIRE((actual.find(key) != actual.end()) == present);
  }
}

template<typename Set>
void verify_dense_iterators(Set const &actual)
{
  auto const backward = std::vector<key_type<Set>>(actual.begin(), actual.end());
  auto const forward = std::vector<key_type<Set>>(actual.rbegin(), actual.rend());
  REQUIRE(backward.size() == actual.size());
  REQUIRE(forward.size() == actual.size());
  REQUIRE(std::ranges::equal(backward, std::views::reverse(forward)));
  REQUIRE(std::ranges::distance(actual.cbegin(), actual.cend()) ==
          static_cast<std::ptrdiff_t>(actual.size()));
  REQUIRE(std::ranges::distance(actual.crbegin(), actual.crend()) ==
          static_cast<std::ptrdiff_t>(actual.size()));
  for (std::size_t i = 0; i < actual.size(); ++i)
  {
    auto iter = actual.cbegin() + static_cast<std::ptrdiff_t>(i);
    REQUIRE(*iter == backward[i]);
    REQUIRE(*(actual.crbegin() + static_cast<std::ptrdiff_t>(i)) == forward[i]);
  }
}

// Deliberately fixed RNG: deterministic across standard-library implementations.
struct rng
{
  std::uint32_t state;
  explicit rng(std::uint32_t seed) : state(seed ? seed : 1u) { }
  std::uint32_t next() noexcept
  {
    auto x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
  }
  std::uint32_t
  bounded(std::uint32_t n) noexcept
  { return next() % n; }
};

template<typename T>
struct is_page : std::false_type { };
template<typename UInt, std::size_t N>
struct is_page<std::array<UInt, N>> : std::true_type { };

struct block
{
  std::size_t count;
  int         owner;
  bool        page;
};
struct allocation_log
{
  std::map<void *, block> outstanding;
  std::size_t allocations{};
  std::size_t deallocations{};
  std::size_t page_allocations{};
  std::size_t wrong_deallocations{};
  bool reject_all{};
  bool reject_pages{};
  bool reject_dense{};
  std::size_t remaining = std::numeric_limits<std::size_t>::max();

  std::size_t live_pages() const
  {
    return static_cast<std::size_t>(std::ranges::count_if(outstanding,
      [](auto const &item)
      {
        return item.second.page;
      }));
  }
  bool balanced() const
  {
    return outstanding.empty() && wrong_deallocations == 0 && allocations == deallocations;
  }
};

template<typename T, bool Propagate = false>
struct tracked_allocator
{
  using value_type = T;
  using propagate_on_container_copy_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_move_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_swap = std::bool_constant<Propagate>;
  using is_always_equal = std::false_type;
  template<typename U>
  struct rebind
  { using other = tracked_allocator<U, Propagate>; };

  std::shared_ptr<allocation_log> log;
  int owner{};
  tracked_allocator() : log(std::make_shared<allocation_log>()) { }
  tracked_allocator(std::shared_ptr<allocation_log> l, int o = 0) : log(std::move(l)), owner(o) { }
  template<typename U>
  tracked_allocator(tracked_allocator<U, Propagate> const &o) noexcept : log(o.log), owner(o.owner) { }

  T *allocate(std::size_t count)
  {
    if (log->reject_all || (log->reject_pages && is_page<T>::value) ||
        (log->reject_dense && std::is_same_v<T, unsigned>) || log->remaining == 0)
      throw std::bad_alloc{};
    --log->remaining;
    T *p = std::allocator<T>{}.allocate(count);
    try
    {
      log->outstanding.emplace(static_cast<void *>(p), block{count, owner, is_page<T>::value});
    }
    catch (...) { std::allocator<T>{}.deallocate(p, count);
    throw; }
    ++log->allocations;
    if constexpr (is_page<T>::value)
  ++log->page_allocations;
    return p;
  }
  void deallocate(T *p, std::size_t count) noexcept
  {
    auto it = log->outstanding.find(static_cast<void *>(p));
    if (it == log->outstanding.end())
      ++log->wrong_deallocations;
    else
    {
      if (it->second.count != count || it->second.owner != owner ||
          it->second.page != is_page<T>::value)
        ++log->wrong_deallocations;
      log->outstanding.erase(it);
    }
    ++log->deallocations;
    std::allocator<T>{}.deallocate(p, count);
  }
  template<typename U, bool P> friend struct tracked_allocator;
  template<typename U>
  bool operator == (tracked_allocator<U, Propagate> const &o) const noexcept
  { return log == o.log && owner == o.owner; }
};

template<std::size_t Page = 8, bool Propagate = false>
using tracked_set = set<unsigned, Page, tracked_allocator<unsigned, Propagate>>;

}
#endif
