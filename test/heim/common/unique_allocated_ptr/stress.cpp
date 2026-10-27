#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <random>
#include <utility>

using heim_test::allocation::balanced;
using heim_test::allocation::ledger;
using heim_test::allocation::live_value;
using heim_test::allocation::tracking_allocator;

TEST_CASE("deterministic ownership operations never duplicate or leak single objects", "[common][unique_allocated_ptr][stress]")
{
  using pointer = heim::unique_allocated_ptr<live_value, tracking_allocator<live_value>, false>;
  auto log = std::make_shared<ledger>();
  live_value::reset();
  std::mt19937 random{0xC0FFEE};
  {
    std::array<pointer, 8> slots{};
    for (int step = 0; step != 600; ++step)
    {
      auto const i = static_cast<std::size_t>(random() % slots.size());
      auto const j = static_cast<std::size_t>(random() % slots.size());
      switch (random() % 4)
      {
        case 0:
          slots[i] = heim::allocate_unique<live_value>(tracking_allocator<live_value>{log, step % 5}, step);
          break;
        case 1:
          slots[i].reset();
          break;
        case 2:
          if (i != j) slots[i] = std::move(slots[j]);
          break;
        case 3:
          slots[i].swap(slots[j]);
          break;
      }
      int expected_live{};
      for (auto const &p : slots)
        if (p) ++expected_live;
      REQUIRE(live_value::alive == expected_live);
      REQUIRE(log->outstanding.size() == static_cast<std::size_t>(expected_live));
      REQUIRE(log->wrong_deallocations == 0);
    }
  }
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::constructed == live_value::destroyed);
  REQUIRE(balanced(*log));
}

TEST_CASE("deterministic array ownership operations preserve all extents", "[common][unique_allocated_ptr][stress][array]")
{
  using pointer = heim::unique_allocated_ptr<live_value[], tracking_allocator<live_value>, false>;
  auto log = std::make_shared<ledger>();
  live_value::reset();
  std::mt19937 random{0xBADC0DE};
  {
    std::array<pointer, 8> slots{};
    std::array<std::size_t, 8> lengths{};
    for (int step = 0; step != 500; ++step)
    {
      auto const i = static_cast<std::size_t>(random() % slots.size());
      auto const j = static_cast<std::size_t>(random() % slots.size());
      switch (random() % 4)
      {
        case 0:
        {
          auto const n = static_cast<std::size_t>(1 + random() % 23);
          slots[i] = heim::allocate_unique<live_value[]>(tracking_allocator<live_value>{log, step % 7}, n);
          lengths[i] = n;
          break;
        }
        case 1:
          slots[i].reset();
          lengths[i] = 0;
          break;
        case 2:
          if (i != j)
          {
            slots[i] = std::move(slots[j]);
            lengths[i] = std::exchange(lengths[j], 0u);
          }
          break;
        case 3:
          slots[i].swap(slots[j]);
          std::swap(lengths[i], lengths[j]);
          break;
      }
      std::size_t expected_live{};
      std::size_t expected_blocks{};
      for (std::size_t index = 0; index != slots.size(); ++index)
      {
        REQUIRE(static_cast<bool>(slots[index]) == (lengths[index] != 0));
        expected_live += lengths[index];
        if (slots[index]) ++expected_blocks;
      }
      REQUIRE(live_value::alive == static_cast<int>(expected_live));
      REQUIRE(log->outstanding.size() == expected_blocks);
      REQUIRE(log->wrong_deallocations == 0);
    }
  }
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::constructed == live_value::destroyed);
  REQUIRE(balanced(*log));
}

TEST_CASE("a sequence of allocation, release and manual deletion stays balanced", "[common][unique_allocated_ptr][stress]")
{
  auto log = std::make_shared<ledger>();
  live_value::reset();
  for (int i = 0; i != 100; ++i)
  {
    auto p = heim::allocate_unique<live_value>(tracking_allocator<live_value>{log, i % 4}, i);
    if (i % 2 == 0)
    {
      auto d = p.get_deleter();
      auto raw = p.release();
      d(raw);
    }
    else
      p.reset();
  }
  REQUIRE(live_value::constructed == 100);
  REQUIRE(live_value::destroyed == 100);
  REQUIRE(balanced(*log));
}
