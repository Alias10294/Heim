#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <cstddef>
#include <memory>
#include <new>
#include <vector>

using heim_test::allocation::construction_failure;
using heim_test::allocation::ledger;
using heim_test::allocation::live_value;
using heim_test::allocation::throwing_value;
using heim_test::allocation::tracking_allocator;

TEST_CASE("a failed single allocation does not leak", "[common][unique_allocated_ptr][exception]")
{
  auto log = std::make_shared<ledger>();
  log->fail_allocation_on = 1;
  REQUIRE_THROWS_AS(
      heim::allocate_unique<int>(tracking_allocator<int>{log}, 9), std::bad_alloc);
  REQUIRE(log->allocations == 1);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE(
    "a failed overwrite single allocation does not leak",
    "[common][unique_allocated_ptr][exception][overwrite]")
{
  auto log = std::make_shared<ledger>();
  log->fail_allocation_on = 1;
  REQUIRE_THROWS_AS(
      heim::allocate_unique_for_overwrite<int>(tracking_allocator<int>{log}), std::bad_alloc);
  REQUIRE(log->allocations == 1);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE("a failed array allocation does not leak", "[common][unique_allocated_ptr][exception][array]")
{
  auto log = std::make_shared<ledger>();
  log->fail_allocation_on = 1;
  REQUIRE_THROWS_AS(
      heim::allocate_unique<int[]>(tracking_allocator<int>{log}, 5), std::bad_alloc);
  REQUIRE(log->allocations == 1);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE(
    "a failed overwrite array allocation does not leak",
    "[common][unique_allocated_ptr][exception][array][overwrite]")
{
  auto log = std::make_shared<ledger>();
  log->fail_allocation_on = 1;
  REQUIRE_THROWS_AS(
      heim::allocate_unique_for_overwrite<int[]>(tracking_allocator<int>{log}, 5), std::bad_alloc);
  REQUIRE(log->allocations == 1);
  REQUIRE(log->deallocations == 0);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE(
    "a failing allocator construct releases single storage",
    "[common][unique_allocated_ptr][exception]")
{
  auto log = std::make_shared<ledger>();
  log->fail_construct_on = 1;
  live_value::reset();
  REQUIRE_THROWS_AS(
      heim::allocate_unique<live_value>(tracking_allocator<live_value>{log}, 1),
      construction_failure);
  REQUIRE(log->construct_attempts == 1);
  REQUIRE(log->constructs == 0);
  REQUIRE(log->destroys == 0);
  REQUIRE(live_value::alive == 0);
  REQUIRE(log->deallocations == 1);
  REQUIRE(log->wrong_deallocations == 0);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE("a failing single constructor releases its storage", "[common][unique_allocated_ptr][exception]")
{
  auto log = std::make_shared<ledger>();
  throwing_value::reset(1);
  REQUIRE_THROWS_AS(
      heim::allocate_unique<throwing_value>(tracking_allocator<throwing_value>{log}),
      construction_failure);
  REQUIRE(throwing_value::attempts == 1);
  REQUIRE(throwing_value::alive == 0);
  REQUIRE(throwing_value::destructions == 0);
  REQUIRE(log->deallocations == 1);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE(
    "a failing overwrite single constructor releases its storage",
    "[common][unique_allocated_ptr][exception][overwrite]")
{
  auto log = std::make_shared<ledger>();
  throwing_value::reset(1);
  REQUIRE_THROWS_AS(
      heim::allocate_unique_for_overwrite<throwing_value>(tracking_allocator<throwing_value>{log}),
      construction_failure);
  REQUIRE(throwing_value::attempts == 1);
  REQUIRE(throwing_value::alive == 0);
  REQUIRE(throwing_value::destructions == 0);
  REQUIRE(log->construct_attempts == 0);
  REQUIRE(log->deallocations == 1);
  REQUIRE(log->outstanding.empty());
}

TEST_CASE(
    "array rollback destroys only fully constructed elements in reverse order",
    "[common][unique_allocated_ptr][exception][array]")
{
  for (int failing_element = 1; failing_element <= 8; ++failing_element)
  {
    DYNAMIC_SECTION("failure on element " << failing_element)
    {
      auto log = std::make_shared<ledger>();
      throwing_value::reset(failing_element);
      REQUIRE_THROWS_AS(
          heim::allocate_unique<throwing_value[]>(tracking_allocator<throwing_value>{log}, 8),
          construction_failure);
      REQUIRE(throwing_value::attempts == failing_element);
      REQUIRE(throwing_value::alive == 0);
      REQUIRE(throwing_value::destructions == failing_element - 1);
      std::vector<int> expected;
      for (int id = failing_element - 1; id > 0; --id)
        expected.push_back(id);
      REQUIRE(throwing_value::destruction_order == expected);
      REQUIRE(log->deallocations == 1);
      REQUIRE(log->wrong_deallocations == 0);
      REQUIRE(log->outstanding.empty());
    }
  }
}

TEST_CASE(
    "overwrite array rollback destroys only fully constructed elements",
    "[common][unique_allocated_ptr][exception][array][overwrite]")
{
  for (int failing_element = 1; failing_element <= 8; ++failing_element)
  {
    DYNAMIC_SECTION("failure on element " << failing_element)
    {
      auto log = std::make_shared<ledger>();
      throwing_value::reset(failing_element);
      REQUIRE_THROWS_AS(
          heim::allocate_unique_for_overwrite<throwing_value[]>(tracking_allocator<throwing_value>{log}, 8),
          construction_failure);
      REQUIRE(throwing_value::attempts == failing_element);
      REQUIRE(throwing_value::alive == 0);
      REQUIRE(throwing_value::destructions == failing_element - 1);
      REQUIRE(log->construct_attempts == 0);
      REQUIRE(log->destroys == 0);
      std::vector<int> expected;
      for (int id = failing_element - 1; id > 0; --id)
        expected.push_back(id);
      REQUIRE(throwing_value::destruction_order == expected);
      REQUIRE(log->deallocations == 1);
      REQUIRE(log->wrong_deallocations == 0);
      REQUIRE(log->outstanding.empty());
    }
  }
}

TEST_CASE(
    "allocator hook failures roll back ordinary arrays completely",
    "[common][unique_allocated_ptr][exception][array]")
{
  for (int failing_element = 1; failing_element <= 8; ++failing_element)
  {
    DYNAMIC_SECTION("failure in allocator construct number " << failing_element)
    {
      auto log = std::make_shared<ledger>();
      log->fail_construct_on = failing_element;
      live_value::reset();
      REQUIRE_THROWS_AS(
          heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log}, 8),
          construction_failure);
      REQUIRE(log->construct_attempts == failing_element);
      REQUIRE(log->constructs == failing_element - 1);
      REQUIRE(log->destroys == failing_element - 1);
      REQUIRE(live_value::alive == 0);
      REQUIRE(live_value::destroyed == failing_element - 1);
      REQUIRE(log->deallocations == 1);
      REQUIRE(log->wrong_deallocations == 0);
      REQUIRE(log->outstanding.empty());
    }
  }
}

TEST_CASE(
    "successful construction after a previous injected failure does not poison allocator",
    "[common][unique_allocated_ptr][exception]")
{
  auto log = std::make_shared<ledger>();
  log->fail_construct_on = 1;
  REQUIRE_THROWS_AS(
      heim::allocate_unique<int>(tracking_allocator<int>{log}, 42),
      construction_failure);
  log->fail_construct_on = 0;
  {
    auto p = heim::allocate_unique<int>(tracking_allocator<int>{log}, 18);
    REQUIRE(*p == 18);
  }
  REQUIRE(log->allocations == 2);
  REQUIRE(log->deallocations == 2);
  REQUIRE(log->wrong_deallocations == 0);
  REQUIRE(log->outstanding.empty());
}
