#include "support.hpp"

using namespace heim_test::sparse_index;

TEST_CASE("page allocations are balanced at scope exit", "[common][sparse_index][allocator]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{log, 21}};
    for (unsigned n = 0; n < 40; ++n)
      idx.assure(n * 8u) = n;
    REQUIRE(log->live_pages() == 40);
    REQUIRE(log->invalid_deallocations == 0);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("empty paged index allocates no pages", "[common][sparse_index][allocator]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{log}};
    check_missing(idx, 9999u);
    idx.clear();
    check_missing(idx, 9999u);
    REQUIRE(log->live_pages() == 0);
    REQUIRE(log->page_allocations == 0);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("contiguous index never allocates page objects", "[common][sparse_index][allocator]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<0> idx{tracked_allocator<unsigned>{log}};
    idx.assure(10u) = 13u;
    idx.assure(100u) = 25u;
    REQUIRE(log->page_allocations == 0);
    check(idx, 10u, 13u);
    check(idx, 100u, 25u);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "copy construction with an explicit allocator uses that allocator",
    "[common][sparse_index][allocator]")
{
  auto source_log = std::make_shared<allocation_log>();
  auto copy_log = std::make_shared<allocation_log>();
  {
    observed_index<8> source{tracked_allocator<unsigned>{source_log, 1}};
    source.assure(1u) = 10u;
    source.assure(100u) = 20u;
    observed_index<8> copy{source, tracked_allocator<unsigned>{copy_log, 2}};
    REQUIRE(copy.get_allocator().owner == 2);
    REQUIRE(copy.get_allocator().log == copy_log);
    REQUIRE(source_log->live_pages() == 2);
    REQUIRE(copy_log->live_pages() == 2);
    REQUIRE(source.get(1u) != copy.get(1u));
    check(copy, 100u, 20u);
  }
  REQUIRE(source_log->balanced());
  REQUIRE(copy_log->balanced());
}

TEST_CASE(
    "copy assignment with non-propagating allocator retains its destination allocator",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8, false> source{tracked_allocator<unsigned, false>{state, 10}};
    observed_index<8, false> dest{tracked_allocator<unsigned, false>{state, 20}};
    source.assure(15u) = 5u;
    source.assure(32u) = 6u;
    dest.assure(7u) = 99u;
    dest = source;
    REQUIRE(dest.get_allocator().owner == 20);
    check(dest, 15u, 5u);
    check(dest, 32u, 6u);
    check_missing(dest, 7u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "copy assignment with propagating allocator adopts the source allocator",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8, true> source{tracked_allocator<unsigned, true>{state, 10}};
    observed_index<8, true> dest{tracked_allocator<unsigned, true>{state, 20}};
    source.assure(3u) = 55u;
    dest.assure(7u) = 99u;
    dest = source;
    REQUIRE(dest.get_allocator().owner == 10);
    check(dest, 3u, 55u);
    check_missing(dest, 7u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "move assignment with non-propagating unequal allocators preserves ownership",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8, false> source{tracked_allocator<unsigned, false>{state, 10}};
    observed_index<8, false> dest{tracked_allocator<unsigned, false>{state, 20}};
    source.assure(1u) = 4u;
    source.assure(800u) = 5u;
    dest.assure(6u) = 6u;
    dest = std::move(source);
    REQUIRE(dest.get_allocator().owner == 20);
    check(dest, 1u, 4u);
    check(dest, 800u, 5u);
    check_missing(dest, 6u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "move assignment with propagating allocator transfers allocator identity",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8, true> source{tracked_allocator<unsigned, true>{state, 10}};
    observed_index<8, true> dest{tracked_allocator<unsigned, true>{state, 20}};
    source.assure(5u) = 77u;
    dest.assure(1u) = 44u;
    dest = std::move(source);
    REQUIRE(dest.get_allocator().owner == 10);
    check(dest, 5u, 77u);
    check_missing(dest, 1u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "move construction with an explicit different allocator is well formed",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> source{tracked_allocator<unsigned>{state, 10}};
    source.assure(5u) = 7u;
    source.assure(400u) = 8u;
    observed_index<8> dest{std::move(source), tracked_allocator<unsigned>{state, 20}};
    REQUIRE(dest.get_allocator().owner == 20);
    check(dest, 5u, 7u);
    check(dest, 400u, 8u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "swapping indices with propagating allocators exchanges allocator identity",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8, true> left{tracked_allocator<unsigned, true>{state, 11}};
    observed_index<8, true> right{tracked_allocator<unsigned, true>{state, 22}};
    left.assure(1u) = 10u;
    right.assure(17u) = 30u;
    left.swap(right);
    REQUIRE(left.get_allocator().owner == 22);
    REQUIRE(right.get_allocator().owner == 11);
    check(left, 17u, 30u);
    check(right, 1u, 10u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "page memory is released after mixed copy move swap and clear operations",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> a{tracked_allocator<unsigned>{state}};
    a.assure(1u) = 1u;
    a.assure(33u) = 33u;
    observed_index<8> b{a};
    observed_index<8> c{std::move(b)};
    a.swap(c);
    c.clear();
    a = c;
    REQUIRE(state->live_pages() == 0);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "contiguous copy and move assignments respect non-propagating allocators",
    "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<0> source{tracked_allocator<unsigned>{state, 1}};
    observed_index<0> dest{tracked_allocator<unsigned>{state, 2}};
    source.assure(20u) = 1u;
    dest.assure(15u) = 2u;
    dest = source;
    REQUIRE(dest.get_allocator().owner == 2);
    check(dest, 20u, 1u);
    dest = std::move(source);
    REQUIRE(dest.get_allocator().owner == 2);
    check(dest, 20u, 1u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("contiguous copy assignment respects propagating allocators", "[common][sparse_index][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<0, true> source{tracked_allocator<unsigned, true>{state, 1}};
    observed_index<0, true> dest{tracked_allocator<unsigned, true>{state, 2}};
    source.assure(100u) = 60u;
    dest = source;
    REQUIRE(dest.get_allocator().owner == 1);
    check(dest, 100u, 60u);
  }
  REQUIRE(state->balanced());
}
