#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE(
    "sparse_map moved-from source retains aligned keys and values",
    "[common][sparse_map][move][regression]")
{
  map<> source{{1u, 11}, {8u, 88}, {255u, 255}, {4000u, 40}};
  map<> destination{std::move(source)};
  verify(destination, {{1u, 11}, {8u, 88}, {255u, 255}, {4000u, 40}}, 4001);
  // Before dereferencing proxy iterators, reject a moved-from key container
  // that is nonempty while its sparse index has lost all its entries.
  if (!source.empty())
    REQUIRE((source.contains(1u) || source.contains(8u) ||
             source.contains(255u) || source.contains(4000u)));
  auto const remaining = snapshot(source);
  verify(source, remaining, 4001, {1u, 8u, 255u, 4000u});
  REQUIRE(source.try_emplace(5000u, 50).second);
  verify(destination, {{1u, 11}, {8u, 88}, {255u, 255}, {4000u, 40}}, 5001);
}

TEST_CASE(
    "sparse_map unequal-allocator move source remains reusable",
    "[common][sparse_map][move][allocator][regression]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<>;
    M source{M::allocator_type{state, 1}};
    source.try_emplace(1u, 11);
    source.try_emplace(255u, 22);
    source.try_emplace(900u, 33);
    M destination{std::move(source), M::allocator_type{state, 2}};
    verify(destination, {{1u, 11}, {255u, 22}, {900u, 33}}, 901);
    if (!source.empty())
      REQUIRE((source.contains(1u) || source.contains(255u) || source.contains(900u)));
    auto const remaining = snapshot(source);
    verify(source, remaining, 901, {1u, 255u, 900u});
    REQUIRE(source.try_emplace(1500u, 44).second);
    verify(destination, {{1u, 11}, {255u, 22}, {900u, 33}}, 1501);
  }
  REQUIRE(state->balanced());
}
