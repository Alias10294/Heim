#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <map>
#include <ranges>
#include <type_traits>
#include <vector>

using namespace heim_test::graph_algorithm::heap;

namespace
{
template <std::size_t Arity>
concept valid_heap_arity = requires {
  typename hg::detail::
      indirect_d_ary_heap<Arity, std::vector<int> &, priority_map, index_map, std::ranges::less>;
};

static_assert(!valid_heap_arity<0>);
static_assert(!valid_heap_arity<1>);
static_assert(valid_heap_arity<2>);
static_assert(valid_heap_arity<8>);
}

TEST_CASE("indirect heap accepts standard map based property maps", "[graph][algorithm][heap][customization]")
{
  std::map<int, int> priorities{{0, 20}, {1, 50}, {2, 10}};
  std::map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = hg::detail::indirect_d_ary_heap{std::integral_constant<std::size_t, 4>{},
                                              container,
                                              heim::make_property_map(priorities),
                                              heim::make_property_map(indices),
                                              std::ranges::less{}};

  heap.push(0);
  heap.push(1);
  heap.push(2);
  REQUIRE(heap.front() == 2);
  REQUIRE(container.at(indices.at(0)) == 0);
  REQUIRE(container.at(indices.at(1)) == 1);
  REQUIRE(container.at(indices.at(2)) == 2);

  priorities.at(1) = 1;
  heap.decrease_key(1);
  REQUIRE(heap.front() == 1);
  heap.pop();
  REQUIRE(heap.front() == 2);
}

TEST_CASE("indirect heap supports a reversed priority comparator", "[graph][algorithm][heap][customization]")
{
  std::unordered_map<int, int> priorities{{0, 10}, {1, 90}, {2, 30}, {3, 70}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = hg::detail::indirect_d_ary_heap{std::integral_constant<std::size_t, 2>{},
                                              container,
                                              priority_map{&priorities},
                                              index_map{&indices},
                                              std::ranges::greater{}};

  for (int id : {0, 1, 2, 3})
    heap.push(id);

  for (int expected : {1, 3, 2, 0})
  {
    REQUIRE(heap.front() == expected);
    heap.pop();
  }
  REQUIRE(heap.empty());
}

TEST_CASE("indirect heap with a default index map remains usable", "[graph][algorithm][heap][customization]")
{
  std::unordered_map<int, int> priorities{{0, 40}, {1, 10}, {2, 30}};
  std::vector<int> container;
  auto heap = hg::detail::indirect_d_ary_heap{std::integral_constant<std::size_t, 3>{},
                                              container,
                                              priority_map{&priorities},
                                              hg::default_property_map,
                                              std::ranges::less{}};

  heap.push(0);
  heap.push(1);
  heap.push(2);
  REQUIRE(heap.front() == 1);
  priorities.at(0) = 5;
  heap.decrease_key(0);
  REQUIRE(heap.front() == 0);
  heap.pop();
  REQUIRE(heap.front() == 1);
}

TEST_CASE("indirect heap recovers its external index after moving an element during pop",
          "[graph][algorithm][heap][customization]")
{
  std::unordered_map<int, int> priorities{{0, 1}, {1, 2}, {2, 3}, {3, 4}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<2>(container, priority_map{&priorities}, index_map{&indices});

  for (int id : {0, 1, 2, 3})
    heap.push(id);
  heap.pop();

  std::unordered_set<int> active{1, 2, 3};
  verify_heap<2>(container, priorities, indices, active);

  priorities.at(3) = 0;
  heap.decrease_key(3);
  REQUIRE(heap.front() == 3);
  verify_heap<2>(container, priorities, indices, active);
}
