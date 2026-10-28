#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <heim/common/unique_allocated_ptr.hpp>

namespace
{
constexpr bool constexpr_single_works()
{
  auto p = heim::allocate_unique<int>(std::allocator<int>{}, 3);
  *p += 9;
  return *p == 12;
}

constexpr bool constexpr_array_works()
{
  auto p = heim::allocate_unique<int[]>(std::allocator<int>{}, 5);
  p[0] = 3;
  p[4] = 8;
  return p[0] == 3 && p[1] == 0 && p[4] == 8;
}

static_assert(constexpr_single_works());
static_assert(constexpr_array_works());
}

TEST_CASE(
    "ordinary allocator-aware allocations are usable in constant evaluation",
    "[common][unique_allocated_ptr][constexpr]")
{
  REQUIRE(constexpr_single_works());
  REQUIRE(constexpr_array_works());
}
