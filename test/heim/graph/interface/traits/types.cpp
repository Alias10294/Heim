#include <concepts>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <heim/graph/interface/traits.hpp>

namespace
{
struct normal_graph
{
  using vertex_type = int;
  using edge_type = unsigned;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;
};

struct external_graph
{
};
}

namespace heim::graphs
{
template <>
struct vertex<external_graph> : std::type_identity<long>
{
};
template <>
struct edge<external_graph> : std::type_identity<short>
{
};
template <>
struct is_directed<external_graph> : std::true_type
{
};
template <>
struct is_multigraph<external_graph> : std::true_type
{
};
}

static_assert(std::same_as<heim::graphs::vertex_t<normal_graph>, int>);
static_assert(std::same_as<heim::graphs::edge_t<normal_graph>, unsigned>);
static_assert(std::same_as<typename heim::graphs::vertex<normal_graph>::type, int>);
static_assert(std::same_as<typename heim::graphs::edge<normal_graph>::type, unsigned>);
static_assert(std::same_as<heim::graphs::vertex_t<external_graph>, long>);
static_assert(std::same_as<heim::graphs::edge_t<external_graph>, short>);
static_assert(heim::graphs::directed<normal_graph>);
static_assert(!heim::graphs::multigraph<normal_graph>);
static_assert(heim::graphs::directed<external_graph>);
static_assert(heim::graphs::multigraph<external_graph>);

TEST_CASE("graph type traits support nested types and external specialization", "[graph][interface][traits]")
{
  REQUIRE(heim::graphs::is_directed<normal_graph>::value);
  REQUIRE_FALSE(heim::graphs::is_multigraph<normal_graph>::value);
  REQUIRE(heim::graphs::is_directed<external_graph>::value);
  REQUIRE(heim::graphs::is_multigraph<external_graph>::value);
}
