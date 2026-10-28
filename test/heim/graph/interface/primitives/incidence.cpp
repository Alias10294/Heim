#include "support.hpp"
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

using namespace heim_test::graph_interface;

namespace
{
struct outgoing_only_directed
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;
  std::array<int, 1> edges{1};
  constexpr auto out_edges(int) const noexcept { return std::span{edges}; }
};
}
static_assert(!std::invocable<decltype(graphs::total_edge_count), outgoing_only_directed const &, int>);

static_assert(noexcept(graphs::out_edges(std::declval<member_graph const &>(), 0)));
static_assert(noexcept(graphs::in_edges(std::declval<adl::graph const &>(), 0)));
static_assert(!std::invocable<decltype(graphs::out_edges), empty_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::in_edges), empty_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::out_edge_count), empty_graph const &, int>);
static_assert(!std::invocable<decltype(graphs::in_edge_count), empty_graph const &, int>);

TEST_CASE("incident edge ranges prefer members and preserve graph IDs", "[graph][interface][primitives][incidence]")
{
  member_graph g;
  REQUIRE((std::ranges::equal(graphs::out_edges(g, 0), std::array{10, 11})));
  REQUIRE((std::ranges::equal(graphs::in_edges(g, 1), std::array{11})));
  REQUIRE(graphs::out_edge_count(g, 0) == 2);
  REQUIRE(graphs::in_edge_count(g, 1) == 1);
  REQUIRE(graphs::out_degree(g, 0) == 2);
  REQUIRE(graphs::in_degree(g, 1) == 1);
}

TEST_CASE("incident edge ranges and degrees resolve through ADL", "[graph][interface][primitives][incidence]")
{
  adl::graph g;
  REQUIRE((std::ranges::equal(graphs::out_edges(g, 0), std::array{12})));
  REQUIRE((std::ranges::equal(graphs::in_edges(g, 1), std::array{13})));
  REQUIRE(graphs::out_edge_count(g, 0) == 1);
  REQUIRE(graphs::in_edge_count(g, 1) == 1);
}

TEST_CASE("incident edge counts fall back to range sizes", "[graph][interface][primitives][incidence]")
{
  fallback_graph<true> g;
  REQUIRE(graphs::out_edge_count(g, 0) == 2);
  REQUIRE(graphs::in_edge_count(g, 0) == 1);
  REQUIRE(graphs::out_degree(g, 0) == 2);
  REQUIRE(graphs::in_degree(g, 0) == 1);
  static_assert(noexcept(graphs::out_edge_count(g, 0)));
  static_assert(noexcept(graphs::in_edge_count(g, 0)));
}

TEST_CASE("total degree prefers explicit graph customization", "[graph][interface][primitives][incidence]")
{
  member_graph m;
  adl::graph a;
  REQUIRE(graphs::total_edge_count(m, 0) == 3);
  REQUIRE(graphs::total_edge_count(a, 0) == 2);
  REQUIRE(graphs::degree(m, 0) == 3);
}

TEST_CASE("directed total degree falls back to incoming plus outgoing degree", "[graph][interface][primitives][incidence]")
{
  fallback_graph<true> g;
  REQUIRE(graphs::total_edge_count(g, 0) == 3);
  REQUIRE(graphs::degree(g, 0) == 3);
  static_assert(noexcept(graphs::total_edge_count(g, 0)));
}

TEST_CASE("undirected total degree prefers outgoing count fallback", "[graph][interface][primitives][incidence]")
{
  fallback_graph<false> g;
  REQUIRE(graphs::total_edge_count(g, 0) == 2);
  REQUIRE(graphs::degree(g, 0) == 2);
}

TEST_CASE("undirected total degree can fall back to incoming count alone", "[graph][interface][primitives][incidence]")
{
  incoming_only_graph g;
  REQUIRE(graphs::total_edge_count(g, 0) == 2);
  REQUIRE(graphs::degree(g, 0) == 2);
}

TEST_CASE("directed total degree is unavailable without both directions", "[graph][interface][primitives][incidence]")
{
  static_assert(!std::invocable<decltype(graphs::total_edge_count), empty_graph &, int>);
}
