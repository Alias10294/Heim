#include "copy_selection_support.hpp"

#include <memory>
#include <type_traits>
#include <utility>

using namespace heim_test::adjacency;

static_assert(std::same_as<std::allocator_traits<copy_select_allocator<unsigned>>::value_type, unsigned>);
static_assert(
    !std::allocator_traits<copy_select_allocator<unsigned>>::propagate_on_container_copy_assignment::value);

TEST_CASE(
    "adjacency_list SOCCC selects a different allocator during implicit copy construction",
    "[graph][adjacency_list][allocator][soccc]")
{
  using G = copy_select_graph<>;
  auto ledger = std::make_shared<allocation_state>();
  {
    G source{G::allocator_type{ledger, 7}};
    auto vertices = make_vertices(source, 24);
    for (unsigned i = 1; i < vertices.size(); ++i)
      source.place_edge(vertices[0], vertices[i]);

    auto const expected = snapshot(source);
    REQUIRE(allocations_owned_by(*ledger, 7) > 0);
    auto const original_allocations = allocated_addresses(*ledger);

    G copy{source};
    REQUIRE(source.get_allocator().id == 7);
    REQUIRE(copy.get_allocator().id == 107);
    REQUIRE(copy == source);
    verify(copy, expected);
    REQUIRE(allocations_owned_by(*ledger, 107) > 0);
    require_new_allocations_owned_by(*ledger, original_allocations, 107);
    require_only_owners(*ledger, 7, 107);

    // A deep copy must not share ownership of incidence or sparse storage.
    copy.remove_edges(vertices[0], vertices[1]);
    REQUIRE(source.edge_count() == 23);
    REQUIRE(copy.edge_count() == 22);
    check_consistency(source);
    check_consistency(copy);
  }
  REQUIRE(ledger->balanced());
}

TEST_CASE(
    "adjacency_list SOCCC propagates through nested directed incidence storage",
    "[graph][adjacency_list][allocator][soccc]")
{
  using G = copy_select_graph<true, true, true, true, 2>;
  auto ledger = std::make_shared<allocation_state>();
  {
    G source{G::allocator_type{ledger, 9}};
    auto vertices = make_vertices(source, 18);
    for (unsigned i = 1; i < vertices.size(); ++i)
    {
      source.place_edge(vertices[0], vertices[i]);
      source.place_edge(vertices[i], vertices[0]);
      source.place_edge(vertices[i], vertices[i]);
    }
    auto const expected = snapshot(source);
    auto const original_allocations = allocated_addresses(*ledger);
    G copy{source};
    verify(copy, expected);
    REQUIRE(copy.get_allocator().id == 109);
    require_new_allocations_owned_by(*ledger, original_allocations, 109);
    REQUIRE(allocations_owned_by(*ledger, 9) > 0);
    REQUIRE(allocations_owned_by(*ledger, 109) > 0);
    // In particular, no nested container should accidentally select twice
    // (209) or revert to the source's identity when allocating copy storage.
    require_only_owners(*ledger, 9, 109);

    auto const before = allocated_addresses(*ledger);
    auto extra = copy.place_vertex();
    copy.place_edge(extra, vertices[0]);
    copy.place_edge(vertices[0], extra);
    // New allocations (if growth requires them) must still use the selected
    // identity; existing capacity may also suffice.
    for (auto const &[ptr, info] : ledger->outstanding)
      if (!before.contains(ptr))
        REQUIRE(info.owner == 109);
    require_only_owners(*ledger, 9, 109);
    check_consistency(source);
    check_consistency(copy);
  }
  REQUIRE(ledger->balanced());
}

TEST_CASE(
    "adjacency_list explicit copy allocator overrides SOCCC selection",
    "[graph][adjacency_list][allocator][soccc]")
{
  using G = copy_select_graph<true, false, false, true, 2>;
  auto ledger = std::make_shared<allocation_state>();
  {
    G source{G::allocator_type{ledger, 11}};
    auto vertices = make_vertices(source, 14);
    for (unsigned i = 1; i < vertices.size(); ++i)
      source.place_edge(vertices[0], vertices[i]);
    auto const expected = snapshot(source);
    auto const original_allocations = allocated_addresses(*ledger);

    G copy{source, G::allocator_type{ledger, 42}};
    verify(copy, expected);
    REQUIRE(copy.get_allocator().id == 42);
    REQUIRE(allocations_owned_by(*ledger, 42) > 0);
    require_new_allocations_owned_by(*ledger, original_allocations, 42);
    REQUIRE(allocations_owned_by(*ledger, 111) == 0);
    REQUIRE(allocations_owned_by(*ledger, 142) == 0);
    require_only_owners(*ledger, 11, 42);
  }
  REQUIRE(ledger->balanced());
}

TEST_CASE(
    "adjacency_list move construction does not invoke SOCCC",
    "[graph][adjacency_list][allocator][soccc]")
{
  using G = copy_select_graph<false, true>;
  auto ledger = std::make_shared<allocation_state>();
  {
    G source{G::allocator_type{ledger, 13}};
    auto vertices = make_vertices(source, 12);
    for (unsigned i = 1; i < vertices.size(); ++i)
      source.place_edge(vertices[0], vertices[i]);
    auto const expected = snapshot(source);

    G moved{std::move(source)};
    REQUIRE(moved.get_allocator().id == 13);
    verify(moved, expected);
    REQUIRE(allocations_owned_by(*ledger, 113) == 0);
    require_only_owners(*ledger, 13, 13);
  }
  REQUIRE(ledger->balanced());
}

TEST_CASE(
    "adjacency_list copy assignment does not apply the SOCCC policy",
    "[graph][adjacency_list][allocator][soccc]")
{
  using G = copy_select_graph<true, true>;
  auto ledger = std::make_shared<allocation_state>();
  {
    G source{G::allocator_type{ledger, 15}};
    G destination{G::allocator_type{ledger, 16}};
    auto vertices = make_vertices(source, 12);
    for (unsigned i = 1; i < vertices.size(); ++i)
      source.place_edge(vertices[0], vertices[i]);
    make_vertices(destination, 3);
    destination = source;
    REQUIRE(destination == source);
    REQUIRE(destination.get_allocator().id == 16);
    REQUIRE(allocations_owned_by(*ledger, 16) > 0);
    require_only_owners(*ledger, 15, 16);
    check_consistency(source);
    check_consistency(destination);
  }
  REQUIRE(ledger->balanced());
}
