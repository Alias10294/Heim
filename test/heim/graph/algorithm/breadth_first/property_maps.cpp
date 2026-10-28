#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS search writes externally owned predecessor, level and discovery maps", "[graph][bfs][property]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, V> predecessors;
  std::unordered_map<V, unsigned> levels;
  std::unordered_map<V, std::size_t> times;

  auto pm = external_map<V, V>{&predecessors};
  auto lm = external_map<V, unsigned>{&levels};
  auto tm = external_map<V, std::size_t>{&times};
  auto result = hg::bfs(g, at<G>(0), pm, lm, tm);

  REQUIRE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));
  REQUIRE(predecessors.at(at<G>(0)) == at<G>(0));
  REQUIRE(predecessors.at(at<G>(1)) == at<G>(0));
  REQUIRE(predecessors.at(at<G>(2)) == at<G>(1));
  REQUIRE(levels.at(at<G>(2)) == 2u);
  REQUIRE(result.level_of(at<G>(2)) == 2u);
  REQUIRE(times.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
  REQUIRE(times.at(at<G>(0)) == 1u);
}

TEST_CASE("BFS search accepts custom white/gray/black colors", "[graph][bfs][property]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, custom_color> colors;
  auto result = hg::bfs(g, at<G>(0),
                        hg::default_property_map, hg::default_property_map,
                        hg::default_property_map, external_map<V, custom_color>{&colors});

  REQUIRE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));
  REQUIRE(colors.at(at<G>(0)) == custom_color::black);
  REQUIRE(colors.at(at<G>(1)) == custom_color::black);
  REQUIRE(colors.at(at<G>(2)) == custom_color::black);
  REQUIRE(colors.at(at<G>(3)) == custom_color::white);
}

TEST_CASE("BFS visit resets a pre-populated external color map", "[graph][bfs][property]")
{
  external_graph g{3};
  g.add_edge(0, 1);

  std::unordered_map<int, hg::color> colors{
      {0, hg::color::black}, {1, hg::color::gray}, {2, hg::color::black}};
  auto cm = external_map<int, hg::color>{&colors};

  REQUIRE(hg::breadth_first_visit(g, 0, std::monostate{}, cm));
  REQUIRE(colors.at(0) == hg::color::black);
  REQUIRE(colors.at(1) == hg::color::black);
  REQUIRE(colors.at(2) == hg::color::white);
}

TEST_CASE("BFS search supports non-default level value types", "[graph][bfs][property]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, int> levels;
  auto result = hg::bfs(g, at<G>(0), hg::default_property_map,
                        external_map<V, int>{&levels});
  REQUIRE(result.level_of(at<G>(0)) == 0);
  REQUIRE(result.level_of(at<G>(2)) == 2);
  REQUIRE(levels.at(at<G>(2)) == 2);
}

TEST_CASE("BFS search can reuse external maps for a new root", "[graph][bfs][property]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {3, 2}});

  std::unordered_map<V, V> predecessors;
  std::unordered_map<V, std::size_t> levels;
  std::unordered_map<V, std::size_t> times;
  auto pm = external_map<V, V>{&predecessors};
  auto lm = external_map<V, std::size_t>{&levels};
  auto tm = external_map<V, std::size_t>{&times};

  auto first = hg::bfs(g, at<G>(0), pm, lm, tm);
  REQUIRE(first.reached(at<G>(1)));
  auto second = hg::bfs(g, at<G>(3), pm, lm, tm);
  REQUIRE_FALSE(second.reached(at<G>(1)));
  REQUIRE(second.reached(at<G>(2)));
  REQUIRE(second.level_of(at<G>(2)) == 1);
  REQUIRE(times.at(at<G>(1)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS_to supports externally supplied result maps", "[graph][bfs][property]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});

  std::unordered_map<V, V> predecessors;
  std::unordered_map<V, int> levels;
  std::unordered_map<V, std::size_t> times;
  auto result = hg::bfs_to(g, at<G>(0), at<G>(2),
                           external_map<V, V>{&predecessors},
                           external_map<V, int>{&levels},
                           external_map<V, std::size_t>{&times});

  REQUIRE(result.reached());
  REQUIRE(result.level() == 2);
  REQUIRE(levels.at(at<G>(2)) == 2);
  REQUIRE(predecessors.at(at<G>(2)) == at<G>(1));
  REQUIRE((path_ids<G>(result.path()) == std::vector<unsigned>{2, 1, 0}));
  REQUIRE(times.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS visit propagates exceptions from user-supplied color maps", "[graph][bfs][property]")
{
  external_graph g{2};
  struct failing_map
  {
    hg::color get(int) const { return hg::color::white; }
    void set(int, hg::color) const { throw std::runtime_error{"color map failed"}; }
  };

  REQUIRE_THROWS_AS(hg::breadth_first_visit(g, 0, std::monostate{}, failing_map{}), std::runtime_error);
}
