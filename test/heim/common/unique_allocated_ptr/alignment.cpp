#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>

namespace {
struct alignas(128) over_aligned
{
  int value{};
  explicit over_aligned(int n = 0) : value{n} { }
};

constexpr bool correctly_aligned(void const *p)
{
  return reinterpret_cast<std::uintptr_t>(p) % alignof(over_aligned) == 0;
}
}

TEST_CASE("ordinary single allocation honors over-aligned types", "[common][unique_allocated_ptr][alignment]")
{
  auto p = heim::allocate_unique<over_aligned>(std::allocator<over_aligned>{}, 9);
  REQUIRE(correctly_aligned(p.get()));
  REQUIRE(p->value == 9);
}

TEST_CASE("ordinary array allocation honors alignment for every element", "[common][unique_allocated_ptr][alignment][array]")
{
  auto p = heim::allocate_unique<over_aligned[]>(std::allocator<over_aligned>{}, 7);
  for (int i = 0; i != 7; ++i)
  {
    REQUIRE(correctly_aligned(std::addressof(p[i])));
    REQUIRE(p[i].value == 0);
  }
}

TEST_CASE("overwrite single allocation honors over-alignment", "[common][unique_allocated_ptr][alignment][overwrite]")
{
  auto p = heim::allocate_unique_for_overwrite<over_aligned>(std::allocator<over_aligned>{});
  REQUIRE(correctly_aligned(p.get()));
  p->value = 3;
  REQUIRE(p->value == 3);
}

TEST_CASE("overwrite array allocation honors alignment for every element", "[common][unique_allocated_ptr][alignment][array][overwrite]")
{
  auto p = heim::allocate_unique_for_overwrite<over_aligned[]>(std::allocator<over_aligned>{}, 7);
  for (int i = 0; i != 7; ++i)
  {
    REQUIRE(correctly_aligned(std::addressof(p[i])));
    p[i].value = i + 5;
  }
  REQUIRE(p[6].value == 11);
}

TEST_CASE("overwrite factories accept non-default-constructible allocators", "[common][unique_allocated_ptr][overwrite]")
{
  auto p = heim::allocate_unique_for_overwrite<int>(heim_test::allocation::explicit_allocator<int>{17});
  *p = 8;
  REQUIRE(*p == 8);

  auto q = heim::allocate_unique_for_overwrite<int[]>(heim_test::allocation::explicit_allocator<int>{20}, 3);
  q[0] = 10;
  q[1] = 11;
  q[2] = 12;
  REQUIRE(q[0] + q[1] + q[2] == 33);
}
