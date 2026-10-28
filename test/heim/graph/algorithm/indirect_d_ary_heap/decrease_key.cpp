#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <limits>
#include <vector>

using namespace heim_test::graph_algorithm::heap;

TEST_CASE("indirect heap decrease_key promotes a previously nonminimal element",
          "[graph][algorithm][heap][decrease_key]")
{
  std::unordered_map<int, int> priorities{{0, 40}, {1, 20}, {2, 30}, {3, 50}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<2>(container, priority_map{&priorities}, index_map{&indices});
  std::unordered_set<int> active{0, 1, 2, 3};

  for (int vertex : {0, 1, 2, 3})
    heap.push(vertex);
  REQUIRE(heap.front() == 1);

  priorities.at(3) = 5;
  heap.decrease_key(3);
  REQUIRE(heap.front() == 3);
  verify_heap<2>(container, priorities, indices, active);

  heap.pop();
  active.erase(3);
  REQUIRE(heap.front() == 1);
  verify_heap<2>(container, priorities, indices, active);
}

TEST_CASE("indirect heap repeated decreases preserve all index positions",
          "[graph][algorithm][heap][decrease_key]")
{
  std::unordered_map<int, int> priorities{{0, 100}, {1, 90}, {2, 80}, {3, 70}, {4, 60}, {5, 50}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<4>(container, priority_map{&priorities}, index_map{&indices});
  std::unordered_set<int> active{0, 1, 2, 3, 4, 5};

  for (int id : {0, 1, 2, 3, 4, 5})
    heap.push(id);

  for (auto [id, new_priority] : {std::pair{0, 12}, std::pair{2, 8}, std::pair{4, 4}, std::pair{1, -10}})
  {
    priorities.at(id) = new_priority;
    heap.decrease_key(id);
    REQUIRE(heap.front() == id);
    verify_heap<4>(container, priorities, indices, active);
  }
}

TEST_CASE("indirect heap decrease_key works after unrelated removals",
          "[graph][algorithm][heap][decrease_key]")
{
  std::unordered_map<int, int> priorities{{1, 1}, {2, 30}, {3, 40}, {4, 50}, {5, 60}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<3>(container, priority_map{&priorities}, index_map{&indices});
  std::unordered_set<int> active{1, 2, 3, 4, 5};

  for (int id : {1, 2, 3, 4, 5})
    heap.push(id);
  REQUIRE(heap.front() == 1);
  heap.pop();
  active.erase(1);

  priorities.at(5) = -1;
  heap.decrease_key(5);
  REQUIRE(heap.front() == 5);
  verify_heap<3>(container, priorities, indices, active);
}

TEST_CASE("indirect heap handles extreme integral priorities", "[graph][algorithm][heap][decrease_key]")
{
  std::unordered_map<int, int> priorities{
      {0, std::numeric_limits<int>::max()}, {1, 0}, {2, std::numeric_limits<int>::min() + 1}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<2>(container, priority_map{&priorities}, index_map{&indices});

  heap.push(0);
  heap.push(1);
  heap.push(2);
  REQUIRE(heap.front() == 2);

  priorities.at(0) = std::numeric_limits<int>::min();
  heap.decrease_key(0);
  REQUIRE(heap.front() == 0);
  heap.pop();
  REQUIRE(heap.front() == 2);
}
