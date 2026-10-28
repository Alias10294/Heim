#include "../reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra distance observers return exact edge weights", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 9);
  f.add(1, 2, 4);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(runtime_distance_of(result, at<G>(0)) == 0);
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 9);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 13);
}

TEST_CASE("Dijkstra unreachable distances are infinite", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE(runtime_distance_of(result, at<G>(2)) == std::numeric_limits<unsigned>::max());
}

TEST_CASE("Dijkstra predecessor observer follows relaxed edges", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 9);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 2);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(runtime_predecessor_of(result, at<G>(1)) == at<G>(2));
  REQUIRE(runtime_predecessor_of(result, at<G>(3)) == at<G>(1));
}

TEST_CASE("Dijkstra_to target observers are usable after completion", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 3);
  f.add(1, 2, 4);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(2), f.map());
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 7);
}

TEST_CASE("Dijkstra_to unreachable target exposes infinity", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(2), f.map());
  REQUIRE_FALSE(result.reached());
  REQUIRE(runtime_target_distance(result) == std::numeric_limits<unsigned>::max());
}

TEST_CASE("Dijkstra aliases preserve result semantics", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  f.add(0, 1, 4);
  auto result = hg::dijkstra_search(f.g, at<G>(0), f.map());
  auto target = hg::dijkstra_search_to(f.g, at<G>(0), at<G>(1), f.map());
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 4);
  REQUIRE(runtime_target_distance(target) == 4);
}
