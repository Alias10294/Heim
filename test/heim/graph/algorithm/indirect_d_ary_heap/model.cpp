#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace heim_test::graph_algorithm::heap;

namespace
{
// Keep the runtime-only model separate from GCC 16's speculative constant
// evaluation of constexpr contract assertions in indirect_d_ary_heap::front.
template <typename Heap>
auto runtime_front(Heap const &heap)
{
  return heap.front();
}

template <std::size_t Arity>
void run_model(std::uint32_t seed, unsigned operations)
{
  std::unordered_map<int, int> priorities;
  std::unordered_map<int, std::size_t> indices;
  std::unordered_set<int> active;
  std::vector<int> container;
  auto heap = make_heap<Arity>(container, priority_map{&priorities}, index_map{&indices});
  rng random{seed};
  int next_vertex{};

  for (unsigned step = 0; step < operations; ++step)
  {
    INFO("arity=" << Arity << " seed=" << seed << " step=" << step);
    unsigned const operation = static_cast<unsigned>(random.bounded(100));

    if (active.empty() || (operation < 48 && active.size() < 60))
    {
      int const id = next_vertex++;
      int const priority = static_cast<int>(random.bounded(201)) - 100;
      priorities.emplace(id, priority);
      active.insert(id);
      heap.push(id);
    }
    else if (operation < 78)
    {
      std::vector<int> candidates(active.begin(), active.end());
      std::ranges::sort(candidates);
      int const id = candidates.at(random.bounded(candidates.size()));
      int const decrement = static_cast<int>(random.bounded(45)) + 1;
      priorities.at(id) -= decrement;
      heap.decrease_key(id);
    }
    else
    {
      int const removed = runtime_front(heap);
      int const minimum =
          std::ranges::min(active, std::ranges::less{}, [&priorities](int id) { return priorities.at(id); });
      REQUIRE(priorities.at(removed) == priorities.at(minimum));
      heap.pop();
      active.erase(removed);
    }

    verify_heap<Arity>(container, priorities, indices, active);
    REQUIRE(heap.empty() == active.empty());
    if (!active.empty())
    {
      int const minimum =
          std::ranges::min(active, std::ranges::less{}, [&priorities](int id) { return priorities.at(id); });
      REQUIRE(priorities.at(runtime_front(heap)) == priorities.at(minimum));
    }
  }

  int previous = std::numeric_limits<int>::min();
  while (!heap.empty())
  {
    int const removed = runtime_front(heap);
    int const priority = priorities.at(removed);
    REQUIRE(priority >= previous);
    previous = priority;
    heap.pop();
    active.erase(removed);
    verify_heap<Arity>(container, priorities, indices, active);
  }
  REQUIRE(active.empty());
}
}

TEST_CASE("indirect heap model: binary heap, reproducible mutations", "[graph][algorithm][heap][model]")
{
  run_model<2>(0xDEADBEEFu, 850);
  run_model<2>(0x12345678u, 850);
}

TEST_CASE("indirect heap model: ternary heap, reproducible mutations", "[graph][algorithm][heap][model]")
{
  run_model<3>(0xDEADBEEFu, 850);
  run_model<3>(0x12345678u, 850);
}

TEST_CASE("indirect heap model: four-way heap, reproducible mutations", "[graph][algorithm][heap][model]")
{
  run_model<4>(0xDEADBEEFu, 850);
  run_model<4>(0x12345678u, 850);
}

TEST_CASE("indirect heap model: eight-way heap, reproducible mutations", "[graph][algorithm][heap][model]")
{
  run_model<8>(0xDEADBEEFu, 850);
  run_model<8>(0x12345678u, 850);
}
