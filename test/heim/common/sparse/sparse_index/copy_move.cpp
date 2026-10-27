#include "support.hpp"

using namespace heim_test::sparse_index;

template<typename I>
void deep_copy()
{
  I source{typename I::allocator_type{}};
  source.assure(0u) = 10u;
  source.assure(100u) = 20u;
  source.assure(400u) = 30u;
  I copied{source};
  check(copied, 0u, 10u);
  check(copied, 100u, 20u);
  check(copied, 400u, 30u);
  REQUIRE(source.get(0u) != copied.get(0u));
  REQUIRE(source.get(100u) != copied.get(100u));
  copied.assure(0u) = 99u;
  source.assure(100u) = 88u;
  check(source, 0u, 10u);
  check(source, 100u, 88u);
  check(copied, 0u, 99u);
  check(copied, 100u, 20u);
  copied.clear();
  check(source, 400u, 30u);
}

TEST_CASE("paged copy constructs independent pages", "[common][sparse_index][copy]")
{ deep_copy<index<unsigned, 8>>(); }
TEST_CASE("contiguous copy constructs independent contents", "[common][sparse_index][copy]")
{ deep_copy<index<unsigned, 0>>(); }
TEST_CASE("one-element pages copy independently", "[common][sparse_index][copy]")
{ deep_copy<index<unsigned, 1>>(); }

template<typename I>
void copy_assignment()
{
  I source{typename I::allocator_type{}};
  source.assure(1u) = 5u;
  source.assure(31u) = 6u;
  I dest{typename I::allocator_type{}};
  dest.assure(2u) = 9u;
  dest.assure(777u) = 8u;
  dest = source;
  check(dest, 1u, 5u);
  check(dest, 31u, 6u);
  check_missing(dest, 2u);
  check_missing(dest, 777u);
  dest.assure(1u) = 35u;
  check(source, 1u, 5u);
  dest = dest;
  check(dest, 1u, 35u);
  check(dest, 31u, 6u);
  source.clear();
  check(dest, 1u, 35u);
}
TEST_CASE(
    "paged copy assignment replaces old pages and handles self assignment",
    "[common][sparse_index][copy]")
{ copy_assignment<index<unsigned, 8>>(); }
TEST_CASE(
    "contiguous copy assignment replaces old contents and handles self assignment",
    "[common][sparse_index][copy]")
{ copy_assignment<index<unsigned, 0>>(); }

TEST_CASE("paged copying an empty index releases the old destination pages", "[common][sparse_index][copy]")
{
  auto state = std::make_shared<allocation_log>();
  observed_index<8> empty{tracked_allocator<unsigned>{state}};
  observed_index<8> destination{tracked_allocator<unsigned>{state}};
  destination.assure(4u) = 10u;
  destination.assure(104u) = 20u;
  REQUIRE(state->live_pages() == 2);
  destination = empty;
  REQUIRE(state->live_pages() == 0);
  check_missing(destination, 4u);
  check_missing(destination, 104u);
}

TEST_CASE("paged copy constructor retains all holes", "[common][sparse_index][copy]")
{
  index<unsigned, 8> src{std::allocator<unsigned>{}};
  src.assure(1u) = 2u;
  src.assure(80u) = 3u;
  index<unsigned, 8> dst{src};
  check_missing(dst, 24u);
  check_missing(dst, 73u);
  check(dst, 80u, 3u);
}

TEST_CASE("paged copy constructor duplicates only materialized pages", "[common][sparse_index][copy]")
{
  auto state = std::make_shared<allocation_log>();
  observed_index<8> src{tracked_allocator<unsigned>{state}};
  src.assure(1u) = 9u;
  src.assure(800u) = 11u;
  REQUIRE(state->live_pages() == 2);
  observed_index<8> copied{src};
  REQUIRE(state->live_pages() == 4);
  check_missing(copied, 400u);
  check(copied, 800u, 11u);
}

template<typename I>
void move_tests()
{
  I source{typename I::allocator_type{}};
  source.assure(5u) = 50u;
  source.assure(99u) = 90u;
  I moved{std::move(source)};
  check(moved, 5u, 50u);
  check(moved, 99u, 90u);
  I dest{typename I::allocator_type{}};
  dest.assure(8u) = 8u;
  dest = std::move(moved);
  check(dest, 5u, 50u);
  check(dest, 99u, 90u);
  check_missing(dest, 8u);
  I &self = dest;
  dest = std::move(self);
  check(dest, 5u, 50u);
  check(dest, 99u, 90u);
}
TEST_CASE("paged index supports move construction and move assignment", "[common][sparse_index][move]")
{ move_tests<index<unsigned, 8>>(); }
TEST_CASE("contiguous index supports move construction and move assignment", "[common][sparse_index][move]")
{ move_tests<index<unsigned, 0>>(); }

TEST_CASE(
    "moving a paged index transfers page ownership without page allocation",
    "[common][sparse_index][move]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> a{tracked_allocator<unsigned>{state}};
    a.assure(10u) = 10u;
    a.assure(20u) = 20u;
    auto before = state->page_allocations;
    observed_index<8> b{std::move(a)};
    REQUIRE(state->page_allocations == before);
    check(b, 10u, 10u);
    check(b, 20u, 20u);
  }
  REQUIRE(state->balanced());
}

template<typename I>
void swap_tests()
{
  I lhs{typename I::allocator_type{}};
  I rhs{typename I::allocator_type{}};
  lhs.assure(1u) = 10u;
  rhs.assure(2u) = 20u;
  rhs.assure(8u) = 80u;
  using std::swap;
  swap(lhs, rhs);
  check(lhs, 2u, 20u);
  check(lhs, 8u, 80u);
  check_missing(lhs, 1u);
  check(rhs, 1u, 10u);
  check_missing(rhs, 2u);
  lhs.swap(rhs);
  check(lhs, 1u, 10u);
  check(rhs, 2u, 20u);
  lhs.swap(lhs);
  check(lhs, 1u, 10u);
}
TEST_CASE("paged index member and free swap exchange mappings", "[common][sparse_index][swap]")
{ swap_tests<index<unsigned, 8>>(); }
TEST_CASE("contiguous index member and free swap exchange mappings", "[common][sparse_index][swap]")
{ swap_tests<index<unsigned, 0>>(); }

TEST_CASE("copying and swapping preserve independent pages", "[common][sparse_index][copy][swap]")
{
  index<unsigned, 8> first{std::allocator<unsigned>{}};
  first.assure(1u) = 10u;
  index<unsigned, 8> second{first};
  index<unsigned, 8> third{std::allocator<unsigned>{}};
  third.assure(17u) = 20u;
  first.swap(third);
  check(second, 1u, 10u);
  check(first, 17u, 20u);
  check(third, 1u, 10u);
  second.assure(1u) = 100u;
  check(third, 1u, 10u);
}
