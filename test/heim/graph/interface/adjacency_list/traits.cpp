#include "support.hpp"

#include <concepts>
#include <cstdint>
#include <functional>
#include <type_traits>

using namespace heim_test::adjacency;
namespace gg = heim::graphs;

static_assert(std::same_as<graph<>::id_type, unsigned>);
static_assert(std::same_as<graph<>::size_type, std::size_t>);
static_assert(std::same_as<graph<>::allocator_type, std::allocator<unsigned>>);
static_assert(graph<>::page_size == 8);
static_assert(!graph<>::is_directed && !graph<>::is_multigraph);
static_assert(graph<>::is_in_incident && graph<>::is_out_incident);
static_assert(gg::directed<graph<true>>);
static_assert(gg::multigraph<graph<false, true>>);
static_assert(!gg::directed<graph<false, true>>);
static_assert(!gg::multigraph<graph<true, false>>);
static_assert(std::same_as<gg::vertex_t<graph<>>, graph<>::vertex_type>);
static_assert(std::same_as<gg::edge_t<graph<>>, graph<>::edge_type>);
static_assert(std::same_as<gg::vertex_t<graph<true>>, graph<true>::vertex_type>);
static_assert(!std::same_as<graph<>::vertex_type, graph<true>::vertex_type>);
static_assert(std::is_trivially_copyable_v<graph<>::vertex_type>);
static_assert(std::is_trivially_copyable_v<graph<>::edge_type>);
static_assert(std::regular<graph<>::vertex_type>);
static_assert(std::regular<graph<>::edge_type>);
static_assert(std::equality_comparable<graph<>>);
static_assert(std::is_same_v<std::invoke_result_t<decltype(gg::vertices), graph<> const &>,
                             decltype(std::declval<graph<> const &>().vertices())>);
static_assert(requires(graph<> &g, graph<>::vertex_type v) {
  { gg::place_vertex(g) } -> std::same_as<graph<>::vertex_type>;
  { gg::place_edge(g, v, v) } -> std::same_as<std::pair<graph<>::edge_type, bool>>;
  gg::clear_vertex(g, v);
  gg::remove_vertex(g, v);
});
template <typename G>
concept has_in_edges = requires(G const &g, typename G::vertex_type v) { g.in_edges(v); };
template <typename G>
concept has_out_edges = requires(G const &g, typename G::vertex_type v) { g.out_edges(v); };
static_assert(!has_in_edges<graph<true, false, false, true>>);
static_assert(!has_out_edges<graph<true, false, true, false>>);

template <bool Directed, bool Incoming, bool Outgoing>
concept valid_layout = requires { typename heim::adjacency_list<Directed, false, Incoming, Outgoing>; };
static_assert(valid_layout<false, true, true>);
static_assert(valid_layout<true, true, false>);
static_assert(valid_layout<true, false, true>);
static_assert(!valid_layout<false, false, true>);
static_assert(!valid_layout<false, true, false>);
static_assert(!valid_layout<true, false, false>);
template <typename UInt, typename Alloc>
concept valid_allocator_id =
    requires { typename heim::adjacency_list<false, false, true, true, UInt, 8, Alloc>; };
static_assert(valid_allocator_id<unsigned, std::allocator<unsigned>>);
static_assert(!valid_allocator_id<unsigned, std::allocator<std::uint16_t>>);

TEST_CASE("adjacency_list graph traits agree with template flags", "[graph][adjacency_list][traits]")
{
  REQUIRE_FALSE(gg::is_directed_v<graph<>>);
  REQUIRE(gg::is_directed_v<graph<true>>);
  REQUIRE(gg::is_multigraph_v<graph<false, true>>);
}

TEST_CASE(
    "adjacency_list supports alternative unsigned id types and page sizes",
    "[graph][adjacency_list][traits]")
{
  graph<true, true, true, true, std::uint16_t, 4> g;
  auto v = make_vertices(g, 5);
  REQUIRE(g.place_edge(v[0], v[4]).second);
  REQUIRE(g.vertex_count() == 5);
  REQUIRE(g.edge_count() == 1);
  check_consistency(g);
}

TEST_CASE(
    "adjacency_list type parameters reject invalid directed incidence layouts",
    "[graph][adjacency_list][traits]")
{
  STATIC_REQUIRE(!std::is_same_v<graph<true, false, true, false>, graph<true, false, false, true>>);
}
