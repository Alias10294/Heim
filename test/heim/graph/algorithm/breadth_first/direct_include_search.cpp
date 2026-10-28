#include <heim/graph/algorithm/breadth_first/search.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BFS search header is independently includable", "[graph][bfs][header]")
{
  STATIC_REQUIRE(std::same_as<decltype(heim::graphs::breadth_first_search),
                              heim::graphs::detail::bfs::fn::algo const>);
}
