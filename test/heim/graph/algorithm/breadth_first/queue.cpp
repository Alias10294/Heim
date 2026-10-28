#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS visit works with a queue exposing only push, pop, front and empty", "[graph][bfs][queue]")
{
  external_graph g{5};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 3);
  g.add_edge(2, 4);

  tracking_queue<int>::statistics stats;
  tracking_queue<int> queue{stats};
  std::set<int> discovered;
  struct visitor
  {
    std::set<int> *values;
    void discover_vertex(external_graph const &, int v) const { values->insert(v); }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&discovered}, hg::default_property_map, queue));
  REQUIRE((discovered == std::set<int>{0, 1, 2, 3, 4}));
  REQUIRE(stats.pushes == 5);
  REQUIRE(stats.pops == 5);
  REQUIRE(stats.front_reads == 5);
  REQUIRE(stats.maximum >= 2);
  REQUIRE(queue.empty());
}

TEST_CASE("BFS search forwards a custom queue to traversal", "[graph][bfs][queue]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 2}, {2, 3}});

  tracking_queue<vertex<G>>::statistics stats;
  tracking_queue<vertex<G>> queue{stats};
  auto result = hg::bfs(g, at<G>(0),
                        hg::default_property_map, hg::default_property_map,
                        hg::default_property_map, hg::default_property_map, queue);

  REQUIRE(result.level_of(at<G>(3)) == 2);
  REQUIRE(stats.pushes == 4);
  REQUIRE(stats.pops == 4);
  REQUIRE(queue.empty());
}

TEST_CASE("BFS_to accepts a custom queue and can finish before draining it", "[graph][bfs][queue]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 2}, {1, 3}});

  tracking_queue<vertex<G>>::statistics stats;
  tracking_queue<vertex<G>> queue{stats};
  auto result = hg::bfs_to(g, at<G>(0), at<G>(1),
                           hg::default_property_map, hg::default_property_map,
                           hg::default_property_map, hg::default_property_map, queue);

  REQUIRE(result.reached());
  REQUIRE(result.level() == 1);
  REQUIRE(stats.pushes == 1);
  REQUIRE(stats.pops == 1);
}

TEST_CASE("BFS visit supports a temporary queue object", "[graph][bfs][queue]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  tracking_queue<int>::statistics stats;

  REQUIRE(hg::breadth_first_visit(g, 0, std::monostate{}, hg::default_property_map,
                                  tracking_queue<int>{stats}));
  REQUIRE(stats.pushes == 3);
  REQUIRE(stats.pops == 3);
}
