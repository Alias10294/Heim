#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

namespace
{
using Directed = graph<true>;
using Undirected = graph<false>;
using DirectedExternal = external_graph<true>;
using UndirectedExternal = external_graph<false>;
}

TEST_CASE("Topological sort requires a directed graph", "[graph][topo][traits]")
{
  STATIC_REQUIRE(std::invocable<decltype(hg::topological_sort), Directed const &>);
  STATIC_REQUIRE(std::invocable<decltype(hg::topological_sort), DirectedExternal const &>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::topological_sort), Undirected const &>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::topological_sort), UndirectedExternal const &>);
}

TEST_CASE("Reverse topological sort requires directed graph and output iterator", "[graph][topo][traits]")
{
  using V = vertex<Directed>;
  using O = std::back_insert_iterator<std::vector<V>>;
  STATIC_REQUIRE(std::invocable<decltype(hg::reverse_topological_sort), Directed const &, O>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::reverse_topological_sort), Undirected const &, O>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::reverse_topological_sort), Directed const &, int>);
}

TEST_CASE("Topological sort result is a standard input range", "[graph][topo][traits]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  auto order = runtime_topological_sort(g);
  STATIC_REQUIRE(std::ranges::input_range<decltype(order)>);
  REQUIRE(std::ranges::distance(order) == 2);
}
