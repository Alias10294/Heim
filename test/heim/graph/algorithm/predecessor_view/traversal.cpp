#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <vector>

using namespace heim_test::graph_algorithm::predecessor;

TEST_CASE("predecessor_view follows the predecessor chain to its root",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};

  REQUIRE((collect(path) == std::vector<int>{7, 5, 3, 1, 0}));
}

TEST_CASE("predecessor_view includes the root exactly once",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 0};

  REQUIRE((collect(path) == std::vector<int>{0}));
}

TEST_CASE("predecessor_view includes an isolated self-predecessor",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 12};

  REQUIRE((collect(path) == std::vector<int>{12}));
}

TEST_CASE("predecessor_view can be iterated repeatedly", "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};

  auto first = collect(path);
  auto second = collect(path);
  REQUIRE(first == second);
}

TEST_CASE("predecessor_view works through a const view", "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view const path{parent_map{&parents}, 5};

  REQUIRE((collect(path) == std::vector<int>{5, 3, 1, 0}));
}

TEST_CASE("predecessor_view traverses independent branches sharing a root",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  parents[9] = 3;
  parents[10] = 9;

  REQUIRE((collect(view{parent_map{&parents}, 10}) == std::vector<int>{10, 9, 3, 1, 0}));
  REQUIRE((collect(view{parent_map{&parents}, 7}) == std::vector<int>{7, 5, 3, 1, 0}));
}

TEST_CASE("predecessor_view reflects external predecessor changes between traversals",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};
  REQUIRE((collect(path) == std::vector<int>{7, 5, 3, 1, 0}));

  parents[7] = 1;
  REQUIRE((collect(path) == std::vector<int>{7, 1, 0}));
}

TEST_CASE("predecessor_view propagates an invalid predecessor lookup",
          "[graph][algorithm][predecessor_view][traversal]")
{
  auto parents = chain();
  parents[5] = 42;
  view path{parent_map{&parents}, 7};

  auto it = path.begin();
  REQUIRE(*it == 7);
  ++it;
  REQUIRE(*it == 5);
  ++it;
  REQUIRE(*it == 42);
  REQUIRE_THROWS_AS(++it, std::out_of_range);
}
