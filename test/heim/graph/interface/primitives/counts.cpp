#include "support.hpp"
#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

using namespace heim_test::graph_interface;

static_assert(std::same_as<decltype(graphs::vertex_count(std::declval<member_graph const &>())), int>);
static_assert(std::same_as<decltype(graphs::edge_count(std::declval<member_graph const &>())), int>);
static_assert(noexcept(graphs::vertex_count(std::declval<member_graph const &>())));
static_assert(noexcept(graphs::edge_count(std::declval<adl::graph const &>())));
static_assert(!std::invocable<decltype(graphs::vertex_count), empty_graph const &>);
static_assert(!std::invocable<decltype(graphs::edge_count), empty_graph const &>);

TEST_CASE("graph count CPOs prefer member functions", "[graph][interface][primitives][counts]")
{
  member_graph g;
  REQUIRE(graphs::vertex_count(g) == 3);
  REQUIRE(graphs::edge_count(g) == 2);
  REQUIRE(graphs::order(g) == 3);
  REQUIRE(graphs::size(g) == 2);
}

TEST_CASE("graph count CPOs resolve free functions via ADL", "[graph][interface][primitives][counts]")
{
  adl::graph g;
  REQUIRE(graphs::vertex_count(g) == 3);
  REQUIRE(graphs::edge_count(g) == 2);
  REQUIRE(graphs::order(g) == 3);
  REQUIRE(graphs::size(g) == 2);
}

TEST_CASE("graph counts fall back to sized vertex and edge ranges", "[graph][interface][primitives][counts]")
{
  fallback_graph<true> g;
  REQUIRE(graphs::vertex_count(g) == 4);
  REQUIRE(graphs::edge_count(g) == 3);
  REQUIRE(graphs::order(g) == 4);
  REQUIRE(graphs::size(g) == 3);
  static_assert(noexcept(graphs::vertex_count(g)));
  static_assert(noexcept(graphs::edge_count(g)));
}

TEST_CASE("graph count CPO forwards graph value categories", "[graph][interface][primitives][counts]")
{
  ref_qualified_graph g;
  ref_qualified_graph const cg;
  REQUIRE(graphs::vertex_count(g) == 1);
  REQUIRE(graphs::vertex_count(std::move(g)) == 2);
  REQUIRE(graphs::vertex_count(cg) == 3);
  REQUIRE(graphs::vertex_count(std::move(cg)) == 4);
}

TEST_CASE("graph count exceptions are propagated", "[graph][interface][primitives][counts]")
{
  throwing_graph g;
  static_assert(!noexcept(graphs::vertex_count(g)));
  REQUIRE_THROWS_AS(graphs::vertex_count(g), std::runtime_error);
}
