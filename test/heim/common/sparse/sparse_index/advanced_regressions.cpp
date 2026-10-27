#include "support.hpp"

using namespace heim_test::sparse_index;

TEST_CASE(
    "paged sparse_index failed copy assignment leaves valid reusable storage",
    "[common][sparse_index][copy][exception][regression]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8, false> from{tracked_allocator<unsigned, false>{log, 1}};
    observed_index<8, false> to{tracked_allocator<unsigned, false>{log, 2}};
    for (unsigned key : {1u, 17u, 33u, 49u}) from.assure(key)
      = key + 1u;
    to.assure(700u) = 42u;
    log->remaining = 2;
    REQUIRE_THROWS_AS(to = from, std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
    // Only the basic guarantee is required: the destination can be empty,
    // unchanged, or partially copied. Any observable stored value must be
    // valid; all source mappings must remain unchanged.
    for (unsigned key : {1u, 17u, 33u, 49u})
    {
      if (auto const *value = to.get(key))
        REQUIRE(*value == key + 1u);
      check(from, key, key + 1u);
    }
    if (auto const *old = to.get(700u))
      REQUIRE(*old == 42u);
    to.clear();
    to.assure(800u) = 80u;
    check(to, 800u, 80u);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "paged sparse_index propagating failed copy assignment leaves valid storage",
    "[common][sparse_index][copy][exception][regression]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8, true> from{tracked_allocator<unsigned, true>{log, 1}};
    observed_index<8, true> to{tracked_allocator<unsigned, true>{log, 2}};
    for (unsigned key : {1u, 17u, 33u, 49u}) from.assure(key)
      = key + 1u;
    to.assure(700u) = 42u;
    log->remaining = 2;
    REQUIRE_THROWS_AS(to = from, std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
    // Only the basic guarantee is required: the destination can be empty,
    // unchanged, or partially copied. Any observable stored value must be
    // valid; all source mappings must remain unchanged.
    for (unsigned key : {1u, 17u, 33u, 49u})
    {
      if (auto const *value = to.get(key))
        REQUIRE(*value == key + 1u);
      check(from, key, key + 1u);
    }
    if (auto const *old = to.get(700u))
      REQUIRE(*old == 42u);
    to.clear();
    to.assure(800u) = 80u;
    check(to, 800u, 80u);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "paged sparse_index unequal allocator move assignment empties source",
    "[common][sparse_index][move][allocator][regression]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8, false> from{tracked_allocator<unsigned, false>{log, 1}};
    observed_index<8, false> to{tracked_allocator<unsigned, false>{log, 2}};
    from.assure(1u) = 5u;
    from.assure(49u) = 6u;
    to = std::move(from);
    check(to, 1u, 5u);
    check(to, 49u, 6u);
    check_missing(from, 1u);
    check_missing(from, 49u);
    from.assure(1u) = 9u;
    check(to, 1u, 5u);
  }
  REQUIRE(log->balanced());
}
