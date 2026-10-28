#include "../support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS output result writes to the caller's three property maps", "[graph][bfs][result][external]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, V> predecessors;
  std::unordered_map<V, int> levels;
  std::unordered_map<V, std::size_t> times;
  using PM = external_map<V, V>;
  using LM = external_map<V, int>;
  using TM = external_map<V, std::size_t>;
  hg::bfs_result<G, PM, LM, TM> result{
      std::in_place_type<G>, PM{&predecessors}, LM{&levels}, TM{&times}};

  hg::bfs(g, result, at<G>(0));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(result.level_of(at<G>(2)) == 2);
  REQUIRE(predecessors.at(at<G>(2)) == at<G>(1));
  REQUIRE(levels.at(at<G>(2)) == 2);
  REQUIRE(times.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS_to output result writes to external maps and stops at target", "[graph][bfs][result][external]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  std::unordered_map<V, V> predecessors;
  std::unordered_map<V, unsigned> levels;
  std::unordered_map<V, std::size_t> times;
  using PM = external_map<V, V>;
  using LM = external_map<V, unsigned>;
  using TM = external_map<V, std::size_t>;
  hg::bfs_to_result<G, PM, LM, TM> result{
      std::in_place_type<G>, at<G>(2), PM{&predecessors}, LM{&levels}, TM{&times}};

  hg::bfs_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(result.level() == 2u);
  REQUIRE(times.at(at<G>(2)) == 3u);
  REQUIRE(times.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
  REQUIRE((path_ids<G>(result.path()) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("BFS result with external maps permits independent simultaneous results", "[graph][bfs][result][external]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {3, 2}});

  auto first = hg::bfs(g, at<G>(0));
  auto second = hg::bfs(g, at<G>(3));
  REQUIRE(first.level_of(at<G>(2)) == 2);
  REQUIRE(second.level_of(at<G>(2)) == 1);
  REQUIRE(first.reached(at<G>(1)));
  REQUIRE_FALSE(second.reached(at<G>(1)));
}
