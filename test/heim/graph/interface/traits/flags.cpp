#include <heim/graph/interface/traits.hpp>
#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <type_traits>

namespace
{
template<bool Directed, bool Multigraph>
struct graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = Directed;
  static constexpr bool is_multigraph = Multigraph;
};
}

static_assert(!heim::graphs::directed<graph<false, false>>);
static_assert(!heim::graphs::directed<graph<false, true>>);
static_assert( heim::graphs::directed<graph<true, false>>);
static_assert( heim::graphs::directed<graph<true, true>>);
static_assert(!heim::graphs::multigraph<graph<false, false>>);
static_assert( heim::graphs::multigraph<graph<false, true>>);
static_assert(!heim::graphs::multigraph<graph<true, false>>);
static_assert( heim::graphs::multigraph<graph<true, true>>);
static_assert(heim::graphs::is_directed_v<graph<true, false>>);
static_assert(!heim::graphs::is_directed_v<graph<false, true>>);
static_assert(heim::graphs::is_multigraph_v<graph<false, true>>);
static_assert(!heim::graphs::is_multigraph_v<graph<true, false>>);
static_assert(std::is_base_of_v<std::true_type, heim::graphs::is_directed<graph<true, false>>>);
static_assert(std::is_base_of_v<std::false_type, heim::graphs::is_multigraph<graph<true, false>>>);

TEST_CASE("graph trait flags describe all orientation/multiplicity combinations", "[graph][interface][traits]")
{
  REQUIRE_FALSE((heim::graphs::is_directed_v<graph<false, false>>));
  REQUIRE_FALSE((heim::graphs::is_multigraph_v<graph<false, false>>));
  REQUIRE((heim::graphs::is_multigraph_v<graph<false, true>>));
  REQUIRE((heim::graphs::is_directed_v<graph<true, false>>));
  REQUIRE((heim::graphs::is_directed_v<graph<true, true>>));
  REQUIRE((heim::graphs::is_multigraph_v<graph<true, true>>));
}
