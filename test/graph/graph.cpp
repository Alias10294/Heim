#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>

TEST_CASE("graph insertion updates vertex and edge counts", "[graph]")
{
  heim::adjacency_list<> graph;
  auto const a = heim::graphs::place_vertex(graph);
  auto const b = heim::graphs::place_vertex(graph);

  REQUIRE(heim::graphs::vertex_count(graph) == 2);
  REQUIRE(heim::graphs::edge_count(graph) == 0);

  heim::graphs::place_edge(graph, a, b);
  REQUIRE(heim::graphs::edge_count(graph) == 1);
}

TEST_CASE("breadth-first search reaches a connected vertex", "[graph][bfs]")
{
  heim::adjacency_list<> graph{{0, 1}, {1, 2}};
  auto const result = heim::graphs::breadth_first_search(graph, 0);

  REQUIRE(result.reached(2));
  REQUIRE(result.level_of(2) == 2);
}
