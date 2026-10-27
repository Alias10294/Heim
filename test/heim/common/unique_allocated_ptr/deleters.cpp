#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <memory>
#include <utility>

using heim_test::allocation::balanced;
using heim_test::allocation::ledger;
using heim_test::allocation::live_value;
using heim_test::allocation::tracking_allocator;

TEST_CASE("normal single deleter destroys and deallocates", "[common][unique_allocated_ptr][deleter]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  auto alloc = tracking_allocator<live_value>{log, 5};
  auto *ptr = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
  std::allocator_traits<decltype(alloc)>::construct(alloc, ptr, 33);
  heim::allocator_aware_deleter<live_value, decltype(alloc), false> d{alloc};
  d(ptr);
  REQUIRE(log->constructs == 1);
  REQUIRE(log->destroys == 1);
  REQUIRE(live_value::destroyed == 1);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "normal array deleter destroys all elements and deallocates exact extent",
    "[common][unique_allocated_ptr][deleter][array]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  auto alloc = tracking_allocator<live_value>{log, 18};
  auto *ptr = alloc.allocate(4);
  for (int i = 0; i != 4; ++i)
    alloc.construct(ptr + i, i);
  heim::allocator_aware_deleter<live_value[], decltype(alloc), false> d{alloc, 4};
  d(ptr);
  REQUIRE(log->destroys == 4);
  REQUIRE(live_value::destroyed == 4);
  REQUIRE(live_value::destruction_order == std::vector<int>{3, 2, 1, 0});
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "overwrite single deleter bypasses allocator destroy hook",
    "[common][unique_allocated_ptr][deleter][overwrite]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  auto alloc = tracking_allocator<live_value>{log, 18};
  auto *ptr = alloc.allocate(1);
  ::new (static_cast<void *>(ptr)) live_value{9};
  heim::allocator_aware_deleter<live_value, decltype(alloc), true> d{alloc};
  d(ptr);
  REQUIRE(log->destroys == 0);
  REQUIRE(live_value::destroyed == 1);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "overwrite array deleter destroys all elements but bypasses allocator hook",
    "[common][unique_allocated_ptr][deleter][array][overwrite]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  auto alloc = tracking_allocator<live_value>{log, 18};
  auto *ptr = alloc.allocate(5);
  for (int i = 0; i != 5; ++i)
    ::new (static_cast<void *>(ptr + i)) live_value{i};
  heim::allocator_aware_deleter<live_value[], decltype(alloc), true> d{alloc, 5};
  d(ptr);
  REQUIRE(log->destroys == 0);
  REQUIRE(live_value::destroyed == 5);
  REQUIRE(live_value::destruction_order == std::vector<int>{4, 3, 2, 1, 0});
  REQUIRE(balanced(*log));
}

TEST_CASE("null normal deleters never deallocate or destroy", "[common][unique_allocated_ptr][deleter]")
{
  auto log = std::make_shared<ledger>();
  auto alloc = tracking_allocator<int>{log};
  heim::allocator_aware_deleter<int, decltype(alloc), false> single{alloc};
  heim::allocator_aware_deleter<int[], decltype(alloc), false> array{alloc, 15};
  single(nullptr);
  array(nullptr);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->destroys == 0);
}

TEST_CASE(
    "null overwrite deleters never deallocate or destroy",
    "[common][unique_allocated_ptr][deleter][overwrite]")
{
  auto log = std::make_shared<ledger>();
  auto alloc = tracking_allocator<int>{log};
  heim::allocator_aware_deleter<int, decltype(alloc), true> single{alloc};
  heim::allocator_aware_deleter<int[], decltype(alloc), true> array{alloc, 15};
  single(nullptr);
  array(nullptr);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->destroys == 0);
}

TEST_CASE(
    "copying a deleter retains its allocator and its array size",
    "[common][unique_allocated_ptr][deleter][array]")
{
  auto log = std::make_shared<ledger>();
  auto alloc = tracking_allocator<int>{log, 17};
  auto *ptr = alloc.allocate(3);
  for (int i = 0; i != 3; ++i)
    alloc.construct(ptr + i, i);
  heim::allocator_aware_deleter<int[], decltype(alloc), false> d{alloc, 3};
  auto copy = d;
  copy(ptr);
  REQUIRE(log->destroys == 3);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "moving a deleter retains its array size and allocator ownership",
    "[common][unique_allocated_ptr][deleter][array]")
{
  auto log = std::make_shared<ledger>();
  auto alloc = tracking_allocator<int>{log, 19};
  auto *ptr = alloc.allocate(6);
  for (int i = 0; i != 6; ++i)
    alloc.construct(ptr + i, i);
  heim::allocator_aware_deleter<int[], decltype(alloc), false> d{alloc, 6};
  auto moved = std::move(d);
  moved(ptr);
  REQUIRE(log->destroys == 6);
  REQUIRE(balanced(*log));
}
