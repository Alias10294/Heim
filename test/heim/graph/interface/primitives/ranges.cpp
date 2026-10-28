#include "support.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using namespace heim_test::graph_interface;

static_assert(std::ranges::sized_range<decltype(graphs::vertices(std::declval<member_graph const &>()))>);
static_assert(
    std::same_as<std::ranges::range_value_t<decltype(graphs::vertices(std::declval<member_graph const &>()))>,
                 int>);
static_assert(
    std::same_as<std::ranges::range_value_t<decltype(graphs::edges(std::declval<member_graph const &>()))>,
                 int>);
static_assert(noexcept(graphs::vertices(std::declval<member_graph const &>())));
static_assert(noexcept(graphs::edges(std::declval<adl::graph const &>())));
static_assert(!std::invocable<decltype(graphs::vertices), empty_graph const &>);
static_assert(!std::invocable<decltype(graphs::edges), empty_graph const &>);
static_assert(
    []
    {
      member_graph g;
      return std::ranges::size(graphs::vertices(g)) == 3 && std::ranges::size(graphs::edges(g)) == 2;
    }());

TEST_CASE("graph vertices and edges prefer members to ADL", "[graph][interface][primitives][ranges]")
{
  member_graph const g;
  auto vs = graphs::vertices(g);
  auto es = graphs::edges(g);
  REQUIRE(std::ranges::size(vs) == 3);
  REQUIRE(std::ranges::size(es) == 2);
  REQUIRE(&*std::ranges::begin(vs) == &g.vertex_ids[0]);
  REQUIRE(&*std::ranges::begin(es) == &g.edge_ids[0]);
  REQUIRE((std::ranges::equal(vs, std::array{0, 1, 2})));
  REQUIRE((std::ranges::equal(es, std::array{10, 11})));
}

TEST_CASE("graph vertices and edges use ADL when no member exists", "[graph][interface][primitives][ranges]")
{
  adl::graph const g;
  REQUIRE((std::ranges::equal(graphs::vertices(g), std::array{0, 1, 2})));
  REQUIRE((std::ranges::equal(graphs::edges(g), std::array{12, 13})));
}

TEST_CASE("a member with an invalid vertex range falls back to ADL", "[graph][interface][primitives][ranges]")
{
  invalid_member::graph const g;
  REQUIRE((std::ranges::equal(graphs::vertices(g), std::array{4, 5})));
}
