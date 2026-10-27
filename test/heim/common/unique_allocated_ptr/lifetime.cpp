#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include <memory>
#include <utility>

namespace {
struct tracked {
  inline static int alive = 0;
  int value;
  explicit tracked(int v = 0) : value(v)
  {
    ++alive;
  }
  ~tracked()
  {
    --alive;
  }
  tracked(tracked const&) = delete;
  tracked& operator= (tracked const&) = delete;
};
}

TEST_CASE(
    "allocate_unique owns and destroys one allocated object",
    "[common][unique_allocated_ptr][allocator]")
{
  tracked::alive = 0;
  {
    auto p = heim::allocate_unique<tracked>(std::allocator<tracked>{}, 42);
    REQUIRE(p != nullptr);
    REQUIRE(p->value == 42);
    REQUIRE(tracked::alive == 1);
    auto moved = std::move(p);
    REQUIRE(p == nullptr);
    REQUIRE(moved->value == 42);
  }
  REQUIRE(tracked::alive == 0);
}

TEST_CASE(
    "allocate_unique allocates and destroys an array",
    "[common][unique_allocated_ptr][array][regression]")
{
  tracked::alive = 0;
  {
    auto p = heim::allocate_unique<tracked[]>(std::allocator<tracked>{}, 5);
    REQUIRE(tracked::alive == 5);
    for (int i = 0; i < 5; ++i)
      REQUIRE(p[i].value == 0);
  }
  REQUIRE(tracked::alive == 0);
}

TEST_CASE("zero-length allocated array is safe", "[common][unique_allocated_ptr][allocator]")
{
  auto p = heim::allocate_unique<int[]>(std::allocator<int>{}, 0);
  REQUIRE(p == nullptr);
}

TEST_CASE(
    "overwrite allocation correctly manages object lifetime",
    "[common][unique_allocated_ptr][allocator]")
{
  tracked::alive = 0;
  {
    auto p = heim::allocate_unique_for_overwrite<tracked>(std::allocator<tracked>{});
    REQUIRE(tracked::alive == 1);
    REQUIRE(p->value == 0);
  }
  REQUIRE(tracked::alive == 0);
}
