#include "support.hpp"
#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace heim_test::graph_interface;

static_assert(std::same_as<decltype(graphs::place_vertex(std::declval<member_graph &>())), int>);
static_assert(std::same_as<decltype(graphs::place_edge(std::declval<member_graph &>(), 0, 1)), std::pair<int, bool>>);
static_assert(std::same_as<decltype(graphs::remove_vertex(std::declval<member_graph &>(), 0)), void>);
static_assert(std::same_as<decltype(graphs::remove_edge(std::declval<member_graph &>(), 10)), void>);
static_assert(std::same_as<decltype(graphs::remove_edges(std::declval<member_graph &>(), 0, 1)), void>);
static_assert(std::same_as<decltype(graphs::clear_vertex(std::declval<member_graph &>(), 0)), void>);
static_assert(noexcept(graphs::place_vertex(std::declval<member_graph &>())));
static_assert(noexcept(graphs::place_edge(std::declval<member_graph &>(), 0, 1)));
static_assert(noexcept(graphs::remove_edge(std::declval<adl::graph &>(), 10)));
static_assert(!std::invocable<decltype(graphs::place_vertex), empty_graph &>);
static_assert(!std::invocable<decltype(graphs::place_edge), empty_graph &, int, int>);
static_assert(!std::invocable<decltype(graphs::remove_vertex), empty_graph &, int>);
static_assert(!std::invocable<decltype(graphs::remove_edge), empty_graph &, int>);
static_assert(!std::invocable<decltype(graphs::remove_edges), empty_graph &, int, int>);
static_assert(!std::invocable<decltype(graphs::clear_vertex), empty_graph &, int>);
static_assert(!std::invocable<decltype(graphs::place_vertex), member_graph const &>);
static_assert(!std::invocable<decltype(graphs::place_edge), member_graph const &, int, int>);
static_assert(!std::invocable<decltype(graphs::remove_vertex), member_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::remove_edge), member_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::remove_edges), member_graph const &, int, int>);
static_assert(!std::invocable<decltype(graphs::clear_vertex), member_graph const &, int>);

TEST_CASE("graph mutation CPOs prefer members", "[graph][interface][primitives][mutation]")
{
  member_graph g;
  REQUIRE(graphs::place_vertex(g) == 3);
  REQUIRE(g.action == 1);
  REQUIRE(graphs::place_edge(g, 0, 1) == std::pair(12, true));
  REQUIRE(g.action == 2);
  graphs::remove_vertex(g, 0);
  REQUIRE(g.action == 3);
  graphs::remove_edge(g, 10);
  REQUIRE(g.action == 4);
  graphs::remove_edges(g, 0, 1);
  REQUIRE(g.action == 5);
  graphs::clear_vertex(g, 0);
  REQUIRE(g.action == 6);
}

TEST_CASE("graph mutation CPOs resolve through ADL", "[graph][interface][primitives][mutation]")
{
  adl::graph g;
  REQUIRE(graphs::place_vertex(g) == 3);
  REQUIRE(g.action == 1);
  REQUIRE(graphs::place_edge(g, 0, 1) == std::pair(14, true));
  REQUIRE(g.action == 2);
  graphs::remove_vertex(g, 0);
  REQUIRE(g.action == 3);
  graphs::remove_edge(g, 12);
  REQUIRE(g.action == 4);
  graphs::remove_edges(g, 0, 1);
  REQUIRE(g.action == 5);
  graphs::clear_vertex(g, 0);
  REQUIRE(g.action == 6);
}

TEST_CASE("graph mutation CPOs propagate exceptions", "[graph][interface][primitives][mutation]")
{
  throwing_graph g;
  static_assert(!noexcept(graphs::remove_vertex(g, 0)));
  REQUIRE_THROWS_AS(graphs::remove_vertex(g, 0), std::runtime_error);
}
