#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological orders handle erased vertices and edge identifiers", "[graph][topo][regression]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 8);
  add_edges(g, {{0, 1}, {1, 2}, {0, 3}, {3, 4}, {4, 7}, {6, 7}});
  g.remove_vertex(at<G>(1));
  g.remove_vertex(at<G>(5));
  g.remove_edges(at<G>(0), at<G>(3));
  add_edges(g, {{0, 4}});

  auto verify_order = [&](auto &&order, bool reverse)
  {
    std::unordered_map<unsigned, std::size_t> positions;
    for (auto v : order)
    {
      unsigned const id = static_cast<unsigned>(v.id());
      REQUIRE(positions.emplace(id, positions.size()).second);
    }
    REQUIRE(positions.size() == g.vertex_count());
    REQUIRE_FALSE(positions.contains(1));
    REQUIRE_FALSE(positions.contains(5));
    for (auto edge : g.edges())
    {
      unsigned const u = static_cast<unsigned>(g.source(edge).id());
      unsigned const v = static_cast<unsigned>(g.target(edge).id());
      if (reverse)
        REQUIRE(positions.at(v) < positions.at(u));
      else
        REQUIRE(positions.at(u) < positions.at(v));
    }
  };

  verify_order(runtime_topological_sort(g), false);
  std::vector<vertex<G>> reverse;
  runtime_reverse_topological_sort(g, std::back_inserter(reverse));
  verify_order(reverse, true);
}

TEST_CASE("Topological sort ignores cycles removed by vertex deletion", "[graph][topo][regression]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 0}, {0, 3}});
  g.remove_vertex(at<G>(1));
  auto order = runtime_topological_sort(g);
  REQUIRE((ids<G>(order).size() == 3));
}
