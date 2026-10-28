#include <heim/graph/algorithm/breadth_first/search_to.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BFS_to search header is independently includable", "[graph][bfs][header]")
{
  STATIC_REQUIRE(std::same_as<decltype(heim::graphs::breadth_first_search_to),
                              heim::graphs::detail::bfs::to::fn::algo const>);
}
