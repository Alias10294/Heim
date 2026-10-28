#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <ranges>

using namespace heim_test::graph_algorithm::predecessor;

TEST_CASE("predecessor_view copied iterators advance independently",
          "[graph][algorithm][predecessor_view][iterator]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};
  auto first = path.begin();
  auto second = first;

  REQUIRE(first == second);
  ++first;
  REQUIRE(*first == 5);
  REQUIRE(*second == 7);
  REQUIRE(first != second);
  ++second;
  REQUIRE(first == second);
}

TEST_CASE("predecessor_view postfix increment preserves previous vertex",
          "[graph][algorithm][predecessor_view][iterator]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};
  auto it = path.begin();

  auto previous = it++;
  REQUIRE(*previous == 7);
  REQUIRE(*it == 5);
}

TEST_CASE("predecessor_view iterator arrow exposes the current value",
          "[graph][algorithm][predecessor_view][iterator]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};
  auto it = path.begin();

  REQUIRE(*it.operator->().operator->() == 7);
  ++it;
  REQUIRE(*it.operator->().operator->() == 5);
}

TEST_CASE("predecessor_view reaches sentinel after the root",
          "[graph][algorithm][predecessor_view][iterator]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 0};
  auto it = path.begin();

  REQUIRE(it != path.end());
  REQUIRE(*it == 0);
  ++it;
  REQUIRE(it == path.end());
}

TEST_CASE("predecessor_view iterator owns a copy of its map",
          "[graph][algorithm][predecessor_view][iterator]")
{
  auto parents = chain();
  auto it = view{parent_map{&parents}, 7}.begin();

  REQUIRE(*it == 7);
  ++it;
  REQUIRE(*it == 5);
  ++it;
  REQUIRE(*it == 3);
}
