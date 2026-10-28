#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra operates on an independent graph with integer vertices", "[graph][dijkstra][generic]")
{
  external_graph<> g{4};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(2, 1);
  g.add_edge(1, 3);
  std::vector<unsigned> weights{10, 1, 2, 3};
  auto map = weight_map<external_graph<>::edge_type>{&weights};
  auto result = hg::dijkstra(g, 0, map);
  REQUIRE(result.reached(3));
  REQUIRE(runtime_distance_of(result, 3) == 6);
  std::vector<int> path;
  for (int v : runtime_path_from(result, 3))
    path.push_back(v);
  REQUIRE((path == std::vector<int>{3, 1, 2, 0}));
}

TEST_CASE("Dijkstra_to operates on independent external graph", "[graph][dijkstra][generic]")
{
  external_graph<> g{4};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(2, 1);
  g.add_edge(1, 3);
  std::vector<unsigned> weights{10, 1, 2, 3};
  auto map = weight_map<external_graph<>::edge_type>{&weights};
  auto result = hg::dijkstra_to(g, 0, 3, map);
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 6);
}

TEST_CASE("Dijkstra handles external graph with unreachable vertices", "[graph][dijkstra][generic]")
{
  external_graph<> g{5};
  g.add_edge(0, 1);
  g.add_edge(3, 4);
  std::vector<unsigned> weights{2, 4};
  auto result = hg::dijkstra(g, 0, weight_map<external_graph<>::edge_type>{&weights});
  REQUIRE(result.reached(0));
  REQUIRE(result.reached(1));
  REQUIRE_FALSE(result.reached(2));
  REQUIRE_FALSE(result.reached(4));
}

TEST_CASE("Dijkstra handles a graph exposing undirected incidences", "[graph][dijkstra][generic]")
{
  external_graph<false> g{4};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  std::vector<unsigned> weights{2, 3, 4};
  auto result = hg::dijkstra(g, 3, weight_map<external_graph<false>::edge_type>{&weights});
  REQUIRE(runtime_distance_of(result, 0) == 9);
}

TEST_CASE("Dijkstra accepts different root descriptors from independent graph", "[graph][dijkstra][generic]")
{
  external_graph<> g{4};
  g.add_edge(0, 2);
  g.add_edge(1, 3);
  std::vector<unsigned> weights{3, 4};
  std::array roots{0, 1};
  auto result = hg::dijkstra(g, roots, weight_map<external_graph<>::edge_type>{&weights});
  REQUIRE(runtime_distance_of(result, 2) == 3);
  REQUIRE(runtime_distance_of(result, 3) == 4);
}

TEST_CASE("Dijkstra visit supports an independent external graph", "[graph][dijkstra][generic]")
{
  external_graph<> g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  std::vector<unsigned> weights{2, 5};
  std::unordered_map<int, unsigned> distance;
  using DM = external_map<int, unsigned>;
  struct visitor
  {
    unsigned *settled;
    void finish_vertex(external_graph<> const &, int) const
    { ++*settled; }
  };
  unsigned settled{};
  REQUIRE(runtime_visit(g, 0, weight_map<external_graph<>::edge_type>{&weights},
                             visitor{&settled}, DM{&distance}));
  REQUIRE(settled == 3);
  REQUIRE(distance.at(2) == 7);
}
