#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include <ranges>

namespace hg = heim::graphs;

TEST_CASE("undirected graph self-loop occupies one edge slot", "[graph][adjacency_list][self-loop]")
{
  heim::adjacency_list<> graph;
  auto a = hg::place_vertex(graph);
  auto [edge, inserted] = hg::place_edge(graph, a, a);
  REQUIRE(inserted);
  REQUIRE(hg::edge_count(graph) == 1);
  REQUIRE_FALSE(hg::place_edge(graph, a, a).second);
  REQUIRE(hg::edge_count(graph) == 1);
  hg::remove_edge(graph, edge);
  REQUIRE(hg::edge_count(graph) == 0);
  REQUIRE(hg::vertex_count(graph) == 1);
}

TEST_CASE("directed graph keeps opposite edges independently", "[graph][adjacency_list][directed]")
{
  heim::adjacency_list<true> graph{{0u, 1u}, {1u, 0u}};
  REQUIRE(hg::edge_count(graph) == 2);
  REQUIRE(hg::in_edge_count(graph, 0u) == 1);
  REQUIRE(hg::out_edge_count(graph, 0u) == 1);
  hg::remove_edges(graph, 0u, 1u);
  REQUIRE(hg::edge_count(graph) == 1);
  REQUIRE(hg::out_edge_count(graph, 0u) == 0);
  REQUIRE(hg::in_edge_count(graph, 0u) == 1);
}
