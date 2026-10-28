#include "support.hpp"

#include <concepts>
#include <type_traits>

using namespace heim_test::adjacency;

constexpr bool descriptors_are_constant_evaluable()
{
  graph<>::vertex_type v{7u};
  graph<>::vertex_type w{8u};
  return v.id() == 7u && v != w && (v < w) && static_cast<unsigned>(v) == 7u;
}
static_assert(descriptors_are_constant_evaluable());
static_assert(std::is_copy_constructible_v<graph<>>);
static_assert(std::is_move_constructible_v<graph<>>);
static_assert(std::is_copy_assignable_v<graph<>>);
static_assert(std::is_move_assignable_v<graph<>>);
static_assert(noexcept(std::declval<graph<> const &>().vertices()));
static_assert(noexcept(std::declval<graph<> const &>().edges()));
static_assert(noexcept(std::declval<graph<> const &>().vertex_count()));
static_assert(noexcept(std::declval<graph<> const &>().edge_count()));
static_assert(noexcept(std::declval<graph<> const &>().get_allocator()));
static_assert(noexcept(std::declval<graph<> &>().clear()));
static_assert(noexcept(std::declval<graph<> &>().remove_edge(std::declval<graph<>::edge_type>())));

TEST_CASE(
    "adjacency_list vertex and edge descriptors remain lightweight values",
    "[graph][adjacency_list][constexpr]")
{
  static_assert(std::copy_constructible<graph<>::vertex_type>);
  static_assert(std::copy_constructible<graph<>::edge_type>);
  graph<> g;
  auto vs = make_vertices(g, 2);
  auto [e, inserted] = g.place_edge(vs[0], vs[1]);
  REQUIRE(inserted);
  auto copied = e;
  REQUIRE(copied == e);
  REQUIRE(e.id() == 0u);
}

TEST_CASE("adjacency_list const operations preserve graph state", "[graph][adjacency_list][constexpr]")
{
  graph<> g{{{0, 1}, {1, 2}}};
  auto copy = g;
  graph<> const &cg = g;
  REQUIRE(cg.vertex_count() == 3);
  REQUIRE(cg.edge_count() == 2);
  REQUIRE(std::ranges::distance(cg.vertices()) == 3);
  REQUIRE(std::ranges::distance(cg.edges()) == 2);
  REQUIRE(g == copy);
}
