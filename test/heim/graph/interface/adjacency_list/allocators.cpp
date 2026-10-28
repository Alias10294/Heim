#include "allocator_support.hpp"

using namespace heim_test::adjacency;

TEST_CASE(
    "adjacency_list allocator reaches sparse and incident storage",
    "[graph][adjacency_list][allocator]")
{
  auto state = std::make_shared<allocation_state>();
  {
    tracked_graph<> g(typename tracked_graph<>::allocator_type{state, 17});
    REQUIRE(g.get_allocator().id == 17);
    auto v = make_vertices(g, 45);
    for (unsigned i = 1; i < v.size(); ++i)
      g.place_edge(v[0], v[i]);
    check_consistency(g);
    REQUIRE(state->allocations > 0);
    REQUIRE(state->outstanding.size() > 0);
    REQUIRE(state->allocations_by_type.size() >= 3);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list allocator reaches both directed incidence lists",
    "[graph][adjacency_list][allocator]")
{
  auto state = std::make_shared<allocation_state>();
  {
    tracked_graph<false, true, true> g(typename tracked_graph<false, true, true>::allocator_type{state, 21});
    auto v = make_vertices(g, 24);
    for (unsigned i = 1; i < v.size(); ++i)
    {
      g.place_edge(v[0], v[i]);
      g.place_edge(v[i], v[0]);
    }
    check_consistency(g);
    REQUIRE(state->outstanding.size() > 0);
    REQUIRE(state->allocations_by_type.size() >= 3);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list copy with explicit different allocator deep copies",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 2});
    auto vs = make_vertices(a, 4);
    a.place_edge(vs[0], vs[1]);
    a.place_edge(vs[0], vs[2]);
    G b(a, G::allocator_type{state, 3});
    REQUIRE(a == b);
    REQUIRE(b.get_allocator().id == 3);
    b.place_edge(vs[2], vs[3]);
    REQUIRE(a.edge_count() == 2);
    REQUIRE(b.edge_count() == 3);
    check_consistency(a);
    check_consistency(b);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list copy constructor uses allocator select-on-copy behavior",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 7});
    auto v = make_vertices(a, 3);
    a.place_edge(v[0], v[1]);
    G b(a);
    REQUIRE(b.get_allocator().id == a.get_allocator().id);
    REQUIRE(a == b);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list move with equal explicit allocator preserves content",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 4});
    auto v = make_vertices(a, 3);
    a.place_edge(v[0], v[1]);
    a.place_edge(v[1], v[2]);
    auto expected = snapshot(a);
    G b(std::move(a), G::allocator_type{state, 4});
    verify(b, expected);
    REQUIRE(b.get_allocator().id == 4);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list move with distinct explicit allocators preserves content",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 4});
    auto v = make_vertices(a, 3);
    a.place_edge(v[0], v[1]);
    a.place_edge(v[1], v[2]);
    auto expected = snapshot(a);
    G b(std::move(a), G::allocator_type{state, 5});
    verify(b, expected);
    REQUIRE(b.get_allocator().id == 5);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list copy assignment retains nonpropagating allocator",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<false>;
  auto state = std::make_shared<allocation_state>();
  {
    G src(G::allocator_type{state, 2}), dst(G::allocator_type{state, 3});
    auto v = make_vertices(src, 3);
    src.place_edge(v[0], v[1]);
    make_vertices(dst, 2);
    dst = src;
    REQUIRE(dst.get_allocator().id == 3);
    REQUIRE(dst == src);
    check_consistency(dst);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list move assignment retains nonpropagating allocator",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<false>;
  auto state = std::make_shared<allocation_state>();
  {
    G src(G::allocator_type{state, 2}), dst(G::allocator_type{state, 3});
    auto v = make_vertices(src, 3);
    src.place_edge(v[0], v[1]);
    make_vertices(dst, 2);
    auto expected = snapshot(src);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().id == 3);
    verify(dst, expected);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list copy assignment propagates allocator when configured",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<true>;
  auto state = std::make_shared<allocation_state>();
  {
    G src(G::allocator_type{state, 2}), dst(G::allocator_type{state, 3});
    auto v = make_vertices(src, 3);
    src.place_edge(v[0], v[1]);
    make_vertices(dst, 2);
    dst = src;
    REQUIRE(dst.get_allocator().id == 2);
    REQUIRE(dst == src);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list move assignment propagates allocator when configured",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<true>;
  auto state = std::make_shared<allocation_state>();
  {
    G src(G::allocator_type{state, 2}), dst(G::allocator_type{state, 3});
    auto v = make_vertices(src, 3);
    src.place_edge(v[0], v[1]);
    make_vertices(dst, 2);
    auto expected = snapshot(src);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().id == 2);
    verify(dst, expected);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("adjacency_list swap with equal nonpropagating allocators", "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<false>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 2}), b(G::allocator_type{state, 2});
    auto av = make_vertices(a, 3), bv = make_vertices(b, 2);
    a.place_edge(av[0], av[1]);
    b.place_edge(bv[0], bv[1]);
    auto expected_a = snapshot(b), expected_b = snapshot(a);
    a.swap(b);
    verify(a, expected_a);
    verify(b, expected_b);
    REQUIRE(a.get_allocator().id == 2);
    REQUIRE(b.get_allocator().id == 2);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("adjacency_list swap with different propagating allocators", "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<true>;
  auto state = std::make_shared<allocation_state>();
  {
    G a(G::allocator_type{state, 2}), b(G::allocator_type{state, 3});
    auto av = make_vertices(a, 3), bv = make_vertices(b, 2);
    a.place_edge(av[0], av[1]);
    b.place_edge(bv[0], bv[1]);
    auto expected_a = snapshot(b), expected_b = snapshot(a);
    swap(a, b);
    verify(a, expected_a);
    verify(b, expected_b);
    REQUIRE(a.get_allocator().id == 3);
    REQUIRE(b.get_allocator().id == 2);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list allocator remains balanced across repeated growth and erasure",
    "[graph][adjacency_list][allocator]")
{
  using G = tracked_graph<false, true, true>;
  auto state = std::make_shared<allocation_state>();
  {
    G g(G::allocator_type{state, 8});
    auto vs = make_vertices(g, 50);
    for (unsigned i = 0; i < 50; ++i)
    {
      g.place_edge(vs[0], vs[i]);
      g.place_edge(vs[i], vs[0]);
    }
    for (unsigned i = 1; i < 50; i += 2)
      g.clear_vertex(vs[i]);
    for (unsigned i = 1; i < 50; i += 3)
      g.remove_vertex(vs[i]);
    check_consistency(g);
  }
  REQUIRE(state->balanced());
}
