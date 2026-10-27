#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace {
using heim_test::allocation::explicit_allocator;
using single_deleter = heim::allocator_aware_deleter<int, std::allocator<int>, false>;
using array_deleter = heim::allocator_aware_deleter<int[], std::allocator<int>, false>;
using overwrite_deleter = heim::allocator_aware_deleter<int, std::allocator<int>, true>;
using overwrite_array_deleter = heim::allocator_aware_deleter<int[], std::allocator<int>, true>;
using single_ptr = heim::unique_allocated_ptr<int, std::allocator<int>, false>;
using array_ptr = heim::unique_allocated_ptr<int[], std::allocator<int>, false>;

template<typename T>
concept allocatable_bounded_array = requires(std::allocator<std::remove_extent_t<T>> const &a) {
  heim::allocate_unique<T>(a);
};

template<typename T>
concept overwritable_bounded_array = requires(std::allocator<std::remove_extent_t<T>> const &a) {
  heim::allocate_unique_for_overwrite<T>(a);
};

static_assert(std::same_as<single_ptr, std::unique_ptr<int, single_deleter>>);
static_assert(std::same_as<array_ptr, std::unique_ptr<int[], array_deleter>>);
static_assert(std::same_as<single_deleter::pointer, int *>);
static_assert(std::same_as<array_deleter::pointer, int *>);
static_assert(!single_deleter::is_for_overwrite);
static_assert(!array_deleter::is_for_overwrite);
static_assert(overwrite_deleter::is_for_overwrite);
static_assert(overwrite_array_deleter::is_for_overwrite);
static_assert(std::default_initializable<single_deleter>);
static_assert(std::default_initializable<array_deleter>);
static_assert(!std::default_initializable<heim::allocator_aware_deleter<int, explicit_allocator<int>, false>>);
static_assert(!std::default_initializable<heim::allocator_aware_deleter<int[], explicit_allocator<int>, true>>);
static_assert(std::is_nothrow_invocable_v<single_deleter &, int *>);
static_assert(std::is_nothrow_invocable_v<array_deleter &, int *>);
static_assert(std::is_nothrow_invocable_v<overwrite_deleter &, int *>);
static_assert(std::is_nothrow_invocable_v<overwrite_array_deleter &, int *>);
static_assert(std::move_constructible<single_ptr>);
static_assert(std::move_constructible<array_ptr>);
static_assert(!std::copy_constructible<single_ptr>);
static_assert(!std::copy_constructible<array_ptr>);
static_assert(!allocatable_bounded_array<int[3]>);
static_assert(!overwritable_bounded_array<int[3]>);
}

TEST_CASE("unique_allocated_ptr exposes the expected unique_ptr interface", "[common][unique_allocated_ptr][traits]")
{
  auto p = heim::allocate_unique<int>(std::allocator<int>{}, 23);
  STATIC_REQUIRE(std::same_as<decltype(p)::element_type, int>);
  REQUIRE(p != nullptr);
  REQUIRE(*p == 23);
  REQUIRE(p.get() == std::addressof(*p));
}

TEST_CASE("allocator-aware deleters have the correct overwrite discriminator", "[common][unique_allocated_ptr][traits]")
{
  REQUIRE_FALSE(single_deleter::is_for_overwrite);
  REQUIRE_FALSE(array_deleter::is_for_overwrite);
  REQUIRE(overwrite_deleter::is_for_overwrite);
  REQUIRE(overwrite_array_deleter::is_for_overwrite);
}

TEST_CASE("single and array deleters accept null pointers", "[common][unique_allocated_ptr][traits]")
{
  single_deleter{}(nullptr);
  array_deleter{}(nullptr);
  overwrite_deleter{}(nullptr);
  overwrite_array_deleter{}(nullptr);
  SUCCEED();
}

TEST_CASE("explicit allocator construction works for both deleter specializations", "[common][unique_allocated_ptr][traits]")
{
  heim::allocator_aware_deleter<int, explicit_allocator<int>, false> d{explicit_allocator<int>{7}};
  heim::allocator_aware_deleter<int[], explicit_allocator<int>, true> a{explicit_allocator<int>{9}, 0};
  d(nullptr);
  a(nullptr);
  SUCCEED();
}
