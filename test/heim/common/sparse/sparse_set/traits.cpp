#include "support.hpp"
#include <concepts>
#include <iterator>
#include <type_traits>

namespace hs = heim_test::sparse_set;
using S = hs::set<>;

static_assert(std::same_as<S::key_type, unsigned>);
static_assert(std::same_as<S::value_type, unsigned>);
static_assert(std::same_as<S::reference, unsigned &>);
static_assert(std::same_as<S::const_reference, unsigned const &>);
static_assert(std::same_as<S::container_type, std::vector<unsigned>>);
static_assert(std::same_as<S::allocator_type, std::allocator<unsigned>>);
static_assert(std::same_as<S::size_type, std::size_t>);
static_assert(std::ranges::random_access_range<S>);
static_assert(std::ranges::random_access_range<S const>);
static_assert(std::ranges::sized_range<S const>);
static_assert(std::ranges::common_range<S const>);
static_assert(std::forward_iterator<S::iterator>);
static_assert(std::same_as<decltype(*std::declval<S &>().begin()), unsigned const &>);
static_assert(std::is_copy_constructible_v<S>);
static_assert(std::is_move_constructible_v<S>);
static_assert(std::is_copy_assignable_v<S>);
static_assert(std::is_move_assignable_v<S>);
static_assert(S::page_size == 8);
static_assert(hs::set<unsigned, 0>::page_size == 0);
static_assert(std::same_as<decltype(std::declval<S const &>().contains(0u)), bool>);
static_assert(std::same_as<decltype(std::declval<S &>().insert(0u)), std::pair<S::iterator, bool>>);
static_assert(std::same_as<decltype(std::declval<S &>().try_erase(0u)), bool>);
static_assert(std::same_as<decltype(std::declval<S && >().extract()), S::container_type>);

TEST_CASE("sparse_set exposes an immutable view of its keys", "[common][sparse_set][traits]")
{
  STATIC_REQUIRE(std::same_as<decltype(*std::declval<S &>().begin()), unsigned const &>);
  STATIC_REQUIRE(std::same_as<decltype(*std::declval<S const &>().begin()), unsigned const &>);
  STATIC_REQUIRE(std::same_as<decltype(std::declval<S const &>().find(1u)), S::const_iterator>);
}

TEST_CASE("sparse_set page sizes are compile-time configuration", "[common][sparse_set][traits]")
{
  STATIC_REQUIRE(hs::set<unsigned, 0>::page_size == 0);
  STATIC_REQUIRE(hs::set<unsigned, 1>::page_size == 1);
  STATIC_REQUIRE(hs::set<unsigned, 7>::page_size == 7);
  STATIC_REQUIRE(heim::sparse_set<unsigned>::page_size == heim::default_page_size_v);
}

TEST_CASE(
    "sparse_set has value and allocator types consistent with the container",
    "[common][sparse_set][traits]")
{
  STATIC_REQUIRE(std::same_as<S::container_type::value_type, S::value_type>);
  STATIC_REQUIRE(std::same_as<S::difference_type, S::container_type::difference_type>);
  STATIC_REQUIRE(std::same_as<S::reverse_iterator, S::container_type::const_iterator>);
}
