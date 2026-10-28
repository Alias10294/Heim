#include "support.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using namespace heim_test::graph_interface;

static_assert(std::same_as<decltype(graphs::source(std::declval<member_graph const &>(), 10)), int>);
static_assert(std::same_as<decltype(graphs::target(std::declval<member_graph const &>(), 10)), int>);
static_assert(
    std::same_as<decltype(graphs::endpoints(std::declval<member_graph const &>(), 10)), std::pair<int, int>>);
static_assert(noexcept(graphs::source(std::declval<member_graph const &>(), 10)));
static_assert(noexcept(graphs::target(std::declval<adl::graph const &>(), 10)));
static_assert(!std::invocable<decltype(graphs::source), empty_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::target), empty_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::endpoints), empty_graph const &, int>);

TEST_CASE(
    "edge source, target, and endpoints prefer member functions",
    "[graph][interface][primitives][endpoints]")
{
  member_graph g;
  REQUIRE(graphs::source(g, 10) == 0);
  REQUIRE(graphs::target(g, 10) == 1);
  REQUIRE(graphs::endpoints(g, 10) == std::pair(0, 1));
}

TEST_CASE("edge source, target, and endpoints resolve via ADL", "[graph][interface][primitives][endpoints]")
{
  adl::graph g;
  REQUIRE(graphs::source(g, 12) == 0);
  REQUIRE(graphs::target(g, 12) == 1);
  REQUIRE(graphs::endpoints(g, 12) == std::pair(0, 1));
}

TEST_CASE("edge endpoints fall back to source and target", "[graph][interface][primitives][endpoints]")
{
  fallback_graph<true> g;
  REQUIRE(graphs::endpoints(g, 11) == std::pair(0, 2));
  static_assert(noexcept(graphs::endpoints(g, 11)));
}

TEST_CASE(
    "edge source CPO falls back to ADL when member is deleted",
    "[graph][interface][primitives][endpoints]")
{
  invalid_member::graph g;
  REQUIRE(graphs::source(g, 0) == 4);
}
