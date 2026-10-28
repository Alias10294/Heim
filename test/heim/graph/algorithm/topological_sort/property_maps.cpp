#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort accepts an external color map", "[graph][topo][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {0, 2}, {2, 3}});
  std::vector<V> output;
  std::unordered_map<V, custom_color> colors;
  auto order = runtime_topological_sort(g, output, external_map<V, custom_color>{&colors});
  reference_graph reference{5};
  reference.add(0, 1);
  reference.add(0, 2);
  reference.add(2, 3);
  reference.verify_order(order);
  REQUIRE(colors.size() == 5);

  for (auto const &[v, color] : colors)
  {
    (void)v;
    REQUIRE(color == custom_color::black);
  }
}

TEST_CASE("Reverse topological sort accepts an external color map", "[graph][topo][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, custom_color> colors;
  std::vector<V> output;
  runtime_reverse_topological_sort(g, std::back_inserter(output), external_map<V, custom_color>{&colors});
  REQUIRE(colors.size() == 4);
  reference_graph reference{4};
  reference.add(0, 1);
  reference.add(1, 2);
  reference.verify_order(output, true);
}

TEST_CASE("Topological sort reinitializes an externally owned color map", "[graph][topo][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, custom_color> colors;
  for (unsigned id = 0; id < 3; ++id)
    colors.emplace(at<G>(id), custom_color::gray);

  std::vector<V> output;
  auto order = runtime_topological_sort(g, output, external_map<V, custom_color>{&colors});
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2}));
  REQUIRE(colors.size() == 3);
  for (auto const &[v, color] : colors)
  {
    (void)v;
    REQUIRE(color == custom_color::black);
  }
}

TEST_CASE("Topological and reverse sort can independently reuse the same color storage", "[graph][topo][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::unordered_map<V, custom_color> colors;
  auto map = external_map<V, custom_color>{&colors};
  std::vector<V> forward;
  std::vector<V> reverse;
  runtime_topological_sort(g, forward, map);
  runtime_reverse_topological_sort(g, std::back_inserter(reverse), map);
  REQUIRE((ids<G>(forward) == std::vector<unsigned>{0, 1, 2, 3}));
  REQUIRE((ids<G>(reverse) == std::vector<unsigned>{3, 2, 1, 0}));
}
