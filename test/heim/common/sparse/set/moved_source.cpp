#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE(
    "sparse_set moved-from source stays coherent and reusable",
    "[common][sparse_set][move][regression]")
{
  hs::set<> source{1u, 9u, 511u, 2048u};
  hs::set<> destination(std::move(source));
  hs::verify(destination, {1u, 9u, 511u, 2048u}, 2050);
  std::unordered_set<unsigned> observed{source.begin(), source.end()};
  hs::verify(source, observed, 2050, {1u, 9u, 511u, 2048u});
  REQUIRE(source.insert(8192u).second);
  REQUIRE(source.contains(8192u));
  hs::verify(destination, {1u, 9u, 511u, 2048u}, 2050);
}

TEST_CASE(
    "sparse_set unequal-allocator move source stays coherent",
    "[common][sparse_set][move][allocator][regression]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using S = hs::tracked_set<8, false>;
    S source{S::allocator_type{log, 1}};
    for (unsigned key : {1u, 9u, 255u, 600u})
      source.insert(key);
    S destination(std::move(source), S::allocator_type{log, 2});
    hs::verify(destination, {1u, 9u, 255u, 600u}, 601);
    std::unordered_set<unsigned> remaining{source.begin(), source.end()};
    hs::verify(source, remaining, 601, {1u, 9u, 255u, 600u});
    REQUIRE(source.insert(900u).second);
    hs::verify(destination, {1u, 9u, 255u, 600u}, 901);
  }
  REQUIRE(log->balanced());
}
