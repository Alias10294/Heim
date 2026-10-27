#include "support.hpp"
#include <sstream>

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set insert reports whether a key is new", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  auto [p, inserted] = s.insert(14u);
  REQUIRE(inserted);
  REQUIRE(p != s.end());
  REQUIRE(*p == 14u);
  auto [q, repeated] = s.insert(14u);
  REQUIRE_FALSE(repeated);
  REQUIRE(*q == 14u);
  REQUIRE(p == q);
  hs::verify(s, {14u});
}

TEST_CASE("sparse_set try_emplace is idempotent", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  REQUIRE(s.try_emplace(9u).second);
  REQUIRE_FALSE(s.try_emplace(9u).second);
  REQUIRE(s.try_emplace(7u).second);
  hs::verify(s, {7u, 9u});
}

TEST_CASE("sparse_set emplace inserts a key absent from the set", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  auto result = s.emplace(22u); // emplace has a distinct-key precondition.
  REQUIRE(result.second);
  REQUIRE(*result.first == 22u);
  hs::verify(s, {22u});
}

TEST_CASE(
    "sparse_set insertion with begin hint locates the inserted key",
    "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u, 5u};
  auto it = s.insert(s.cbegin(), 8u);
  REQUIRE(*it == 8u);
  hs::verify(s, {1u, 3u, 5u, 8u});
}

TEST_CASE("sparse_set insertion with end hint locates the inserted key", "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u, 5u};
  auto it = s.insert(s.cend(), 8u);
  REQUIRE(*it == 8u);
  hs::verify(s, {1u, 3u, 5u, 8u});
}

TEST_CASE(
    "sparse_set insertion with middle hint locates the inserted key",
    "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u, 5u};
  auto it = s.insert(s.cbegin() + 1, 8u);
  REQUIRE(*it == 8u);
  hs::verify(s, {1u, 3u, 5u, 8u});
}

TEST_CASE(
    "sparse_set duplicate hinted insertion leaves membership unchanged",
    "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u, 5u};
  for (auto pos : {0, 1, 3})
  {
    auto it = s.insert(s.cbegin() + pos, 3u);
    REQUIRE(*it == 3u);
  }
  hs::verify(s, {1u, 3u, 5u});
}

TEST_CASE("sparse_set try_emplace_hint handles repeated keys", "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u};
  REQUIRE(*s.try_emplace_hint(s.cbegin(), 3u) == 3u);
  REQUIRE(*s.try_emplace_hint(s.cend(), 10u) == 10u);
  hs::verify(s, {1u, 3u, 10u});
}

TEST_CASE("sparse_set emplace_hint inserts a fresh key", "[common][sparse_set][insertion]")
{
  hs::set<> s{1u, 3u};
  REQUIRE(*s.emplace_hint(s.cbegin(), 7u) == 7u);
  hs::verify(s, {1u, 3u, 7u});
}

TEST_CASE(
    "sparse_set iterator-range insertion deduplicates within input",
    "[common][sparse_set][insertion]")
{
  hs::set<> s{3u};
  std::array<unsigned, 8> input{1u, 2u, 3u, 3u, 2u, 4u, 6u, 1u};
  s.insert(input.begin(), input.end());
  hs::verify(s, {1u, 2u, 3u, 4u, 6u});
}

TEST_CASE("sparse_set insert_range accepts filtered view", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  auto keys = std::views::iota(0u, 80u) | std::views::filter([](auto v)
  {
    return v % 9 == 0;
  });
  s.insert_range(keys);
  hs::verify(s, {0u, 9u, 18u, 27u, 36u, 45u, 54u, 63u, 72u});
}

TEST_CASE(
    "sparse_set insert from initializer list handles duplicates and empty",
    "[common][sparse_set][insertion]")
{
  hs::set<> s;
  s.insert({4u, 4u, 5u, 5u, 6u});
  s.insert({});
  hs::verify(s, {4u, 5u, 6u});
}

TEST_CASE("sparse_set inserts thousands of unique keys in reverse order", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  for (unsigned k = 2000; k != 0; --k)
    REQUIRE(s.insert(k - 1).second);
  REQUIRE(s.size() == 2000);
  for (unsigned k = 0; k < 2000; ++k)
    REQUIRE(s.contains(k));
  hs::verify_dense_iterators(s);
}

TEST_CASE("sparse_set insertion after many deletions restores every key", "[common][sparse_set][insertion]")
{
  hs::set<> s;
  for (unsigned k = 0; k < 300; ++k)
    s.insert(k);
  for (unsigned k = 0; k < 300; k += 2)
    REQUIRE(s.try_erase(k));
  for (unsigned k = 0; k < 300; k += 2)
    REQUIRE(s.insert(k).second);
  REQUIRE(s.size() == 300);
  for (unsigned k = 0; k < 300; ++k)
    REQUIRE(s.contains(k));
}
