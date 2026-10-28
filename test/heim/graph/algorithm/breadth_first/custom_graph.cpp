#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS visit traverses an external graph with integer descriptors", "[graph][bfs][custom]")
{
  external_graph g = make_external_graph();
  std::set<int> seen;
  struct visitor
  {
    std::set<int> *seen;
    void discover_vertex(external_graph const &, int v) const { seen->insert(v); }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&seen}));
  REQUIRE((seen == std::set<int>{0, 1, 2, 3}));
}

TEST_CASE("BFS search computes predecessor and level maps on an external graph", "[graph][bfs][custom]")
{
  external_graph g = make_external_graph();
  auto result = hg::bfs(g, 0);
  REQUIRE(runtime_level_of(result, 0) == 0);
  REQUIRE(runtime_level_of(result, 1) == 1);
  REQUIRE(runtime_level_of(result, 2) == 1);
  REQUIRE(runtime_level_of(result, 3) == 2);
  REQUIRE(runtime_predecessor_of(result, 0) == 0);
  REQUIRE(runtime_predecessor_of(result, 3) == 2);
  std::vector<int> path;
  for (int v : runtime_path_from(result, 3))
    path.push_back(v);
  REQUIRE((path == std::vector<int>{3, 2, 0}));
}

TEST_CASE("BFS_to finds targets on an external graph using ADL-compatible primitives", "[graph][bfs][custom]")
{
  external_graph g = make_external_graph();
  auto result = hg::bfs_to(g, 0, 3);
  REQUIRE(result.reached());
  REQUIRE(runtime_target_level(result) == 2);

  std::vector<int> path;
  for (int v : runtime_target_path(result))
    path.push_back(v);
  REQUIRE((path == std::vector<int>{3, 2, 0}));
}

TEST_CASE("BFS visit reports the expected number of tree edges", "[graph][bfs][custom]")
{
  external_graph g = make_external_graph();
  unsigned trees{};
  unsigned other{};
  struct visitor
  {
    unsigned *trees;
    unsigned *other;
    void tree_edge(external_graph const &, int) const { ++*trees; }
    void non_tree_edge(external_graph const &, int) const { ++*other; }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&trees, &other}));
  REQUIRE(trees == 3);
  REQUIRE(other == 3);
}

TEST_CASE("BFS search with multiple external roots does not traverse unrelated components", "[graph][bfs][custom]")
{
  external_graph g{7};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(4, 5);
  std::array<int, 2> roots{0, 4};

  auto result = hg::bfs(g, roots);
  REQUIRE(result.reached(0));
  REQUIRE(result.reached(2));
  REQUIRE(result.reached(5));
  REQUIRE_FALSE(result.reached(3));
  REQUIRE_FALSE(result.reached(6));
  REQUIRE(runtime_level_of(result, 2) == 2);
  REQUIRE(runtime_level_of(result, 5) == 1);
}
