#include "support.hpp"

using namespace heim_test::adjacency;

TEST_CASE("adjacency_list places monotonically identified vertices", "[graph][adjacency_list][vertices]")
{
  graph<> g;
  reference_graph ref;
  for (unsigned i = 0; i < 60; ++i)
    add_vertex(g, ref);
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list removing a vertex preserves remaining identifiers",
    "[graph][adjacency_list][vertices]")
{
  graph<> g;
  reference_graph ref;
  auto v = make_vertices(g, 5);
  for (unsigned i = 0; i < 5; ++i)
    ref.add_vertex(i);
  g.remove_vertex(v[2]);
  ref.remove_vertex(2);
  verify(g, ref);
  add_vertex(g, ref);
  verify(g, ref);
}

TEST_CASE("adjacency_list removed vertex id is not reused", "[graph][adjacency_list][vertices]")
{
  graph<> g;
  auto vs = make_vertices(g, 7);
  g.remove_vertex(vs[3]);
  auto next = g.place_vertex();
  REQUIRE(next.id() == 7u);
  REQUIRE(next != vs[3]);
}

TEST_CASE(
    "adjacency_list clear is idempotent and retains fresh-id sequence",
    "[graph][adjacency_list][vertices]")
{
  graph<> g;
  make_vertices(g, 3);
  g.clear();
  g.clear();
  verify_empty(g);
  REQUIRE(g.place_vertex().id() == 3u);
}

TEST_CASE("adjacency_list sparse vertex identifiers survive erasure", "[graph][adjacency_list][vertices]")
{
  graph<> g{{{1, 100}, {10, 100}}};
  REQUIRE(g.vertex_count() == 3);
  g.remove_vertex(graph<>::vertex_type{10u});
  REQUIRE((snapshot(g).vertices == std::set<unsigned>{1u, 100u}));
  REQUIRE(g.place_vertex().id() == 101u);
  check_consistency(g);
}

TEST_CASE("adjacency_list vertices view is a range of value descriptors", "[graph][adjacency_list][vertices]")
{
  graph<> g;
  make_vertices(g, 4);
  auto view = g.vertices();
  static_assert(std::ranges::range<decltype(view)>);
  static_assert(std::convertible_to<std::ranges::range_reference_t<decltype(view)>, graph<>::vertex_type>);
  REQUIRE(std::ranges::distance(view) == 4);
}
