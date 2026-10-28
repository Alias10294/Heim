#include "allocator_support.hpp"

#include <limits>

using namespace heim_test::adjacency;

// Measure successful allocations first, then fail each one in an independent
// graph instance. On failure the entire observable state must be unchanged.
template <typename G, typename Prepare, typename Operation>
void enumerate_injected_failures(Prepare prepare, Operation operation)
{
  constexpr auto unlimited = std::numeric_limits<std::size_t>::max();
  std::size_t allocation_count{};
  {
    auto state = std::make_shared<allocation_state>();
    {
      G g(typename G::allocator_type{state, 12});
      prepare(g);
      auto before = state->allocations;
      operation(g);
      allocation_count = state->allocations - before;
      check_consistency(g);
    }
    REQUIRE(state->balanced());
  }
  REQUIRE(allocation_count > 0);
  for (std::size_t allowance = 0; allowance <= allocation_count; ++allowance)
  {
    INFO("injection allowance=" << allowance << "/" << allocation_count);
    auto state = std::make_shared<allocation_state>();
    {
      G g(typename G::allocator_type{state, 12});
      prepare(g);
      auto before = g;
      state->remaining = allowance;
      bool threw = false;
      try
      {
        operation(g);
      }
      catch (std::bad_alloc const &)
      {
        threw = true;
      }
      state->remaining = unlimited;
      if (threw)
        REQUIRE(g == before);
      check_consistency(g);
      // Also verify the failed object remains usable after the exception.
      if (threw)
        operation(g);
      check_consistency(g);
    }
    REQUIRE(state->balanced());
  }
}

TEST_CASE(
    "adjacency_list place_vertex strong guarantee on every allocation",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, false, false, true, true, 2>;
  enumerate_injected_failures<G>(
      [](G &g)
      {
        g.place_vertex();
        g.place_vertex();
      },
      [](G &g) { (void)g.place_vertex(); });
}

TEST_CASE(
    "adjacency_list place_edge strong guarantee: directed full storage",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, true, false, true, true, 2>;
  enumerate_injected_failures<G>(
      [](G &g)
      {
        g.place_vertex();
        g.place_vertex();
      },
      [](G &g) { (void)g.place_edge(typename G::vertex_type{0u}, typename G::vertex_type{1u}); });
}

TEST_CASE(
    "adjacency_list place_edge strong guarantee: undirected self-loop",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, false, true, true, true, 2>;
  enumerate_injected_failures<G>(
      [](G &g) { g.place_vertex(); },
      [](G &g) { (void)g.place_edge(typename G::vertex_type{0u}, typename G::vertex_type{0u}); });
}

TEST_CASE(
    "adjacency_list place_edge strong guarantee: outgoing-only directed",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, true, false, false, true, 2>;
  enumerate_injected_failures<G>(
      [](G &g)
      {
        g.place_vertex();
        g.place_vertex();
      },
      [](G &g) { (void)g.place_edge(typename G::vertex_type{0u}, typename G::vertex_type{1u}); });
}

TEST_CASE(
    "adjacency_list place_edge strong guarantee: incoming-only directed",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, true, false, true, false, 2>;
  enumerate_injected_failures<G>(
      [](G &g)
      {
        g.place_vertex();
        g.place_vertex();
      },
      [](G &g) { (void)g.place_edge(typename G::vertex_type{0u}, typename G::vertex_type{1u}); });
}

TEST_CASE(
    "adjacency_list place_edge rollback preserves a populated graph",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, true, false, true, true, 2>;
  enumerate_injected_failures<G>(
      [](G &g)
      {
        g.place_vertex();
        g.place_vertex();
        g.place_vertex();
        (void)g.place_edge(typename G::vertex_type{0u}, typename G::vertex_type{1u});
      },
      [](G &g) { (void)g.place_edge(typename G::vertex_type{2u}, typename G::vertex_type{0u}); });
}

TEST_CASE(
    "adjacency_list failed allocator rejects vertex insertion without changes",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G g(G::allocator_type{state, 2});
    auto before = g;
    state->reject = true;
    REQUIRE_THROWS_AS(g.place_vertex(), std::bad_alloc);
    state->reject = false;
    REQUIRE(g == before);
    REQUIRE(g.place_vertex().id() == 0u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("adjacency_list duplicate edge insertion does not allocate", "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<>;
  auto state = std::make_shared<allocation_state>();
  {
    G g(G::allocator_type{state, 2});
    auto v = make_vertices(g, 2);
    REQUIRE(g.place_edge(v[0], v[1]).second);
    auto before = g;
    state->reject = true;
    auto [edge, placed] = g.place_edge(v[1], v[0]);
    state->reject = false;
    REQUIRE_FALSE(placed);
    REQUIRE(g == before);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list failed edge insertion permits a later insertion",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<false, true, true>;
  auto state = std::make_shared<allocation_state>();
  {
    G g(G::allocator_type{state, 5});
    auto v = make_vertices(g, 2);
    auto before = g;
    state->reject = true;
    REQUIRE_THROWS_AS(g.place_edge(v[0], v[1]), std::bad_alloc);
    state->reject = false;
    REQUIRE(g == before);
    auto [edge, placed] = g.place_edge(v[0], v[1]);
    REQUIRE(placed);
    REQUIRE(edge.id() == 0u);
    check_consistency(g);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "adjacency_list edge-list construction releases allocations on failure",
    "[graph][adjacency_list][exception]")
{
  using G = tracked_graph<>;
  std::vector<std::pair<unsigned, unsigned>> edges{{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}};
  bool saw_failure = false;
  for (std::size_t allowance = 0; allowance < 32; ++allowance)
  {
    auto state = std::make_shared<allocation_state>();
    state->remaining = allowance;
    try
    {
      G g(edges.begin(), edges.end(), typename G::allocator_type{state, 7});
      REQUIRE(g.vertex_count() == 7);
    }
    catch (std::bad_alloc const &)
    {
      saw_failure = true;
    }
    REQUIRE(state->balanced());
  }
  REQUIRE(saw_failure);
}
