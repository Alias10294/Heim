#include "support.hpp"

#include <sstream>

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS visit accepts an empty graph and an empty root range", "[graph][bfs][visit]")
{
  external_graph g{0};
  std::vector<int> roots;
  REQUIRE(hg::breadth_first_visit(g, roots, [](auto &&...) {}));
}

TEST_CASE("BFS visit accepts an empty root range with existing vertices", "[graph][bfs][visit]")
{
  external_graph g{3};
  unsigned initialized{};
  struct visitor
  {
    unsigned *initialized;
    void initialize_vertex(external_graph const &, int) const { ++*initialized; }
  };

  std::vector<int> const roots;
  REQUIRE(hg::breadth_first_visit(g, roots, visitor{&initialized}));
  REQUIRE(initialized == 3);
}

TEST_CASE("BFS visit accepts a visitor providing only discover_vertex", "[graph][bfs][visit]")
{
  external_graph g{4};
  g.add_edge(0, 1);
  g.add_edge(1, 2);

  struct visitor
  {
    std::vector<int> *discovered;
    void discover_vertex(external_graph const &, int v) const { discovered->push_back(v); }
  };

  std::vector<int> discovered;
  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&discovered}));
  REQUIRE((discovered == std::vector<int>{0, 1, 2}));
}

TEST_CASE("BFS visit accepts a genuinely single-pass range of roots", "[graph][bfs][visit]")
{
  external_graph g{4};
  g.add_edge(0, 1);
  g.add_edge(2, 3);

  struct visitor
  {
    std::set<int> *discovered;
    void discover_vertex(external_graph const &, int v) const { discovered->insert(v); }
  };

  std::set<int> discovered;
  std::istringstream input{"0 2"};
  auto roots = std::ranges::istream_view<int>(input);
  REQUIRE(hg::breadth_first_visit(g, roots, visitor{&discovered}));
  REQUIRE((discovered == std::set<int>{0, 1, 2, 3}));
}

TEST_CASE("BFS visit does not process an already discovered duplicate root", "[graph][bfs][visit]")
{
  external_graph g{3};
  g.add_edge(0, 1);

  struct visitor
  {
    unsigned *sources;
    void source_vertex(external_graph const &, int) const { ++*sources; }
  };

  unsigned sources{};
  std::array<int, 4> roots{0, 0, 1, 1};
  REQUIRE(hg::breadth_first_visit(g, roots, visitor{&sources}));
  REQUIRE(sources == 2);
}

TEST_CASE("BFS visit works with an undirected graph containing loops", "[graph][bfs][visit]")
{
  graph<false, true> g;
  auto vertices = make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {1, 2}, {2, 2}});

  struct visitor
  {
    std::set<unsigned> *discovered;
    void discover_vertex(graph<false, true> const &, vertex<graph<false, true>> v) const
    { discovered->insert(v.id()); }
  };

  std::set<unsigned> discovered;
  REQUIRE(hg::breadth_first_visit(g, vertices[0], visitor{&discovered}));
  REQUIRE((discovered == std::set<unsigned>{0, 1, 2}));
}

TEST_CASE("BFS visit can use a movable-only visitor", "[graph][bfs][visit]")
{
  external_graph g{2};
  g.add_edge(0, 1);

  struct visitor
  {
    std::unique_ptr<unsigned> count = std::make_unique<unsigned>();
    visitor() = default;
    visitor(visitor &&) = default;
    visitor(visitor const &) = delete;
    void discover_vertex(external_graph const &, int) { ++*count; }
  };

  visitor vis;
  REQUIRE(hg::breadth_first_visit(g, 0, vis));
  REQUIRE(*vis.count == 2);
}
