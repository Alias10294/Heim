#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Reverse topological sort writes through a back inserter", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  std::vector<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  REQUIRE((ids<G>(output) == std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("Reverse topological sort writes through a front inserter", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  std::list<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::front_inserter(output));
  REQUIRE((ids<G>(output) == std::vector<unsigned>{0, 1, 2, 3, 4}));
}

TEST_CASE("Reverse topological sort returns the advanced output iterator", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  std::vector<vertex<G>> output(5);
  auto it = runtime_reverse_topological_sort(g, output.begin());
  REQUIRE(it == output.end());
  REQUIRE((ids<G>(output) == std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("Reverse topological sort returns unchanged output iterator for empty graph", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  std::vector<vertex<G>> output(2);
  auto it = runtime_reverse_topological_sort(g, output.begin());
  REQUIRE(it == output.begin());
}

TEST_CASE("Reverse topological sort accepts a deque output iterator", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::deque<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  REQUIRE((ids<G>(output) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("Reverse topological sort supports an ostream iterator", "[graph][topo][iterator]")
{
  external_graph<true> g{4};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  std::ostringstream output;
  runtime_reverse_topological_sort(g, std::ostream_iterator<int>{output, " "});
  REQUIRE(output.str() == "3 2 1 0 ");
}

TEST_CASE("Reverse topological sort writes all vertices exactly once", "[graph][topo][iterator]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 2}, {1, 2}, {3, 4}});
  std::vector<vertex<G>> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output));
  reference_graph reference{6};
  reference.add(0, 2);
  reference.add(1, 2);
  reference.add(3, 4);
  reference.verify_order(output, true);
}
