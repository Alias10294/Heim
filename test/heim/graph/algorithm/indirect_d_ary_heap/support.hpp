#ifndef HEIM_TEST_GRAPH_ALGORITHM_INDIRECT_HEAP_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_INDIRECT_HEAP_SUPPORT_HPP

#include <heim/graph/algorithm/indirect_d_ary_heap.hpp>
#include <heim/property/property_map.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace heim_test::graph_algorithm::heap
{
namespace hg = heim::graphs;
namespace hp = heim::properties;

struct priority_map
{
  std::unordered_map<int, int> *values{};

  int get(int vertex) const { return values->at(vertex); }
};

struct index_map
{
  std::unordered_map<int, std::size_t> *values{};

  std::size_t get(int vertex) const { return values->at(vertex); }

  void set(int vertex, std::size_t pos) const { values->insert_or_assign(vertex, pos); }
};

struct rng
{
  std::uint32_t state;

  explicit rng(std::uint32_t seed) : state{seed ? seed : 1u} {}

  std::uint32_t next() noexcept
  {
    auto value = state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return state = value;
  }

  std::size_t bounded(std::size_t upper) noexcept { return static_cast<std::size_t>(next()) % upper; }
};

template <std::size_t Arity, typename Container>
auto make_heap(Container &container, priority_map priorities, index_map indices)
{
  return hg::detail::indirect_d_ary_heap{
      std::integral_constant<std::size_t, Arity>{}, container, priorities, indices, std::ranges::less{}};
}

template <std::size_t Arity, typename Container>
void verify_heap(Container const &container,
                 std::unordered_map<int, int> const &priorities,
                 std::unordered_map<int, std::size_t> const &indices,
                 std::unordered_set<int> const &active)
{
  REQUIRE(container.size() == active.size());

  std::unordered_set<int> seen;
  for (std::size_t pos = 0; pos < container.size(); ++pos)
  {
    int const vertex = container[pos];
    INFO("heap arity=" << Arity << " position=" << pos << " vertex=" << vertex);
    REQUIRE(active.contains(vertex));
    REQUIRE(seen.insert(vertex).second);
    REQUIRE(indices.at(vertex) == pos);
    REQUIRE(priorities.contains(vertex));

    if (pos > 0)
    {
      std::size_t const parent = (pos - 1) / Arity;
      REQUIRE(priorities.at(container[parent]) <= priorities.at(vertex));
    }
  }
  REQUIRE(seen == active);
}

} // namespace heim_test::graph_algorithm::heap

#endif // HEIM_TEST_GRAPH_ALGORITHM_INDIRECT_HEAP_SUPPORT_HPP
