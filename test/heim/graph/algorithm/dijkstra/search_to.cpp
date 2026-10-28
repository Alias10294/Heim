#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra_to finds a direct target", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 5);
  f.add(1, 2, 3);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(1), f.map());
  verify_target<G>(result, f.reference, {0}, 1);
}

TEST_CASE("Dijkstra_to finds a target by a cheaper indirect path", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 3, 20);
  f.add(0, 1, 1);
  f.add(1, 2, 2);
  f.add(2, 3, 3);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map());
  REQUIRE(runtime_target_distance(result) == 6);
  verify_target<G>(result, f.reference, {0}, 3);
}

TEST_CASE("Dijkstra_to does not stop when target is first discovered", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{5};
  f.add(0, 4, 90);
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  f.add(2, 4, 1);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(4), f.map());
  REQUIRE(runtime_target_distance(result) == 6);
  verify_target<G>(result, f.reference, {0}, 4);
}

TEST_CASE("Dijkstra_to accepts a root as target", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(1, 2, 1);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(0), f.map());
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 0);
  verify_target<G>(result, f.reference, {0}, 0);
}

TEST_CASE("Dijkstra_to reports unreachable target", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 4);
  f.add(2, 3, 1);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map());
  verify_target<G>(result, f.reference, {0}, 3);
}

TEST_CASE("Dijkstra_to reports unreachable target from empty roots", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 4);
  std::vector<vertex<G>> roots;
  auto result = hg::dijkstra_to(f.g, roots, at<G>(1), f.map());
  verify_target<G>(result, f.reference, {}, 1);
}

TEST_CASE("Dijkstra_to accepts multiple sources", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{5};
  f.add(0, 4, 10);
  f.add(1, 2, 1);
  f.add(2, 4, 1);
  std::array const roots{at<G>(0), at<G>(1)};
  auto result = hg::dijkstra_to(f.g, roots, at<G>(4), f.map());
  verify_target<G>(result, f.reference, {0, 1}, 4);
}

TEST_CASE("Dijkstra_to accepts target among several roots", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{5};
  f.add(0, 1, 4);
  std::array const roots{at<G>(0), at<G>(3)};
  auto result = hg::dijkstra_to(f.g, roots, at<G>(3), f.map());
  verify_target<G>(result, f.reference, {0, 3}, 3);
}

TEST_CASE("Dijkstra_to tolerates repeated source descriptors", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 1);
  f.add(1, 2, 2);
  std::array const roots{at<G>(0), at<G>(0)};
  auto result = hg::dijkstra_to(f.g, roots, at<G>(2), f.map());
  verify_target<G>(result, f.reference, {0}, 2);
}

TEST_CASE("Dijkstra_to accepts undirected graph", "[graph][dijkstra][to]")
{
  using G = graph<false>;
  weighted_graph<G> f{4};
  f.add(0, 1, 2);
  f.add(1, 2, 2);
  f.add(2, 3, 2);
  auto result = hg::dijkstra_to(f.g, at<G>(3), at<G>(0), f.map());
  verify_target<G>(result, f.reference, {3}, 0);
}

TEST_CASE("Dijkstra_to accepts parallel edges", "[graph][dijkstra][to]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{3};
  f.add(0, 1, 60);
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(2), f.map());
  verify_target<G>(result, f.reference, {0}, 2);
}

TEST_CASE("Dijkstra_to accepts zero-cost target path", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 0);
  f.add(1, 2, 0);
  f.add(2, 3, 0);
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map());
  verify_target<G>(result, f.reference, {0}, 3);
}

TEST_CASE("Dijkstra_to does not require completion of the entire search", "[graph][dijkstra][to]")
{
  using G = graph<>;
  weighted_graph<G> f{6};
  f.add(0, 1, 1);
  f.add(0, 2, 100);
  f.add(0, 3, 100);
  f.add(0, 4, 100);
  f.add(0, 5, 100);
  std::unordered_map<vertex<G>, unsigned> distances;
  using DM = external_map<vertex<G>, unsigned>;
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(1), f.map(),
                                hg::default_property_map, DM{&distances});
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 1);
  REQUIRE(distances.at(at<G>(2)) == 100);
}
