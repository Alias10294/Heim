#include <array>
#include <forward_list>
#include <sstream>

#include "support.hpp"

using namespace heim_test::sparse_index;

template<typename I>
void indexed_construction()
{
  std::array<unsigned, 7> const keys{10, 0, 16, 5, 10, 3, 16};
  I idx{keys};
  check(idx, 10u, 4u);
  check(idx, 0u, 1u);
  check(idx, 16u, 6u);
  check(idx, 5u, 3u);
  check(idx, 3u, 5u);
  check_missing(idx, 1u);
  check_missing(idx, 15u);
}

TEST_CASE(
    "paged range constructor indexes positions with last occurrence winning",
    "[common][sparse_index][range]")
{
  indexed_construction<index_type<unsigned, 8>>();
}

TEST_CASE(
    "contiguous range constructor indexes positions with last occurrence winning",
    "[common][sparse_index][range]")
{
  indexed_construction<index_type<unsigned, 0>>();
}

TEST_CASE("page-size-one range constructor handles duplicates", "[common][sparse_index][range]")
{
  indexed_construction<index_type<unsigned, 1>>();
}

TEST_CASE("paged index accepts an empty range", "[common][sparse_index][range]")
{
  std::vector<unsigned> none;
  index_type<unsigned, 8> idx{none};
  check_missing(idx, 0u);
  check_missing(idx, 123u);
}

TEST_CASE("contiguous index accepts an empty range", "[common][sparse_index][range]")
{
  std::vector<unsigned> none;
  index_type<unsigned, 0> idx{none};
  check_missing(idx, 0u);
  check_missing(idx, 123u);
}

TEST_CASE("paged index accepts a non-random-access forward range", "[common][sparse_index][range]")
{
  std::forward_list<unsigned> keys{9, 4, 12, 9};
  index_type<unsigned, 8> idx{keys};
  check(idx, 9u, 3u);
  check(idx, 4u, 1u);
  check(idx, 12u, 2u);
}

TEST_CASE("contiguous index accepts a non-random-access forward range", "[common][sparse_index][range]")
{
  std::forward_list<unsigned> keys{9, 4, 12, 9};
  index_type<unsigned, 0> idx{keys};
  check(idx, 9u, 3u);
  check(idx, 4u, 1u);
  check(idx, 12u, 2u);
}

TEST_CASE("paged index accepts a genuine single-pass input range", "[common][sparse_index][range]")
{
  std::istringstream input{"12 3 12 0 7"};
  auto keys = std::ranges::istream_view<unsigned>(input);
  index_type<unsigned, 8> idx{keys};
  check(idx, 12u, 2u);
  check(idx, 3u, 1u);
  check(idx, 0u, 3u);
  check(idx, 7u, 4u);
}

TEST_CASE("contiguous index accepts a genuine single-pass input range", "[common][sparse_index][range]")
{
  std::istringstream input{"12 3 12 0 7"};
  auto keys = std::ranges::istream_view<unsigned>(input);
  index_type<unsigned, 0> idx{keys};
  check(idx, 12u, 2u);
  check(idx, 3u, 1u);
  check(idx, 0u, 3u);
  check(idx, 7u, 4u);
}

template<typename I>
void resetting_range()
{
  std::array<unsigned, 3> first{1, 5, 10};
  std::array<unsigned, 5> second{9, 5, 9, 2, 0};
  I idx{first};
  check(idx, 1u, 0u);
  check(idx, 5u, 1u);
  check(idx, 10u, 2u);
  idx.reset(second);
  check_missing(idx, 1u);
  check_missing(idx, 10u);
  check(idx, 9u, 2u);
  check(idx, 5u, 1u);
  check(idx, 2u, 3u);
  check(idx, 0u, 4u);
  idx.reset(std::array<unsigned, 0>{});
  check_missing(idx, 9u);
  check_missing(idx, 0u);
  idx.reset(first);
  check(idx, 1u, 0u);
  check_missing(idx, 9u);
}

TEST_CASE("paged reset reindexes its input and drops old keys", "[common][sparse_index][range]")
{
  resetting_range<index_type<unsigned, 8>>();
}

TEST_CASE("contiguous reset reindexes its input and drops old keys", "[common][sparse_index][range]")
{
  resetting_range<index_type<unsigned, 0>>();
}

TEST_CASE(
    "range constructor with an explicit allocator preserves allocator identity",
    "[common][sparse_index][range]")
{
  auto state = std::make_shared<allocation_log>();
  tracked_allocator<unsigned> alloc{state, 7};
  std::vector<unsigned> keys{5, 12, 20};
  observed_index<8> a{keys, alloc};
  observed_index<0> b{keys, alloc};
  REQUIRE(a.get_allocator() == alloc);
  REQUIRE(b.get_allocator() == alloc);
  check(a, 20u, 2u);
  check(b, 20u, 2u);
}

TEST_CASE("reset works repeatedly with a single-pass input range", "[common][sparse_index][range]")
{
  index_type<unsigned, 8> idx{std::allocator<unsigned>{}};
  for (unsigned round = 0; round < 8; ++round)
  {
    std::istringstream input{round % 2 ? "3 4 3 10" : "1 2 1 8"};
    auto rg = std::ranges::istream_view<unsigned>(input);
    idx.reset(rg);
    if (round % 2)
    {
      check(idx, 3u, 2u);
      check(idx, 4u, 1u);
      check(idx, 10u, 3u);
      check_missing(idx, 1u);
    }
    else
    {
      check(idx, 1u, 2u);
      check(idx, 2u, 1u);
      check(idx, 8u, 3u);
      check_missing(idx, 3u);
    }
  }
}
