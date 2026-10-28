#include <heim/graph/interface/primitives.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <concepts>
#include <type_traits>

namespace
{
struct graph
{
  using vertex_type = unsigned;
  using edge_type = unsigned;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
  constexpr auto vertices() const noexcept { return std::array{1u, 2u}; }
};
static_assert(std::invocable<decltype(heim::graphs::vertices), graph const &>);
static_assert(std::same_as<heim::graphs::vertex_t<graph>, unsigned>);
}
TEST_CASE("graph primitives header is independently includable", "[graph][interface][headers]")
{
  graph g;
  REQUIRE(heim::graphs::vertex_count(g) == 2);
}
