#include "reference.hpp"

#include <sstream>

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra search handles sparse vertex ids and removed edge ids", "[graph][dijkstra][regression]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{9};
  f.add(0, 1, 1);
  f.add(1, 2, 1);
  f.add(0, 3, 20);
  f.add(3, 4, 1);
  f.add(0, 4, 15);
  f.add(4, 6, 2);
  f.add(6, 8, 3);
  f.add(0, 7, 4);
  f.add(7, 8, 1);
  f.g.remove_vertex(at<G>(1));
  f.g.remove_vertex(at<G>(5));
  f.g.remove_edges(at<G>(0), at<G>(3));
  f.add(3, 8, 2);

  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(runtime_distance_of(result, at<G>(8)) == 5);
  REQUIRE(runtime_distance_of(result, at<G>(6)) == 17);
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(8))) == std::vector<unsigned>{8, 7, 0}));

  auto target = hg::dijkstra_to(f.g, at<G>(0), at<G>(8), f.map());
  REQUIRE(target.reached());
  REQUIRE(runtime_target_distance(target) == 5);
  REQUIRE((path_ids<G>(runtime_target_path(target)) == std::vector<unsigned>{8, 7, 0}));
}

TEST_CASE("Dijkstra accepts a genuine single-pass root range", "[graph][dijkstra][regression]")
{
  external_graph<> g{5};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(3, 4);
  std::vector<unsigned> weights{7, 2, 1};
  auto wm = weight_map<external_graph<>::edge_type>{&weights};
  std::istringstream input{"0 3"};
  auto roots = std::ranges::istream_view<int>(input);
  static_assert(std::ranges::input_range<decltype(roots)>);
  static_assert(!std::ranges::forward_range<decltype(roots)>);

  auto result = hg::dijkstra(g, roots, wm);
  REQUIRE(runtime_distance_of(result, 2) == 9);
  REQUIRE(runtime_distance_of(result, 4) == 1);
}

TEST_CASE("Dijkstra_to accepts a genuine single-pass root range", "[graph][dijkstra][regression]")
{
  external_graph<> g{5};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(3, 4);
  std::vector<unsigned> weights{7, 2, 1};
  std::istringstream input{"0 3"};
  auto roots = std::ranges::istream_view<int>(input);
  auto result = hg::dijkstra_to(g, roots, 2, weight_map<external_graph<>::edge_type>{&weights});
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 9);
}

TEST_CASE("Dijkstra preserves representable costs immediately below infinity", "[graph][dijkstra][regression]")
{
  using G = graph<>;
  constexpr unsigned inf = std::numeric_limits<unsigned>::max();
  weighted_graph<G> f{3};
  f.add(0, 1, inf - 9u);
  f.add(1, 2, 8u);

  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  REQUIRE(result.reached(at<G>(1)));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(runtime_distance_of(result, at<G>(2)) == inf - 1u);
}

TEST_CASE("Dijkstra supports saturating path addition without integer overflow", "[graph][dijkstra][regression]")
{
  using G = graph<>;
  constexpr unsigned inf = std::numeric_limits<unsigned>::max();
  weighted_graph<G> f{3};
  f.add(0, 1, inf - 5u);
  f.add(1, 2, 20u);
  f.add(0, 2, 42u);
  auto saturating_add = [](unsigned lhs, unsigned rhs)
  {
    constexpr unsigned infinity = std::numeric_limits<unsigned>::max();
    return rhs > infinity - lhs ? infinity : lhs + rhs;
  };

  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             inf, 0u, std::less<>{}, saturating_add);
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 42u);
}

TEST_CASE("Dijkstra treats the configured infinity as an unreachable sentinel", "[graph][dijkstra][regression]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 99u);
  f.add(1, 2, 1u);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             100u, 0u);
  REQUIRE(result.reached(at<G>(1)));
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 99u);
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 100u);
}

TEST_CASE("Dijkstra stop on relaxation leaves the candidate tentative", "[graph][dijkstra][regression]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  std::unordered_map<V, unsigned> distances;
  std::unordered_map<V, hg::color> colors;
  struct visitor
  {
    hg::visit relaxed_edge(G const &, edge<G>) const { return hg::visit::stop; }
  };
  using DM = external_map<V, unsigned>;
  using CM = external_map<V, hg::color>;
  REQUIRE_FALSE(runtime_visit(f.g, at<G>(0), f.map(), visitor{}, DM{&distances},
                              std::numeric_limits<unsigned>::max(), 0u,
                              std::less<>{}, std::plus<>{}, CM{&colors}));
  REQUIRE(distances.at(at<G>(1)) == 2u);
  REQUIRE(distances.at(at<G>(2)) == std::numeric_limits<unsigned>::max());
  REQUIRE(colors.at(at<G>(1)) == hg::color::white);
}
