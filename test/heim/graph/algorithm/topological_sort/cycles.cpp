#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort rejects a self loop", "[graph][topo][cycle]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 1);
  add_edges(g, {{0, 0}});
  REQUIRE_THROWS_AS(runtime_topological_sort(g), std::logic_error);
}

TEST_CASE("Reverse topological sort rejects a self loop", "[graph][topo][cycle]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 1);
  add_edges(g, {{0, 0}});
  std::vector<vertex<G>> result;
  REQUIRE_THROWS_AS(runtime_reverse_topological_sort(g, std::back_inserter(result)), std::logic_error);
}

TEST_CASE("Topological sort rejects two opposing edges", "[graph][topo][cycle]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}, {1, 0}});
  REQUIRE_THROWS_AS(runtime_topological_sort(g), std::logic_error);
}

TEST_CASE("Reverse topological sort rejects a directed triangle", "[graph][topo][cycle]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 0}});
  std::vector<vertex<G>> result;
  REQUIRE_THROWS_AS(runtime_reverse_topological_sort(g, std::back_inserter(result)), std::logic_error);
}

TEST_CASE("Topological sort detects a cycle in a disconnected component", "[graph][topo][cycle]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {5, 3}});
  REQUIRE_THROWS_AS(runtime_topological_sort(g), std::logic_error);
}

TEST_CASE("Topological sort detects cycles even with parallel edges", "[graph][topo][cycle]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 1}, {1, 2}, {2, 1}, {2, 3}});
  REQUIRE_THROWS_AS(runtime_topological_sort(g), std::logic_error);
}

TEST_CASE("Reverse topological sort propagates the cycle exception to the caller", "[graph][topo][cycle]")
{
  external_graph<true> g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 0);
  std::vector<int> output;
  REQUIRE_THROWS_AS(runtime_reverse_topological_sort(g, std::back_inserter(output)), std::logic_error);
}

TEST_CASE("Duplicate edges alone do not form a directed cycle", "[graph][topo][cycle]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}, {0, 1}, {0, 1}});
  REQUIRE((ids<G>(runtime_topological_sort(g)) == std::vector<unsigned>{0, 1}));
}
