#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort operates on a CPO-compatible external graph", "[graph][topo][generic]")
{
  external_graph<true> g{6};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 3);
  g.add_edge(2, 3);
  g.add_edge(3, 5);
  reference_graph reference{6};
  reference.add(0, 1);
  reference.add(0, 2);
  reference.add(1, 3);
  reference.add(2, 3);
  reference.add(3, 5);
  reference.verify_order(runtime_topological_sort(g));
}

TEST_CASE("Reverse topological sort operates on an external graph", "[graph][topo][generic]")
{
  external_graph<true> g{5};
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  g.add_edge(0, 4);
  std::vector<int> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  reference_graph reference{5};
  reference.add(1, 2);
  reference.add(2, 3);
  reference.add(0, 4);
  reference.verify_order(output, true);
}

TEST_CASE("Topological sort accepts parallel edges in an external graph", "[graph][topo][generic]")
{
  external_graph<true> g{3};
  g.add_edge(0, 1);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  auto order = runtime_topological_sort(g);
  REQUIRE((std::vector<int>(order.begin(), order.end()) == std::vector<int>{0, 1, 2}));
}

TEST_CASE("Topological sort of an external isolated graph includes every vertex", "[graph][topo][generic]")
{
  external_graph<true> g{6};
  reference_graph reference{6};
  reference.verify_order(runtime_topological_sort(g));
}

TEST_CASE("Topological sort of an external graph detects a self loop", "[graph][topo][generic]")
{
  external_graph<true> g{3};
  g.add_edge(1, 1);
  REQUIRE_THROWS_AS(runtime_topological_sort(g), std::logic_error);
}
