#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE(
    "sparse_set failed copy assignment clears destination and preserves source",
    "[common][sparse_set][exception][copy]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using S = hs::tracked_set<8>;
    S source{S::allocator_type{log, 9}}, destination{S::allocator_type{log, 9}};
    for (unsigned k = 0; k < 48; ++k)
      source.insert(k * 8u);
    destination.insert(7000u);
    log->remaining = 0;
    REQUIRE_THROWS_AS(destination = source, std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
    hs::verify(destination, {}, 7001);
    REQUIRE(source.size() == 48);
    REQUIRE(source.contains(0u));
    REQUIRE(source.contains(376u));
    REQUIRE(destination.insert(8000u).second);
  }
  REQUIRE(log->balanced());
}
