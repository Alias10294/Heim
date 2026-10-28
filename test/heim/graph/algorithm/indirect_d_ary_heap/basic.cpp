#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <concepts>
#include <deque>
#include <functional>
#include <type_traits>
#include <vector>

using namespace heim_test::graph_algorithm::heap;

namespace
{
template <std::size_t Arity>
void check_sorting()
{
  std::unordered_map<int, int> priorities{{0, 70}, {1, 30}, {2, 10}, {3, 60}, {4, 40}, {5, 20}, {6, 50}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<Arity>(container, priority_map{&priorities}, index_map{&indices});

  REQUIRE(heap.empty());
  std::unordered_set<int> active;
  for (int vertex : {3, 1, 5, 0, 6, 2, 4})
  {
    heap.push(vertex);
    active.insert(vertex);
    verify_heap<Arity>(container, priorities, indices, active);
  }

  for (int expected : {2, 5, 1, 4, 6, 3, 0})
  {
    REQUIRE_FALSE(heap.empty());
    REQUIRE(heap.front() == expected);
    heap.pop();
    active.erase(expected);
    verify_heap<Arity>(container, priorities, indices, active);
  }
  REQUIRE(heap.empty());
  REQUIRE(container.empty());
}
}

TEST_CASE("indirect heap sorts distinct priorities for arity two", "[graph][algorithm][heap][basic]")
{
  check_sorting<2>();
}

TEST_CASE("indirect heap sorts distinct priorities for arity three", "[graph][algorithm][heap][basic]")
{
  check_sorting<3>();
}

TEST_CASE("indirect heap sorts distinct priorities for arity four", "[graph][algorithm][heap][basic]")
{
  check_sorting<4>();
}

TEST_CASE("indirect heap sorts distinct priorities for arity eight", "[graph][algorithm][heap][basic]")
{
  check_sorting<8>();
}

TEST_CASE("indirect heap accepts equal priorities without requiring stable tie order",
          "[graph][algorithm][heap][basic]")
{
  std::unordered_map<int, int> priorities{{1, 4}, {2, 4}, {3, 4}, {4, 2}, {5, 4}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<3>(container, priority_map{&priorities}, index_map{&indices});
  std::unordered_set<int> active;

  for (int vertex : {1, 2, 3, 4, 5})
  {
    heap.push(vertex);
    active.insert(vertex);
    verify_heap<3>(container, priorities, indices, active);
  }

  std::vector<int> output;
  while (!heap.empty())
  {
    output.push_back(priorities.at(heap.front()));
    int removed = heap.front();
    heap.pop();
    active.erase(removed);
    verify_heap<3>(container, priorities, indices, active);
  }
  REQUIRE((output == std::vector<int>{2, 4, 4, 4, 4}));
}

TEST_CASE("indirect heap works with a deque as its container", "[graph][algorithm][heap][basic]")
{
  std::unordered_map<int, int> priorities{{3, 30}, {2, 20}, {1, 10}};
  std::unordered_map<int, std::size_t> indices;
  std::deque<int> container;
  auto heap = make_heap<2>(container, priority_map{&priorities}, index_map{&indices});

  heap.push(3);
  heap.push(2);
  heap.push(1);
  REQUIRE(heap.front() == 1);
  heap.pop();
  REQUIRE(heap.front() == 2);
  heap.pop();
  REQUIRE(heap.front() == 3);
  heap.pop();
  REQUIRE(heap.empty());
}

TEST_CASE("indirect heap forwards access to the supplied external container",
          "[graph][algorithm][heap][basic]")
{
  std::unordered_map<int, int> priorities{{1, 5}, {2, 3}};
  std::unordered_map<int, std::size_t> indices;
  std::vector<int> container;
  auto heap = make_heap<2>(container, priority_map{&priorities}, index_map{&indices});

  heap.push(1);
  REQUIRE(container.size() == 1);
  heap.push(2);
  REQUIRE(container.size() == 2);
  REQUIRE(container.front() == 2);
  heap.pop();
  REQUIRE(container.size() == 1);
}
