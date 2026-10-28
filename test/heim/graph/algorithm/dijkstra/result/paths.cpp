#include "../reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra path_from orders a path from target back to root", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(1, 2, 2);
  f.add(2, 3, 3);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(3))) == std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("Dijkstra path_from of root includes root only", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  auto result = hg::dijkstra(f.g, at<G>(1), f.map());
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(1))) == std::vector<unsigned>{1}));
}

TEST_CASE("Dijkstra_to path preserves the cheapest chain", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 3, 50);
  f.add(0, 1, 1);
  f.add(1, 2, 2);
  f.add(2, 3, 3);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("Dijkstra_to root path contains a single vertex", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(0), f.map());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{0}));
}

TEST_CASE("Dijkstra path allows iterating several times", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 1);
  f.add(1, 2, 1);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  auto path = runtime_path_from(result, at<G>(2));
  REQUIRE((path_ids<G>(path) == std::vector<unsigned>{2, 1, 0}));
  REQUIRE((path_ids<G>(path) == std::vector<unsigned>{2, 1, 0}));
}
