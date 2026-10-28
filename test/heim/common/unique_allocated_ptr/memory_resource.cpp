#include <cstddef>
#include <memory_resource>

#include <catch2/catch_test_macros.hpp>

#include <heim/common/unique_allocated_ptr.hpp>

#include "support.hpp"

namespace
{
class recording_resource final : public std::pmr::memory_resource
{
public:
  std::size_t last_bytes{};
  std::size_t last_alignment{};
  int allocations{};
  int deallocations{};
  int mismatches{};

private:
  void *do_allocate(std::size_t bytes, std::size_t alignment) override
  {
    ++allocations;
    last_bytes = bytes;
    last_alignment = alignment;
    return std::pmr::new_delete_resource()->allocate(bytes, alignment);
  }

  void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) override
  {
    ++deallocations;
    if (bytes != last_bytes || alignment != last_alignment)
      ++mismatches;
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }

  bool do_is_equal(std::pmr::memory_resource const &other) const noexcept override
  { return this == &other; }
};
}

TEST_CASE("allocate_unique accepts std::pmr::polymorphic_allocator", "[common][unique_allocated_ptr][pmr]")
{
  recording_resource resource;
  {
    auto p = heim::allocate_unique<int>(std::pmr::polymorphic_allocator<int>{&resource}, 8);
    REQUIRE(*p == 8);
    REQUIRE(resource.allocations == 1);
    REQUIRE(resource.last_bytes == sizeof(int));
  }
  REQUIRE(resource.deallocations == 1);
  REQUIRE(resource.mismatches == 0);
}

TEST_CASE(
    "allocate_unique arrays request the expected PMR allocation size",
    "[common][unique_allocated_ptr][pmr][array]")
{
  recording_resource resource;
  {
    auto p = heim::allocate_unique<int[]>(std::pmr::polymorphic_allocator<int>{&resource}, 13);
    REQUIRE(resource.allocations == 1);
    REQUIRE(resource.last_bytes == 13 * sizeof(int));
    REQUIRE(p[0] == 0);
  }
  REQUIRE(resource.deallocations == 1);
  REQUIRE(resource.mismatches == 0);
}

TEST_CASE(
    "overwrite allocations use PMR resource and matching deallocation",
    "[common][unique_allocated_ptr][pmr][overwrite]")
{
  recording_resource resource;
  {
    auto p = heim::allocate_unique_for_overwrite<int>(std::pmr::polymorphic_allocator<int>{&resource});
    *p = 28;
    REQUIRE(*p == 28);
  }
  REQUIRE(resource.allocations == 1);
  REQUIRE(resource.deallocations == 1);
  REQUIRE(resource.mismatches == 0);
}

TEST_CASE(
    "overwrite arrays use PMR resource and correct byte count",
    "[common][unique_allocated_ptr][pmr][array][overwrite]")
{
  recording_resource resource;
  {
    auto p = heim::allocate_unique_for_overwrite<int[]>(std::pmr::polymorphic_allocator<int>{&resource}, 7);
    REQUIRE(resource.last_bytes == 7 * sizeof(int));
    p[2] = 19;
    REQUIRE(p[2] == 19);
  }
  REQUIRE(resource.deallocations == 1);
  REQUIRE(resource.mismatches == 0);
}

TEST_CASE(
    "zero-length PMR arrays do not call the memory resource",
    "[common][unique_allocated_ptr][pmr][array]")
{
  recording_resource resource;
  auto p = heim::allocate_unique<int[]>(std::pmr::polymorphic_allocator<int>{&resource}, 0);
  auto q = heim::allocate_unique_for_overwrite<int[]>(std::pmr::polymorphic_allocator<int>{&resource}, 0);
  REQUIRE_FALSE(p);
  REQUIRE_FALSE(q);
  REQUIRE(resource.allocations == 0);
  REQUIRE(resource.deallocations == 0);
}
