#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra handles a graph with no vertices", "[graph][dijkstra][basic]")
{
  weighted_graph<graph<>> f{0};
  std::array<vertex<graph<>>, 0> roots{};
  auto result = hg::dijkstra(f.g, roots, f.map());
  REQUIRE_FALSE(f.g.vertex_count());
  (void)result;
}

TEST_CASE("Dijkstra root is its own predecessor", "[graph][dijkstra][basic]")
{
  weighted_graph<graph<>> f{1};
  auto result = hg::dijkstra(f.g, at<graph<>>(0), f.map());
  REQUIRE(result.reached(at<graph<>>(0)));
  REQUIRE(runtime_distance_of(result, at<graph<>>(0)) == 0);
  REQUIRE(runtime_predecessor_of(result, at<graph<>>(0)) == at<graph<>>(0));
  REQUIRE((path_ids<graph<>>(runtime_path_from(result, at<graph<>>(0))) == std::vector<unsigned>{0}));
}

TEST_CASE("Dijkstra keeps isolated vertices unreachable", "[graph][dijkstra][basic]")
{
  weighted_graph<graph<>> f{5};
  auto result = hg::dijkstra(f.g, at<graph<>>(2), f.map());
  verify_search<graph<>>(result, f.reference, {2});
}

TEST_CASE("Dijkstra chooses the cheaper of two directed paths", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 1);
  f.add(2, 3, 20);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(runtime_distance_of(result, at<G>(3)) == 3);
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(3))) == std::vector<unsigned>{3, 1, 2, 0}));
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra does not treat incoming edges as outgoing", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(1, 0, 1);
  f.add(1, 2, 2);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra recognizes a zero-cost edge", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 0);
  f.add(1, 2, 0);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 0);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra ignores a positive self-loop", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 0, 4);
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra accepts zero-weight self-loops", "[graph][dijkstra][basic]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{3};
  f.add(0, 0, 0);
  f.add(0, 1, 0);
  f.add(1, 1, 0);
  f.add(1, 2, 1);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra finds the cheapest of parallel edges", "[graph][dijkstra][basic]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{3};
  f.add(0, 1, 30);
  f.add(0, 1, 4);
  f.add(0, 1, 8);
  f.add(1, 2, 2);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 6);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra does not confuse equal priorities", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{5};
  f.add(0, 1, 1);
  f.add(0, 2, 1);
  f.add(1, 3, 1);
  f.add(2, 3, 1);
  f.add(3, 4, 1);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra accepts multiple sources", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{6};
  f.add(0, 2, 15);
  f.add(1, 2, 2);
  f.add(2, 3, 1);
  f.add(4, 5, 1);
  std::array const roots{at<G>(0), at<G>(1)};
  auto result = hg::dijkstra(f.g, roots, f.map());
  verify_search<G>(result, f.reference, {0, 1});
}

TEST_CASE("Dijkstra ignores repeated roots", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 2);
  std::array const roots{at<G>(0), at<G>(0), at<G>(0)};
  auto result = hg::dijkstra(f.g, roots, f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra accepts an empty source range", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 2);
  std::vector<vertex<G>> roots;
  auto result = hg::dijkstra(f.g, roots, f.map());
  verify_search<G>(result, f.reference, {});
}

TEST_CASE("Dijkstra handles disconnected directed components", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{7};
  f.add(0, 1, 3);
  f.add(1, 2, 5);
  f.add(4, 5, 1);
  f.add(5, 6, 1);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra handles an undirected graph", "[graph][dijkstra][basic]")
{
  using G = graph<false>;
  weighted_graph<G> f{5};
  f.add(0, 1, 5);
  f.add(1, 2, 3);
  f.add(2, 3, 1);
  f.add(3, 4, 2);
  f.add(0, 4, 20);
  auto result = hg::dijkstra(f.g, at<G>(4), f.map());
  verify_search<G>(result, f.reference, {4});
}

TEST_CASE("Dijkstra handles an undirected multigraph", "[graph][dijkstra][basic]")
{
  using G = graph<false, true>;
  weighted_graph<G> f{4};
  f.add(0, 1, 9);
  f.add(1, 0, 2);
  f.add(1, 2, 0);
  f.add(2, 3, 1);
  auto result = hg::dijkstra(f.g, at<G>(3), f.map());
  verify_search<G>(result, f.reference, {3});
}

TEST_CASE("Dijkstra works with outgoing-only adjacency storage", "[graph][dijkstra][basic]")
{
  using G = graph<true, false, false, true>;
  weighted_graph<G> f{5};
  f.add(0, 1, 7);
  f.add(0, 2, 2);
  f.add(2, 1, 2);
  f.add(1, 3, 1);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra accepts a single-pass root range", "[graph][dijkstra][basic]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 2, 3);
  f.add(1, 3, 2);
  std::array const sources{at<G>(0), at<G>(1)};
  auto roots = sources | std::views::filter([](auto const &){ return true; });
  auto result = hg::dijkstra(f.g, roots, f.map());
  verify_search<G>(result, f.reference, {0, 1});
}
