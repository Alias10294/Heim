#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <memory>
#include <utility>

using heim_test::allocation::balanced;
using heim_test::allocation::ledger;
using heim_test::allocation::live_value;
using heim_test::allocation::tracking_allocator;

TEST_CASE(
    "a stateful allocator deallocates the same single block it allocated",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto p = heim::allocate_unique<int>(tracking_allocator<int>{log, 17}, 95);
    REQUIRE(*p == 95);
    REQUIRE(log->allocations == 1);
    REQUIRE(log->outstanding.size() == 1);
    REQUIRE(log->outstanding.front().count == 1);
    REQUIRE(log->outstanding.front().owner == 17);
  }
  REQUIRE(log->deallocations == 1);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "a stateful allocator deallocates the same array block and count",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto p = heim::allocate_unique<int[]>(tracking_allocator<int>{log, 27}, 13);
    REQUIRE(log->allocations == 1);
    REQUIRE(log->outstanding.size() == 1);
    REQUIRE(log->outstanding.front().count == 13);
    REQUIRE(log->outstanding.front().owner == 27);
  }
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "overwrite allocations preserve allocator state",
    "[common][unique_allocated_ptr][allocator][overwrite]")
{
  auto log = std::make_shared<ledger>();
  {
    auto p = heim::allocate_unique_for_overwrite<int>(tracking_allocator<int>{log, 48});
    *p = 14;
    REQUIRE(*p == 14);
    auto a = heim::allocate_unique_for_overwrite<int[]>(tracking_allocator<int>{log, 49}, 9);
    a[0] = 43;
    REQUIRE(a[0] == 43);
    REQUIRE(log->outstanding.size() == 2);
  }
  REQUIRE(log->allocations == 2);
  REQUIRE(log->deallocations == 2);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "normal single-object allocation uses allocator_traits construct and destroy",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto p = heim::allocate_unique<live_value>(tracking_allocator<live_value>{log}, 41);
    REQUIRE(p->payload == 41);
    REQUIRE(log->construct_attempts == 1);
    REQUIRE(log->constructs == 1);
    REQUIRE(log->destroys == 0);
  }
  REQUIRE(log->destroys == 1);
  REQUIRE(live_value::destroyed == 1);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "normal array allocation uses allocator_traits hooks once per element",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto p = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log}, 11);
    REQUIRE(log->construct_attempts == 11);
    REQUIRE(log->constructs == 11);
    REQUIRE(log->destroys == 0);
  }
  REQUIRE(log->destroys == 11);
  REQUIRE(live_value::destroyed == 11);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "overwrite single-object allocation bypasses allocator_traits construction hooks",
    "[common][unique_allocated_ptr][allocator][overwrite]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto p = heim::allocate_unique_for_overwrite<live_value>(tracking_allocator<live_value>{log});
    REQUIRE(live_value::alive == 1);
    REQUIRE(log->construct_attempts == 0);
    REQUIRE(log->constructs == 0);
  }
  REQUIRE(log->destroys == 0);
  REQUIRE(live_value::destroyed == 1);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "overwrite array allocation bypasses allocator_traits construction hooks",
    "[common][unique_allocated_ptr][allocator][overwrite]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  {
    auto p = heim::allocate_unique_for_overwrite<live_value[]>(tracking_allocator<live_value>{log}, 6);
    REQUIRE(live_value::alive == 6);
    REQUIRE(log->construct_attempts == 0);
    REQUIRE(log->constructs == 0);
  }
  REQUIRE(log->destroys == 0);
  REQUIRE(live_value::destroyed == 6);
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "two independent allocators keep their allocation ownership separate",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto a = heim::allocate_unique<int>(tracking_allocator<int>{log, 100}, 1);
    auto b = heim::allocate_unique<int>(tracking_allocator<int>{log, 200}, 2);
    REQUIRE(log->outstanding.size() == 2);
    a.reset();
    REQUIRE(log->wrong_deallocations == 0);
    REQUIRE(log->outstanding.size() == 1);
    REQUIRE(*b == 2);
  }
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "move assignment preserves the correct allocator for each deallocation",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto a = heim::allocate_unique<int>(tracking_allocator<int>{log, 100}, 1);
    auto b = heim::allocate_unique<int>(tracking_allocator<int>{log, 200}, 2);
    b = std::move(a);
    REQUIRE(a == nullptr);
    REQUIRE(*b == 1);
    REQUIRE(log->deallocations == 1);
    REQUIRE(log->wrong_deallocations == 0);
  }
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "array move assignment preserves the correct allocator and length",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto a = heim::allocate_unique<int[]>(tracking_allocator<int>{log, 100}, 9);
    auto b = heim::allocate_unique<int[]>(tracking_allocator<int>{log, 200}, 2);
    a[5] = 94;
    b = std::move(a);
    REQUIRE(a == nullptr);
    REQUIRE(b[5] == 94);
    REQUIRE(log->deallocations == 1);
    REQUIRE(log->wrong_deallocations == 0);
  }
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "cross-allocator swap preserves the allocator bound to its pointer",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto a = heim::allocate_unique<int[]>(tracking_allocator<int>{log, 100}, 9);
    auto b = heim::allocate_unique<int[]>(tracking_allocator<int>{log, 200}, 2);
    a[5] = 59;
    b[1] = 12;
    a.swap(b);
    REQUIRE(a[1] == 12);
    REQUIRE(b[5] == 59);
  }
  REQUIRE(balanced(*log));
}

TEST_CASE(
    "reset and release preserve stateful allocator accounting",
    "[common][unique_allocated_ptr][allocator]")
{
  auto log = std::make_shared<ledger>();
  {
    auto p = heim::allocate_unique<int>(tracking_allocator<int>{log, 31}, 2);
    auto deleter = p.get_deleter();
    int *raw = p.release();
    REQUIRE(log->deallocations == 0);
    deleter(raw);
    REQUIRE(log->deallocations == 1);
    p.reset();
    REQUIRE(log->deallocations == 1);
  }
  REQUIRE(balanced(*log));
}
