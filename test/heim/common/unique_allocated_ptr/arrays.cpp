#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

using heim_test::allocation::live_value;
using heim_test::allocation::ledger;
using heim_test::allocation::tracking_allocator;
using heim_test::allocation::balanced;

TEST_CASE("ordinary arrays value-initialize scalar elements", "[common][unique_allocated_ptr][array]")
{
  for (std::size_t n : {1u, 2u, 3u, 8u, 64u, 257u})
  {
    DYNAMIC_SECTION("array size=" << n)
    {
      auto p = heim::allocate_unique<int[]>(std::allocator<int>{}, n);
      REQUIRE(p != nullptr);
      for (std::size_t i = 0; i != n; ++i)
        REQUIRE(p[i] == 0);
      for (std::size_t i = 0; i != n; ++i)
        p[i] = static_cast<int>(i * 3);
      for (std::size_t i = 0; i != n; ++i)
        REQUIRE(p[i] == static_cast<int>(i * 3));
    }
  }
}

TEST_CASE(
    "zero-length ordinary arrays do not allocate or construct",
    "[common][unique_allocated_ptr][array]")
{
  auto log = std::make_shared<ledger>();
  auto p = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log}, 0);
  REQUIRE(p == nullptr);
  REQUIRE(log->allocations == 0);
  REQUIRE(log->construct_attempts == 0);
  REQUIRE(log->deallocations == 0);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "zero-length overwrite arrays do not allocate or construct",
    "[common][unique_allocated_ptr][array][overwrite]")
{
  auto log = std::make_shared<ledger>();
  auto p = heim::allocate_unique_for_overwrite<live_value[]>(tracking_allocator<live_value>{log}, 0);
  REQUIRE(p == nullptr);
  REQUIRE(log->allocations == 0);
  REQUIRE(log->construct_attempts == 0);
  REQUIRE(log->deallocations == 0);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "ordinary arrays construct and destroy exactly N objects",
    "[common][unique_allocated_ptr][array]")
{
  for (std::size_t n : {1u, 2u, 3u, 8u, 33u})
  {
    DYNAMIC_SECTION("array size=" << n)
    {
      live_value::reset();
      {
        auto p = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, n);
        REQUIRE(live_value::alive == static_cast<int>(n));
        REQUIRE(live_value::constructed == static_cast<int>(n));
        for (std::size_t i = 0; i != n; ++i)
          p[i].payload = static_cast<int>(i + 3);
        for (std::size_t i = 0; i != n; ++i)
          REQUIRE(p[i].payload == static_cast<int>(i + 3));
      }
      REQUIRE(live_value::alive == 0);
      REQUIRE(live_value::destroyed == static_cast<int>(n));
    }
  }
}

TEST_CASE(
    "ordinary arrays destroy class elements in reverse order",
    "[common][unique_allocated_ptr][array]")
{
  live_value::reset();
  {
    auto p = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, 4);
    REQUIRE(live_value::alive == 4);
  }
  REQUIRE(live_value::destruction_order == std::vector<int>{3, 2, 1, 0});
}

TEST_CASE(
    "overwrite arrays construct and destroy class elements",
    "[common][unique_allocated_ptr][array][overwrite]")
{
  for (std::size_t n : {1u, 2u, 5u, 32u})
  {
    DYNAMIC_SECTION("array size=" << n)
    {
      live_value::reset();
      {
        auto p = heim::allocate_unique_for_overwrite<live_value[]>(std::allocator<live_value>{}, n);
        REQUIRE(live_value::alive == static_cast<int>(n));
        for (std::size_t i = 0; i != n; ++i)
          p[i].payload = static_cast<int>(i * 5);
        for (std::size_t i = 0; i != n; ++i)
          REQUIRE(p[i].payload == static_cast<int>(i * 5));
      }
      REQUIRE(live_value::alive == 0);
      REQUIRE(live_value::destroyed == static_cast<int>(n));
    }
  }
}

TEST_CASE(
    "overwrite arrays destroy class elements in reverse order",
    "[common][unique_allocated_ptr][array][overwrite]")
{
  live_value::reset();
  {
    auto p = heim::allocate_unique_for_overwrite<live_value[]>(std::allocator<live_value>{}, 4);
    REQUIRE(live_value::alive == 4);
  }
  REQUIRE(live_value::destruction_order == std::vector<int>{3, 2, 1, 0});
}

TEST_CASE(
    "overwrite arrays allow scalar assignment without reading indeterminate values",
    "[common][unique_allocated_ptr][array][overwrite]")
{
  auto p = heim::allocate_unique_for_overwrite<int[]>(std::allocator<int>{}, 128);
  for (int i = 0; i != 128; ++i)
    p[i] = i * i;
  for (int i = 0; i != 128; ++i)
    REQUIRE(p[i] == i * i);
}

TEST_CASE("moving an array leaves exactly one owner", "[common][unique_allocated_ptr][array]")
{
  live_value::reset();
  {
    auto a = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, 5);
    auto *address = a.get();
    a[0].payload = 12;
    auto b = std::move(a);
    REQUIRE(a == nullptr);
    REQUIRE(b.get() == address);
    REQUIRE(b[0].payload == 12);
    REQUIRE(live_value::constructed == 5);
  }
  REQUIRE(live_value::destroyed == 5);
}

TEST_CASE(
    "moving onto an existing array destroys its old elements",
    "[common][unique_allocated_ptr][array]")
{
  live_value::reset();
  {
    auto a = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, 3);
    auto b = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, 2);
    a[1].payload = 63;
    b = std::move(a);
    REQUIRE(a == nullptr);
    REQUIRE(b[1].payload == 63);
    REQUIRE(live_value::alive == 3);
    REQUIRE(live_value::destroyed == 2);
  }
  REQUIRE(live_value::destroyed == 5);
}

TEST_CASE("array reset destroys each element once", "[common][unique_allocated_ptr][array]")
{
  live_value::reset();
  auto p = heim::allocate_unique<live_value[]>(std::allocator<live_value>{}, 4);
  p.reset();
  REQUIRE_FALSE(p);
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 4);
  p.reset();
  REQUIRE(live_value::destroyed == 4);
}

TEST_CASE("array release preserves array extent in copied deleter", "[common][unique_allocated_ptr][array]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto p = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log, 30}, 7);
    auto deleter = p.get_deleter();
    auto raw = p.release();
    REQUIRE(p == nullptr);
    REQUIRE(live_value::alive == 7);
    deleter(raw);
  }
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 7);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "array swap exchanges ownership and keeps correct array extents",
    "[common][unique_allocated_ptr][array]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto a = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log}, 2);
    auto b = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log}, 5);
    auto *pa = a.get();
    auto *pb = b.get();
    a.swap(b);
    REQUIRE(a.get() == pb);
    REQUIRE(b.get() == pa);
  }
  REQUIRE(live_value::destroyed == 7);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "an array allocation works with a non-default-constructible allocator",
    "[common][unique_allocated_ptr][array]")
{
  auto p = heim::allocate_unique<int[]>(heim_test::allocation::explicit_allocator<int>{8}, 3);
  for (int i = 0; i != 3; ++i)
    REQUIRE(p[i] == 0);
}
