#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort accepts an empty graph", "[graph][topo][basic]")
{
  graph<> g;
  auto order = runtime_topological_sort(g);
  REQUIRE(std::ranges::empty(order));
}

TEST_CASE("Reverse topological sort accepts an empty graph", "[graph][topo][basic]")
{
  graph<> g;
  std::vector<vertex<graph<>>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  REQUIRE(output.empty());
}

TEST_CASE("Topological sort accepts one isolated vertex", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 1);
  REQUIRE((ids<G>(runtime_topological_sort(g)) == std::vector<unsigned>{0}));
}

TEST_CASE("Reverse topological sort accepts one isolated vertex", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 1);
  std::vector<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  REQUIRE((ids<G>(output) == std::vector<unsigned>{0}));
}

TEST_CASE("Topological sort respects a directed path", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  REQUIRE((ids<G>(runtime_topological_sort(g)) == std::vector<unsigned>{0, 1, 2, 3, 4}));
}

TEST_CASE("Reverse topological sort respects a directed path", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  std::vector<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  REQUIRE((ids<G>(output) == std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("Topological sort respects a diamond DAG", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 2}, {1, 3}, {2, 3}});
  reference_graph reference{4};
  reference.add(0, 1);
  reference.add(0, 2);
  reference.add(1, 3);
  reference.add(2, 3);
  verify_topological(g, reference);
}

TEST_CASE("Topological sort includes disconnected components and isolates", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 8);
  add_edges(g, {{0, 3}, {1, 2}, {3, 4}, {5, 7}});
  reference_graph reference{8};
  reference.add(0, 3);
  reference.add(1, 2);
  reference.add(3, 4);
  reference.add(5, 7);
  verify_topological(g, reference);
}

TEST_CASE("Topological sort accepts all isolated vertices", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 7);
  reference_graph reference{7};
  verify_topological(g, reference);
}

TEST_CASE("Topological sort is stable in meaning across duplicate edges", "[graph][topo][basic]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 1}, {0, 2}, {1, 3}, {1, 3}, {2, 3}});
  reference_graph reference{4};
  reference.add(0, 1);
  reference.add(0, 1);
  reference.add(0, 2);
  reference.add(1, 3);
  reference.add(1, 3);
  reference.add(2, 3);
  verify_topological(g, reference);
}

TEST_CASE("Topological sort leaves the input graph unchanged", "[graph][topo][basic]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {0, 2}, {2, 4}});
  auto const before_vertices = g.vertex_count();
  auto const before_edges = g.edge_count();
  auto result = runtime_topological_sort(g);
  REQUIRE(std::ranges::distance(result) == 5);
  REQUIRE(g.vertex_count() == before_vertices);
  REQUIRE(g.edge_count() == before_edges);
}
