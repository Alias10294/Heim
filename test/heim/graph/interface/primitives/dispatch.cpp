#include "support.hpp"

#include <array>
#include <concepts>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace
{
namespace graphs = heim::graphs;

struct preferred_member
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
  constexpr int vertex_count() const { return 4; }
  constexpr void remove_vertex(int) {}
};
constexpr int vertex_count(preferred_member const &) noexcept
{
  return 99;
}
constexpr void remove_vertex(preferred_member &, int) noexcept
{
}

namespace adl_throw
{
struct graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
};
inline int vertex_count(graph const &)
{
  throw std::runtime_error{"adl"};
}
inline void remove_edge(graph &, int)
{
  throw std::runtime_error{"adl edge"};
}
}

struct private_member
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;

private:
  constexpr int vertex_count() const noexcept { return 99; }

public:
  friend constexpr int vertex_count(private_member const &) noexcept { return 5; }
};

struct qualified_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
  constexpr int place_vertex() & noexcept { return 1; }
  constexpr int place_vertex() && noexcept { return 2; }
};

struct invalid_range_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
  constexpr auto vertices() const noexcept { return std::array{std::pair{1, 2}}; }
  constexpr auto edges() const noexcept { return std::array{std::pair{1, 2}}; }
  constexpr int place_edge(int, int) const noexcept { return 1; }
};

static_assert(!noexcept(graphs::vertex_count(std::declval<preferred_member const &>())));
static_assert(!noexcept(graphs::remove_vertex(std::declval<preferred_member &>(), 0)));
static_assert(noexcept(graphs::vertex_count(std::declval<private_member const &>())));
static_assert(!noexcept(graphs::vertex_count(std::declval<adl_throw::graph const &>())));
static_assert(std::invocable<decltype(graphs::place_vertex), qualified_graph &>);
static_assert(std::invocable<decltype(graphs::place_vertex), qualified_graph &&>);
static_assert(!std::invocable<decltype(graphs::place_vertex), qualified_graph const &>);
static_assert(!std::invocable<decltype(graphs::vertices), invalid_range_graph const &>);
static_assert(!std::invocable<decltype(graphs::edges), invalid_range_graph const &>);
static_assert(!std::invocable<decltype(graphs::place_edge), invalid_range_graph const &, int, int>);
static_assert(
    []
    {
      qualified_graph g;
      return graphs::place_vertex(g) == 1 && graphs::place_vertex(std::move(g)) == 2;
    }());
}

TEST_CASE(
    "graph CPO does not substitute a noexcept ADL overload for a viable throwing member",
    "[graph][interface][primitives][dispatch]")
{
  preferred_member g;
  REQUIRE(graphs::vertex_count(g) == 4);
  graphs::remove_vertex(g, 0);
}

TEST_CASE("graph CPO can use ADL when member is inaccessible", "[graph][interface][primitives][dispatch]")
{
  private_member g;
  REQUIRE(graphs::vertex_count(g) == 5);
}

TEST_CASE("graph CPO propagates ADL exceptions", "[graph][interface][primitives][dispatch]")
{
  adl_throw::graph g;
  REQUIRE_THROWS_AS(graphs::vertex_count(g), std::runtime_error);
  REQUIRE_THROWS_AS(graphs::remove_edge(g, 0), std::runtime_error);
}

TEST_CASE(
    "graph mutating CPOs preserve lvalue and rvalue categories",
    "[graph][interface][primitives][dispatch]")
{
  qualified_graph g;
  REQUIRE(graphs::place_vertex(g) == 1);
  REQUIRE(graphs::place_vertex(std::move(g)) == 2);
}
