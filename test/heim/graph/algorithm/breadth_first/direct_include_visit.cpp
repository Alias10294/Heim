#include <heim/graph/algorithm/breadth_first/visit.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BFS visit header is independently includable", "[graph][bfs][header]")
{
  STATIC_REQUIRE(std::same_as<decltype(heim::graphs::breadth_first_visit),
                              heim::graphs::detail::bfs::vis::fn::algo const>);
}
