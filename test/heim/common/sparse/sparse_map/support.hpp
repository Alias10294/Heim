#ifndef HEIM_TEST_COMMON_SPARSE_MAP_SUPPORT_HPP
#define HEIM_TEST_COMMON_SPARSE_MAP_SUPPORT_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <memory_resource>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <heim/common/sparse.hpp>

namespace heim_test::sparse_map
{
template<typename UInt = unsigned, typename T = int, std::size_t Page = 8>
using map = heim::sparse_map<UInt, T, Page>;

template<typename Map>
using key_type = typename Map::key_type;

template<typename Map>
using mapped_type = typename Map::mapped_type;

template<typename Map>
std::unordered_map<key_type<Map>, mapped_type<Map>> snapshot(Map const &actual)
{
  std::unordered_map<key_type<Map>, mapped_type<Map>> result;
  for (auto [key, value] : actual)
    result.emplace(key, value);
  return result;
}

// Checks both directions, immutable/mutable lookups through the public API,
// key/value alignment, uniqueness and stale entries absent from dense storage.
template<typename Map>
void verify(Map const &actual,
            std::unordered_map<key_type<Map>, mapped_type<Map>> const &expected,
            std::size_t probe_limit = 256,
            std::initializer_list<key_type<Map>> extra_probes = {})
{
  REQUIRE(actual.size() == expected.size());
  REQUIRE(actual.empty() == expected.empty());
  REQUIRE(actual.size() <= actual.max_size());

  std::unordered_map<key_type<Map>, mapped_type<Map>> observed;
  for (auto [key, value] : actual)
  {
    INFO("iterated key=" << static_cast<std::uint64_t>(key));
    REQUIRE(expected.contains(key));
    REQUIRE(observed.emplace(key, value).second);
    REQUIRE(expected.at(key) == value);
    REQUIRE(actual.contains(key));
    auto it = actual.find(key);
    REQUIRE(it != actual.end());
    REQUIRE(it->first == key);
    REQUIRE(it->second == value);
    REQUIRE(actual.at(key) == value);
    REQUIRE(actual[key] == value);
  }
  REQUIRE(observed == expected);

  for (auto const &[key, value] : expected)
  {
    INFO("expected key=" << static_cast<std::uint64_t>(key));
    REQUIRE(actual.contains(key));
    REQUIRE(actual.at(key) == value);
    auto it = actual.find(key);
    REQUIRE(it != actual.end());
    REQUIRE(it->second == value);
  }

  // Avoid narrowing wraparound for small unsigned key types.
  const auto maximum_probe = std::min<std::size_t>(
      probe_limit, static_cast<std::size_t>(std::numeric_limits<key_type<Map>>::max()));
  for (std::size_t raw = 0; raw <= maximum_probe && raw < probe_limit; ++raw)
  {
    auto key = static_cast<key_type<Map>>(raw);
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

struct allocation_log
{
  enum class region
  {
    keys,
    values,
    pages,
    page_table
  };
  struct block
  {
    std::size_t count;
    int         owner;
    region      kind;
  };
  std::array<std::size_t, 4> allocated_by_region{};
  std::array<std::size_t, 4> deallocated_by_region{};
  std::map<void *, block> outstanding;
  std::size_t allocations{};
  std::size_t deallocations{};
  std::size_t wrong_deallocations{};
  bool reject{};
  std::size_t remaining = std::numeric_limits<std::size_t>::max();

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

  std::shared_ptr<allocation_log> state{};
  int id{};

  tracked_allocator() = default;
  tracked_allocator(std::shared_ptr<allocation_log> s, int n) : state(std::move(s)), id(n) { }
  template<typename U>
  tracked_allocator(tracked_allocator<U, Propagate> const &other) noexcept
    : state(other.state), id(other.id) { }

  static constexpr allocation_log::region region_of() noexcept
  {
    if constexpr (std::is_same_v<T, unsigned>)
  return allocation_log::region::keys;
    else if constexpr (std::is_same_v<T, int>)
  return allocation_log::region::values;
    else if constexpr (requires { std::tuple_size<T>::value; })
  return allocation_log::region::pages;
    else return allocation_log::region::page_table;
  }

  T *allocate(std::size_t n)
  {
    if (state && (state->reject || state->remaining == 0))
      throw std::bad_alloc{};
    auto *p = std::allocator<T>{}.allocate(n);
    if (state)
    {
      if (state->remaining != std::numeric_limits<std::size_t>::max())
        --state->remaining;
      // Logging is allowed to throw. If it does, release the actual allocation.
      try
      {
        state->outstanding.emplace(static_cast<void *>(p), allocation_log::block{n, id, region_of()});
      }
      catch (...)
      {
        std::allocator<T>{}.deallocate(p, n);
        throw;
      }
      ++state->allocated_by_region[static_cast<std::size_t>(region_of())];
      ++state->allocations;
    }
    return p;
  }
  void deallocate(T *p, std::size_t n) noexcept
  {
    if (state)
    {
      auto it = state->outstanding.find(static_cast<void *>(p));
      if (it == state->outstanding.end() || it->second.owner != id || it->second.count != n)
        ++state->wrong_deallocations;
      else
      {
        if (it->second.kind != region_of())
          ++state->wrong_deallocations;
        ++state->deallocated_by_region[static_cast<std::size_t>(region_of())];
        state->outstanding.erase(it);
      }
      ++state->deallocations;
    }
    std::allocator<T>{}.deallocate(p, n);
  }
  template<typename, bool>
  friend struct tracked_allocator;
  template<typename U>
  friend bool operator == (tracked_allocator const &a, tracked_allocator<U, Propagate> const &b) noexcept
  { return a.id == b.id && a.state == b.state; }
};

template<bool Propagate = false>
using tracked_map = heim::sparse_map<
  unsigned, int, 8,
  std::vector<unsigned, tracked_allocator<unsigned, Propagate>>,
  std::vector<int, tracked_allocator<int, Propagate>>,
  tracked_allocator<std::pair<unsigned, int>, Propagate>>;

}
#endif
