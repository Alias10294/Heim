#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <utility>
#include <vector>

using namespace heim_test::graph_algorithm::heap;

namespace
{
struct counted_less
{
  int *calls;

  bool operator()(int lhs, int rhs) const
  {
    ++*calls;
    return lhs < rhs;
  }
};
}

TEST_CASE("indirect heap owns an rvalue container", "[graph][algorithm][heap][ownership]")
{
  std::unordered_map<int, int> priorities{{0, 7}, {1, 2}, {2, 5}};
  std::unordered_map<int, std::size_t> indices;
  auto heap = hg::detail::indirect_d_ary_heap{std::integral_constant<std::size_t, 2>{},
                                              std::vector<int>{},
                                              priority_map{&priorities},
                                              index_map{&indices},
                                              std::ranges::less{}};

  heap.push(0);
  heap.push(1);
  heap.push(2);
  REQUIRE(heap.front() == 1);
  heap.pop();
  REQUIRE(heap.front() == 2);
  heap.pop();
  REQUIRE(heap.front() == 0);
  heap.pop();
  REQUIRE(heap.empty());
}

TEST_CASE("indirect heap forwards an lvalue comparison object", "[graph][algorithm][heap][ownership]")
{
  std::unordered_map<int, int> priorities{{0, 15}, {1, 5}, {2, 10}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  int comparison_count{};
  counted_less comparator{&comparison_count};

  auto heap = hg::detail::indirect_d_ary_heap{std::integral_constant<std::size_t, 3>{},
                                              container,
                                              priority_map{&priorities},
                                              index_map{&indices},
                                              comparator};

  heap.push(0);
  heap.push(1);
  heap.push(2);
  REQUIRE(comparison_count > 0);
  REQUIRE(heap.front() == 1);
  heap.pop();
  REQUIRE(heap.front() == 2);
}
