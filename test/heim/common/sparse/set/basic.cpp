#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set initially contains no keys", "[common][sparse_set][basic]")
{
  hs::set<> s;
  REQUIRE(s.empty());
  REQUIRE(s.size() == 0);
  REQUIRE(s.max_size() >= 1);
  REQUIRE(s.begin() == s.end());
  REQUIRE(s.cbegin() == s.cend());
  REQUIRE(s.rbegin() == s.rend());
  REQUIRE_FALSE(s.contains(0u));
  REQUIRE(s.find(0u) == s.end());
  REQUIRE_FALSE(s.try_erase(0u));
}

TEST_CASE("sparse_set accepts zero and non-contiguous keys", "[common][sparse_set][basic]")
{
  hs::set<> s;
  for (auto k : {0u, 1u, 7u, 8u, 9u, 63u, 64u, 10000u})
    REQUIRE(s.insert(k).second);
  hs::verify(s, {0u, 1u, 7u, 8u, 9u, 63u, 64u, 10000u});
  REQUIRE_FALSE(s.contains(10u));
  REQUIRE_FALSE(s.contains(9999u));
}

TEST_CASE("sparse_set can clear empty and populated collections repeatedly", "[common][sparse_set][basic]")
{
  hs::set<> s;
  s.clear();
  s.clear();
  s.insert({1u, 20u, 300u});
  s.clear();
  hs::verify(s, {});
  s.insert(20u);
  s.clear();
  s.clear();
  hs::verify(s, {});
}

TEST_CASE("sparse_set clearing removes all holes and permits reinsertion", "[common][sparse_set][basic]")
{
  hs::set<> s{1u, 15u, 31u};
  REQUIRE(s.try_erase(15u));
  s.clear();
  hs::verify(s, {});
  REQUIRE(s.insert(15u).second);
  REQUIRE(s.insert(1u).second);
  hs::verify(s, {1u, 15u});
}

TEST_CASE("sparse_set const lookup and mutable lookup agree", "[common][sparse_set][basic]")
{
  hs::set<> s{7u, 1u, 23u};
  auto const &cs = s;
  for (unsigned k = 0; k != 30; ++k)
  {
    REQUIRE(s.contains(k) == cs.contains(k));
    REQUIRE((s.find(k) != s.end()) == (cs.find(k) != cs.end()));
  }
}

TEST_CASE("sparse_set repeated key lookup never changes the collection", "[common][sparse_set][basic]")
{
  hs::set<> s{0u, 7u, 27u};
  for (int i = 0; i < 1000; ++i)
  {
    REQUIRE(s.contains(27u));
    REQUIRE_FALSE(s.contains(26u));
    REQUIRE(*s.find(7u) == 7u);
  }
  hs::verify(s, {0u, 7u, 27u});
}

TEST_CASE(
    "sparse_set paged and contiguous backends have the same membership",
    "[common][sparse_set][basic]")
{
  hs::set<unsigned, 0> contiguous;
  hs::set<unsigned, 1> paged;
  for (auto key : {1u, 3u, 19u, 23u, 199u})
  {
    contiguous.insert(key);
    paged.insert(key);
  }
  hs::verify(contiguous, {1u, 3u, 19u, 23u, 199u});
  hs::verify(paged, {1u, 3u, 19u, 23u, 199u});
  REQUIRE(contiguous.try_erase(19u) == paged.try_erase(19u));
  hs::verify(contiguous, {1u, 3u, 23u, 199u});
  hs::verify(paged, {1u, 3u, 23u, 199u});
}

TEST_CASE("sparse_set can reuse a key after multiple deletions", "[common][sparse_set][basic]")
{
  hs::set<> s;
  for (unsigned i = 0; i < 100; ++i)
  {
    REQUIRE(s.insert(51u).second);
    REQUIRE(s.contains(51u));
    REQUIRE(s.try_erase(51u));
    REQUIRE_FALSE(s.contains(51u));
  }
  REQUIRE(s.empty());
}
