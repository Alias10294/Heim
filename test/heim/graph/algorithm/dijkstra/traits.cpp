#include "support.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

namespace
{
using G = graph<>;
using V = vertex<G>;
using WM = weight_map<edge<G>>;
using Result = hg::dijkstra_result<G, WM>;
using ToResult = hg::dijkstra_to_result<G, WM>;

template<typename T>
concept has_temporary_path_from
= requires (T &&value, V v) { std::forward<T>(value).path_from(v); };

template<typename T>
concept has_temporary_target_path
= requires (T &&value) { std::forward<T>(value).path(); };

static_assert(std::move_constructible<Result>);
static_assert(std::move_constructible<ToResult>);
static_assert(!std::copy_constructible<Result>);
static_assert(!std::copy_constructible<ToResult>);
static_assert(std::is_constructible_v<Result, std::in_place_type_t<G>, std::in_place_type_t<WM>>);
static_assert(std::is_constructible_v<ToResult, std::in_place_type_t<G>, V, std::in_place_type_t<WM>>);
static_assert(std::same_as<hg::dijkstra_result<G, WM>, hg::dijkstra_search_result<G, WM>>);
static_assert(std::same_as<hg::dijkstra_to_result<G, WM>, hg::dijkstra_search_to_result<G, WM>>);
static_assert(requires (Result const &r, V v) { r.reached(v); });
static_assert(requires (Result const &r, V v) { r.distance_of(v); });
static_assert(requires (Result const &r, V v) { r.predecessor_of(v); });
static_assert(requires (ToResult const &r) { r.reached(); });
static_assert(requires (ToResult const &r) { r.distance(); });
static_assert(requires (Result const &r, V v) { r.path_from(v); });
static_assert(requires (ToResult const &r) { r.path(); });
static_assert(!has_temporary_path_from<Result>);
static_assert(!has_temporary_path_from<Result const>);
static_assert(!has_temporary_target_path<ToResult>);
static_assert(!has_temporary_target_path<ToResult const>);
static_assert(hg::dijkstra_heap_arity_v >= 2);
static_assert(std::is_invocable_v<decltype(hg::dijkstra), G const &, V, WM>);
static_assert(std::is_invocable_v<decltype(hg::dijkstra_to), G const &, V, V, WM>);
struct trivial_visitor { };
static_assert(std::is_invocable_v<decltype(hg::dijkstra_visit), G const &, V, WM, trivial_visitor>);
} // namespace

TEST_CASE("Dijkstra aliases denote the same callable type", "[graph][dijkstra][traits]")
{
  STATIC_REQUIRE(std::same_as<decltype(hg::dijkstra_search), decltype(hg::dijkstra)>);
  STATIC_REQUIRE(std::same_as<decltype(hg::dijkstra_search_to), decltype(hg::dijkstra_to)>);
}
