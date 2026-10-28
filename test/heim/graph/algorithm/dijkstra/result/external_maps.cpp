#include "../reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra supplied result preserves externally owned maps", "[graph][dijkstra][result][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  using WM = weight_map<edge<G>>;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  weighted_graph<G> f{4};
  f.add(0, 1, 8);
  f.add(0, 2, 1);
  f.add(2, 1, 2);
  f.add(1, 3, 1);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distance;
  hg::dijkstra_result<G, WM, PM, DM> result{
      std::in_place_type<G>, std::in_place_type<WM>, PM{&parents}, DM{&distance}};
  hg::dijkstra(f.g, result, at<G>(0), f.map());
  REQUIRE(distance.at(at<G>(1)) == 3);
  REQUIRE(parents.at(at<G>(1)) == at<G>(2));
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra_to supplied result preserves externally owned maps", "[graph][dijkstra][result][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  using WM = weight_map<edge<G>>;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  weighted_graph<G> f{4};
  f.add(0, 1, 8);
  f.add(0, 2, 1);
  f.add(2, 1, 2);
  f.add(1, 3, 1);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distance;
  hg::dijkstra_to_result<G, WM, PM, DM> result{
      std::in_place_type<G>, at<G>(3), std::in_place_type<WM>, PM{&parents}, DM{&distance}};
  hg::dijkstra_to(f.g, result, at<G>(0), f.map());
  REQUIRE(distance.at(at<G>(3)) == 4);
  REQUIRE(parents.at(at<G>(3)) == at<G>(1));
  verify_target<G>(result, f.reference, {0}, 3);
}

TEST_CASE("Dijkstra external maps remain valid after moving result", "[graph][dijkstra][result][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distances;
  auto original = hg::dijkstra(f.g, at<G>(0), f.map(), PM{&parents}, DM{&distances});
  auto moved = std::move(original);
  REQUIRE(distances.at(at<G>(2)) == 5);
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  REQUIRE(runtime_distance_of(moved, at<G>(2)) == 5);
}

TEST_CASE("Dijkstra_to externally owned maps support multiple roots", "[graph][dijkstra][result][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  weighted_graph<G> f{4};
  f.add(0, 3, 8);
  f.add(1, 3, 2);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distances;
  std::array const roots{at<G>(0), at<G>(1)};
  auto result = hg::dijkstra_to(f.g, roots, at<G>(3), f.map(), PM{&parents}, DM{&distances});
  REQUIRE(result.reached());
  REQUIRE(distances.at(at<G>(3)) == 2);
  REQUIRE(parents.at(at<G>(3)) == at<G>(1));
}
