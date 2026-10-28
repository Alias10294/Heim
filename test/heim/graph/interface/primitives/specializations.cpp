#include <algorithm>
#include <array>
#include <concepts>
#include <span>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/graph/interface/primitives.hpp>

namespace external_graph
{
struct graph
{
  std::array<long, 3> vertices_storage{10, 20, 30};
  std::array<short, 2> edges_storage{3, 4};
  long last_source{};
  long last_target{};
};
constexpr auto vertices(graph const &g) noexcept
{
  return std::span{g.vertices_storage};
}
constexpr auto edges(graph const &g) noexcept
{
  return std::span{g.edges_storage};
}
constexpr long source(graph const &, short) noexcept
{
  return 10;
}
constexpr long target(graph const &, short) noexcept
{
  return 20;
}
constexpr auto place_edge(graph &g, long u, long v) noexcept
{
  g.last_source = u;
  g.last_target = v;
  return std::pair<short, bool>{5, true};
}
}

namespace heim::graphs
{
template <>
struct vertex<external_graph::graph> : std::type_identity<long>
{
};
template <>
struct edge<external_graph::graph> : std::type_identity<short>
{
};
}

static_assert(std::same_as<heim::graphs::vertex_t<external_graph::graph>, long>);
static_assert(std::same_as<heim::graphs::edge_t<external_graph::graph>, short>);
static_assert(std::invocable<decltype(heim::graphs::vertices), external_graph::graph const &>);
static_assert(std::invocable<decltype(heim::graphs::edges), external_graph::graph const &>);
static_assert(std::invocable<decltype(heim::graphs::place_edge), external_graph::graph &, long, long>);
static_assert(
    std::same_as<decltype(heim::graphs::place_edge(std::declval<external_graph::graph &>(), 10L, 20L)),
                 std::pair<short, bool>>);

TEST_CASE(
    "graph CPOs honor specialized vertex and edge traits",
    "[graph][interface][primitives][specialization]")
{
  external_graph::graph g;
  REQUIRE((std::ranges::equal(heim::graphs::vertices(g), std::array{10L, 20L, 30L})));
  REQUIRE((std::ranges::equal(heim::graphs::edges(g), std::array<short, 2>{3, 4})));
  REQUIRE(heim::graphs::source(g, short{3}) == 10L);
  REQUIRE(heim::graphs::target(g, short{3}) == 20L);
  auto [edge, placed] = heim::graphs::place_edge(g, 10L, 30L);
  REQUIRE(placed);
  REQUIRE(edge == short{5});
  REQUIRE(g.last_source == 10L);
  REQUIRE(g.last_target == 30L);
}
