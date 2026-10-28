#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Connected components accept an external undirected graph", "[graph][cc][generic]")
{
  external_graph<false> g{7};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(3, 4);
  g.add_edge(4, 3);
  g.add_edge(5, 5);
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 4);
  REQUIRE(result.connected(0, 2));
  REQUIRE(result.connected(3, 4));
  REQUIRE_FALSE(result.connected(2, 3));
  REQUIRE_FALSE(result.connected(5, 6));
}

TEST_CASE("Connected components support external graph and external component map", "[graph][cc][generic]")
{
  external_graph<false> g{5};
  g.add_edge(0, 2);
  g.add_edge(2, 4);
  std::unordered_map<int, std::size_t> ids;
  auto result = hg::cc(g, external_map<int, std::size_t>{&ids});
  REQUIRE(result.component_count() == 3);
  REQUIRE(ids.size() == 5);
  REQUIRE(ids.at(0) == ids.at(4));
  REQUIRE(ids.at(0) != ids.at(1));
}

TEST_CASE("Connected components accept external multigraph parallel edges and loops", "[graph][cc][generic]")
{
  external_graph<false> g{4};
  g.add_edge(0, 1);
  g.add_edge(0, 1);
  g.add_edge(1, 0);
  g.add_edge(1, 1);
  g.add_edge(2, 3);
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 2);
  REQUIRE(result.connected(0, 1));
  REQUIRE(result.connected(2, 3));
  REQUIRE_FALSE(result.connected(1, 3));
}
