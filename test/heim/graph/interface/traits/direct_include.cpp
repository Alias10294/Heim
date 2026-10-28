#include <heim/graph/interface/traits.hpp>
#include <catch2/catch_test_macros.hpp>

#include <concepts>

namespace
{
struct graph
{
  using vertex_type = char;
  using edge_type = long;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = true;
};
static_assert(std::same_as<heim::graphs::vertex_t<graph>, char>);
static_assert(std::same_as<heim::graphs::edge_t<graph>, long>);
}
TEST_CASE("graph traits header is independently includable", "[graph][interface][headers]")
{
  REQUIRE_FALSE(heim::graphs::directed<graph>);
  REQUIRE(heim::graphs::multigraph<graph>);
}
