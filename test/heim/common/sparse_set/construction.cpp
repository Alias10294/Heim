#include "support.hpp"
#include <sstream>

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set initializer list eliminates duplicates", "[common][sparse_set][construction]")
{
  hs::set<> s{4u, 3u, 4u, 9u, 3u, 2u};
  hs::verify(s, {2u, 3u, 4u, 9u});
}

TEST_CASE("sparse_set empty initializer list creates empty set", "[common][sparse_set][construction]")
{
  hs::set<> s{std::initializer_list<unsigned>{}};
  hs::verify(s, {});
}

TEST_CASE("sparse_set vector rvalue constructor removes duplicates", "[common][sparse_set][construction]")
{
  hs::set<> s{std::vector<unsigned>{9u, 8u, 9u, 8u, 7u}};
  hs::verify(s, {7u, 8u, 9u});
}

TEST_CASE("sparse_set vector lvalue allocator constructor preserves the input", "[common][sparse_set][construction]")
{
  std::vector<unsigned> input{3u, 2u, 3u, 5u};
  hs::set<> s{input, std::allocator<unsigned>{}};
  REQUIRE(input == std::vector<unsigned>{3u, 2u, 3u, 5u});
  hs::verify(s, {2u, 3u, 5u});
}

TEST_CASE("sparse_set from_range construction works with random access ranges", "[common][sparse_set][construction]")
{
  std::array<unsigned, 6> input{7u, 2u, 7u, 4u, 2u, 1u};
  hs::set<> s{std::from_range, input};
  hs::verify(s, {1u, 2u, 4u, 7u});
}

TEST_CASE("sparse_set from_range construction works with views", "[common][sparse_set][construction]")
{
  auto range = std::views::iota(1u, 40u) | std::views::filter([](unsigned v){return v % 5 == 0;});
  hs::set<> s{std::from_range, range};
  hs::verify(s, {5u, 10u, 15u, 20u, 25u, 30u, 35u});
}

TEST_CASE("sparse_set input-iterator construction accepts single-pass streams", "[common][sparse_set][construction]")
{
  std::istringstream input{"5 7 5 11 1 7"};
  hs::set<> s{std::istream_iterator<unsigned>{input}, std::istream_iterator<unsigned>{}};
  hs::verify(s, {1u, 5u, 7u, 11u});
}

TEST_CASE("sparse_set unique-tag initializer list honors distinct input", "[common][sparse_set][construction]")
{
  hs::set<> s{heim::unique, {8u, 4u, 11u, 3u}};
  hs::verify(s, {3u, 4u, 8u, 11u});
}

TEST_CASE("sparse_set unique-tag iterator construction honors distinct input", "[common][sparse_set][construction]")
{
  std::vector<unsigned> input{3u, 6u, 9u, 12u};
  hs::set<> s{heim::unique, input.begin(), input.end()};
  hs::verify(s, {3u, 6u, 9u, 12u});
}

TEST_CASE("sparse_set unique-tag container constructor honors distinct input", "[common][sparse_set][construction]")
{
  hs::set<> s{heim::unique, std::vector<unsigned>{1u, 19u, 15u}};
  hs::verify(s, {1u, 15u, 19u});
}

TEST_CASE("sparse_set assignment from initializer list replaces membership", "[common][sparse_set][construction]")
{
  hs::set<> s{10u, 20u, 30u};
  s = {8u, 8u, 9u, 12u};
  hs::verify(s, {8u, 9u, 12u});
  s = {};
  hs::verify(s, {});
}

TEST_CASE("sparse_set iterator-range construction handles zero and one element", "[common][sparse_set][construction]")
{
  std::vector<unsigned> empty;
  hs::set<> a{empty.begin(), empty.end()};
  hs::verify(a, {});
  std::vector<unsigned> one{47u};
  hs::set<> b{one.begin(), one.end()};
  hs::verify(b, {47u});
}
