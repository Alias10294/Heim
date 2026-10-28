#include "../reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra result can be constructed independently", "[graph][dijkstra][result]")
{
  using G = graph<>;
  using WM = weight_map<edge<G>>;
  hg::dijkstra_result<G, WM> result{std::in_place_type<G>, std::in_place_type<WM>};
  (void)result;
}

TEST_CASE("Dijkstra_to result can be constructed independently", "[graph][dijkstra][result]")
{
  using G = graph<>;
  using WM = weight_map<edge<G>>;
  hg::dijkstra_to_result<G, WM> result{std::in_place_type<G>, at<G>(2), std::in_place_type<WM>};
  (void)result;
}

TEST_CASE("Dijkstra result can be populated after construction", "[graph][dijkstra][result]")
{
  using G = graph<>;
  using WM = weight_map<edge<G>>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  hg::dijkstra_result<G, WM> result{std::in_place_type<G>, std::in_place_type<WM>};
  hg::dijkstra(f.g, result, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra_to result can be populated after construction", "[graph][dijkstra][result]")
{
  using G = graph<>;
  using WM = weight_map<edge<G>>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  hg::dijkstra_to_result<G, WM> result{std::in_place_type<G>, at<G>(2), std::in_place_type<WM>};
  hg::dijkstra_to(f.g, result, at<G>(0), f.map());
  verify_target<G>(result, f.reference, {0}, 2);
}

TEST_CASE("Dijkstra result retains data after move construction", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  auto original = hg::dijkstra(f.g, at<G>(0), f.map());
  auto moved = std::move(original);
  verify_search<G>(moved, f.reference, {0});
}

TEST_CASE("Dijkstra_to result retains data after move construction", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  auto original = hg::dijkstra_to(f.g, at<G>(0), at<G>(2), f.map());
  auto moved = std::move(original);
  verify_target<G>(moved, f.reference, {0}, 2);
}

TEST_CASE("Independent Dijkstra results do not share owned storage", "[graph][dijkstra][result]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 3);
  f.add(2, 3, 5);
  auto a = hg::dijkstra(f.g, at<G>(0), f.map());
  auto b = hg::dijkstra(f.g, at<G>(2), f.map());
  REQUIRE(a.reached(at<G>(1)));
  REQUIRE_FALSE(b.reached(at<G>(1)));
  REQUIRE_FALSE(a.reached(at<G>(3)));
  REQUIRE(b.reached(at<G>(3)));
}
