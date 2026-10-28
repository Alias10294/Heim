#include <concepts>
#include <memory>
#include <type_traits>

#include "support.hpp"

using namespace heim_test::sparse_index;

static_assert(index_type<unsigned, 0>::page_size == 0);
static_assert(!index_type<unsigned, 0>::is_paged);
static_assert(index_type<unsigned, 1>::page_size == 1);
static_assert(index_type<unsigned, 1>::is_paged);
static_assert(index_type<unsigned, 8>::page_size == 8);
static_assert(index_type<unsigned>::page_size == 8);
static_assert(index_type<unsigned, heim::default_page_size_v>::is_paged);
static_assert(heim::default_page_size_v > 0);

static_assert(std::same_as<typename index_type<unsigned, 8>::value_type, unsigned>);
static_assert(std::same_as<typename index_type<std::uint8_t, 8>::value_type, std::uint8_t>);
static_assert(std::same_as<typename index_type<std::uint16_t, 8>::value_type, std::uint16_t>);
static_assert(std::same_as<typename index_type<std::uint64_t, 8>::value_type, std::uint64_t>);
static_assert(std::same_as<typename index_type<unsigned, 8>::allocator_type, std::allocator<unsigned>>);

static_assert(std::same_as<decltype(std::declval<index_type<unsigned, 8> &>().get(0u)), unsigned *>);
static_assert(std::same_as<decltype(std::declval<index_type<unsigned, 8> const &>().get(0u)), unsigned const *>);
static_assert(std::same_as<decltype(std::declval<index_type<unsigned, 8> &>().assure(0u)), unsigned &>);
static_assert(std::same_as<decltype(std::declval<index_type<unsigned, 0> const &>().get(0u)), unsigned const *>);
static_assert(std::same_as<decltype(std::declval<index_type<unsigned, 0> &>().assure(0u)), unsigned &>);

static_assert(std::copy_constructible<index_type<unsigned, 8>>);
static_assert(std::move_constructible<index_type<unsigned, 8>>);
static_assert(std::is_copy_assignable_v<index_type<unsigned, 8>>);
static_assert(std::is_move_assignable_v<index_type<unsigned, 8>>);
static_assert(std::copy_constructible<index_type<unsigned, 0>>);
static_assert(std::is_copy_assignable_v<index_type<unsigned, 0>>);

static_assert(requires(index_type<unsigned, 8> &i)
{
  i.clear();
  i.get_allocator();
  i.max_size();
  i.swap(i);
});
static_assert(requires(index_type<unsigned, 0> &i)
{
  i.clear();
  i.get_allocator();
  i.max_size();
  i.swap(i);
});

TEST_CASE(
    "sparse_index exposes its documented paged and contiguous configurations",
    "[common][sparse_index][traits]")
{
  REQUIRE(index_type<unsigned, 8>::is_paged);
  REQUIRE_FALSE(index_type<unsigned, 0>::is_paged);
  REQUIRE(index_type<unsigned, 8>::page_size == 8);
  REQUIRE(index_type<unsigned, 0>::page_size == 0);
}

TEST_CASE(
    "sparse_index max_size cannot exceed its integral index representation",
    "[common][sparse_index][traits]")
{
  index_type<std::uint8_t, 0> a{std::allocator<std::uint8_t>{}};
  index_type<std::uint8_t, 8> b{std::allocator<std::uint8_t>{}};
  index_type<std::uint16_t, 0> c{std::allocator<std::uint16_t>{}};
  index_type<std::uint16_t, 8> d{std::allocator<std::uint16_t>{}};
  REQUIRE(a.max_size() <= std::numeric_limits<std::uint8_t>::max());
  REQUIRE(b.max_size() <= std::numeric_limits<std::uint8_t>::max());
  REQUIRE(c.max_size() <= std::numeric_limits<std::uint16_t>::max());
  REQUIRE(d.max_size() <= std::numeric_limits<std::uint16_t>::max());
  REQUIRE(a.max_size() >= 100);
  REQUIRE(b.max_size() >= 100);
}

TEST_CASE("sparse_index returns the configured allocator", "[common][sparse_index][traits]")
{
  auto log = std::make_shared<allocation_log>();
  auto alloc = tracked_allocator<unsigned>{log, 17};
  observed_index<8> a{alloc};
  observed_index<0> b{alloc};
  REQUIRE(a.get_allocator() == alloc);
  REQUIRE(b.get_allocator() == alloc);
}
