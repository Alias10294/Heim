#include <catch2/catch_test_macros.hpp>
#include <heim/graph/algorithm/indirect_d_ary_heap.hpp>
#include <heim/property.hpp>
#include <algorithm>
#include <array>
#include <type_traits>
#include <cstddef>
#include <functional>
#include <random>
#include <unordered_map>
#include <vector>

namespace {
template<std::size_t Arity, typename Priorities, typename Indices>
auto make_heap(Priorities& priorities, Indices& indices)
{
  return heim::graphs::detail::indirect_d_ary_heap{
    std::integral_constant<std::size_t, Arity>{}, std::vector<unsigned>{},
    heim::make_readable_property_map(priorities),
    heim::make_property_map(indices), std::less<>{}};
}
}

TEST_CASE("indirect d-ary heap returns keys in ascending priority", "[graph][heap]")
{
  std::unordered_map<unsigned, unsigned> priorities{{0u, 50u}, {1u, 10u}, {2u, 30u}, {3u, 20u}};
  std::unordered_map<unsigned, std::size_t> indices;
  auto heap = make_heap<4>(priorities, indices);
  REQUIRE(heap.empty());
  for (unsigned i = 0; i < 4; ++i) heap.push(i);
  REQUIRE_FALSE(heap.empty());
  for (unsigned key : {1u, 3u, 2u, 0u}) {
    REQUIRE(heap.front() == key);
    heap.pop();
  }
  REQUIRE(heap.empty());
}

TEST_CASE("indirect d-ary heap decrease_key updates the minimum", "[graph][heap]")
{
  std::unordered_map<unsigned, unsigned> priorities{{0u, 50u}, {1u, 40u}, {2u, 30u}, {3u, 20u}};
  std::unordered_map<unsigned, std::size_t> indices;
  auto heap = make_heap<2>(priorities, indices);
  for (unsigned i = 0; i < 4; ++i) heap.push(i);
  REQUIRE(heap.front() == 3u);
  priorities[0u] = 5u;
  heap.decrease_key(0u);
  REQUIRE(heap.front() == 0u);
  heap.pop();
  REQUIRE(heap.front() == 3u);
}

TEST_CASE("indirect d-ary heap handles many unique priorities", "[graph][heap][model]")
{
  constexpr unsigned n = 150;
  std::unordered_map<unsigned, unsigned> priorities;
  std::unordered_map<unsigned, std::size_t> indices;
  auto heap = make_heap<3>(priorities, indices);
  std::vector<unsigned> ids(n);
  for (unsigned i = 0; i < n; ++i) { ids[i] = i; priorities.emplace(i, n - i); }
  std::mt19937 rng{0x0BEEFu};
  std::shuffle(ids.begin(), ids.end(), rng);
  for (unsigned key : ids) heap.push(key);
  for (unsigned key = n; key-- > 0;) {
    REQUIRE(heap.front() == key);
    heap.pop();
  }
  REQUIRE(heap.empty());
}
