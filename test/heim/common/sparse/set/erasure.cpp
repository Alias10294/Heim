#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set try_erase on absent key is a no-op", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 3u, 5u};
  REQUIRE_FALSE(s.try_erase(8u));
  REQUIRE_FALSE(s.try_erase(0u));
  hs::verify(s, {1u, 3u, 5u});
}

TEST_CASE("sparse_set erase removes a present key", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 3u, 5u};
  s.erase(3u); // erase(key) has a present-key precondition.
  hs::verify(s, {1u, 5u});
}

TEST_CASE("sparse_set try_erase returns true exactly once per membership", "[common][sparse_set][erasure]")
{
  hs::set<> s{17u};
  REQUIRE(s.try_erase(17u));
  REQUIRE_FALSE(s.try_erase(17u));
  REQUIRE(s.empty());
}

TEST_CASE("sparse_set deletion of first inserted key fixes dense lookup", "[common][sparse_set][erasure]")
{
  hs::set<> s{10u, 20u, 30u, 40u, 50u};
  REQUIRE(s.try_erase(10u));
  hs::verify(s, {20u, 30u, 40u, 50u});
}

TEST_CASE("sparse_set deletion of last inserted key fixes dense lookup", "[common][sparse_set][erasure]")
{
  hs::set<> s{10u, 20u, 30u, 40u, 50u};
  REQUIRE(s.try_erase(50u));
  hs::verify(s, {10u, 20u, 30u, 40u});
}

TEST_CASE("sparse_set deletion of middle key fixes relocated key index", "[common][sparse_set][erasure]")
{
  hs::set<> s{10u, 20u, 30u, 40u, 50u};
  REQUIRE(s.try_erase(30u));
  hs::verify(s, {10u, 20u, 40u, 50u});
  REQUIRE(s.insert(30u).second);
  hs::verify(s, {10u, 20u, 30u, 40u, 50u});
}

TEST_CASE("sparse_set repeated deletion of all keys leaves an empty set", "[common][sparse_set][erasure]")
{
  hs::set<> s;
  for (unsigned k = 0; k < 256; ++k)
    s.insert(k);
  for (unsigned k = 0; k < 256; k += 2)
    REQUIRE(s.try_erase(k));
  for (unsigned k = 255; k < 256; k -= 2)
  {
    REQUIRE(s.try_erase(k));
    if (k == 1)
      break;
  }
  hs::verify(s, {});
}

TEST_CASE("sparse_set erase(iterator) removes the pointed-to key", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 2u, 3u, 4u};
  auto it = s.find(2u);
  REQUIRE(it != s.end());
  s.erase(it);
  hs::verify(s, {1u, 3u, 4u});
}

TEST_CASE("sparse_set erase(begin) returns the new begin", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 2u, 3u};
  auto it = s.erase(s.begin());
  REQUIRE(it == s.begin());
  hs::verify(s, {1u, 2u});
}

TEST_CASE("sparse_set erase(last iterator) returns end", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 2u, 3u};
  auto it = s.erase(s.end() - 1);
  REQUIRE(it == s.end());
  hs::verify(s, {2u, 3u});
}

TEST_CASE("sparse_set erase(begin,end) can empty a set", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 2u, 3u, 4u, 5u, 6u};
  auto it = s.erase(s.begin(), s.end());
  REQUIRE(it == s.end());
  hs::verify(s, {});
}

TEST_CASE("sparse_set erase(empty range) does not mutate membership", "[common][sparse_set][erasure]")
{
  hs::set<> s{1u, 3u, 5u};
  auto it = s.erase(s.begin(), s.begin());
  REQUIRE(it == s.begin());
  hs::verify(s, {1u, 3u, 5u});
}

TEST_CASE("sparse_set alternating erase-by-iterator visits every survivor", "[common][sparse_set][erasure]")
{
  hs::set<> s;
  for (unsigned k = 0; k < 120; ++k)
    s.insert(k);
  for (auto it = s.begin(); it != s.end(); )
  {
    if (*it % 2 == 0)
      it = s.erase(it);
    else ++it;
  }
  REQUIRE(s.size() == 60);
  for (unsigned k = 0; k < 120; ++k) REQUIRE(s.contains(k)
    == (k % 2 != 0));
}

TEST_CASE("sparse_set erase from each page boundary preserves other keys", "[common][sparse_set][erasure]")
{
  hs::set<unsigned, 8> s;
  for (auto k : {7u, 8u, 15u, 16u, 23u, 24u, 31u, 32u})
    s.insert(k);
  for (auto k : {8u, 16u, 24u, 32u})
    REQUIRE(s.try_erase(k));
  hs::verify(s, {7u, 15u, 23u, 31u});
}
