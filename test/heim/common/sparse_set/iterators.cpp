#include "support.hpp"
#include <iterator>

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set forward traversal is reverse of dense container", "[common][sparse_set][iterators]")
{
  hs::set<> s{1u, 7u, 9u, 2u};
  REQUIRE(std::vector<unsigned>(s.begin(), s.end()) == std::vector<unsigned>{2u, 9u, 7u, 1u});
  REQUIRE(std::vector<unsigned>(s.rbegin(), s.rend()) == std::vector<unsigned>{1u, 7u, 9u, 2u});
}

TEST_CASE("sparse_set const and nonconst iterators agree", "[common][sparse_set][iterators]")
{
  hs::set<> s{2u, 7u, 5u, 1u};
  auto const &cs = s;
  REQUIRE(std::ranges::equal(s, cs));
  REQUIRE(std::ranges::equal(s, std::ranges::subrange(cs.cbegin(), cs.cend())));
  REQUIRE(std::ranges::equal(std::ranges::subrange(s.rbegin(), s.rend()),
                             std::ranges::subrange(cs.crbegin(), cs.crend())));
}

TEST_CASE("sparse_set iterators support random access arithmetic", "[common][sparse_set][iterators]")
{
  hs::set<> s{1u, 2u, 3u, 4u, 5u};
  REQUIRE(s.end() - s.begin() == 5);
  REQUIRE(s.begin() + 5 == s.end());
  REQUIRE(s.end() - 5 == s.begin());
  REQUIRE(s.begin()[0] == 5u);
  REQUIRE(s.begin()[4] == 1u);
  REQUIRE(s.begin() < s.end());
  REQUIRE(s.begin() + 2 > s.begin());
}

TEST_CASE("sparse_set find returns an iterator into the same range", "[common][sparse_set][iterators]")
{
  hs::set<> s{11u, 22u, 33u};
  auto const &cs = s;
  REQUIRE(s.find(22u) == s.begin() + 1);
  REQUIRE(cs.find(22u) == cs.cbegin() + 1);
  REQUIRE(s.find(44u) == s.end());
  REQUIRE(cs.find(44u) == cs.end());
}

TEST_CASE("sparse_set every iterator position can be found by its key", "[common][sparse_set][iterators]")
{
  hs::set<> s;
  for (unsigned i = 0; i < 100; ++i) s.insert((i * 19u) % 127u);
  for (auto it = s.begin(); it != s.end(); ++it) REQUIRE(s.find(*it) == it);
  hs::verify_dense_iterators(s);
}

TEST_CASE("sparse_set lookup iterators remain valid after duplicate insertion", "[common][sparse_set][iterators]")
{
  hs::set<> s{8u, 9u, 10u};
  auto pos = s.find(9u);
  REQUIRE_FALSE(s.insert(9u).second);
  REQUIRE(pos == s.find(9u));
  REQUIRE(*pos == 9u);
}

TEST_CASE("sparse_set extract returns underlying dense order", "[common][sparse_set][iterators]")
{
  hs::set<> s{4u, 8u, 2u};
  auto dense = std::move(s).extract();
  REQUIRE(dense == std::vector<unsigned>{4u, 8u, 2u});
}

TEST_CASE("sparse_set iterator order remains consistent after erase and insertion", "[common][sparse_set][iterators]")
{
  hs::set<> s{1u, 2u, 3u, 4u};
  REQUIRE(s.try_erase(2u));
  REQUIRE(s.insert(6u).second);
  hs::verify_dense_iterators(s);
  hs::verify(s, {1u, 3u, 4u, 6u});
}

TEST_CASE("sparse_set range algorithms can consume const keys", "[common][sparse_set][iterators]")
{
  hs::set<> s{9u, 3u, 1u, 15u};
  REQUIRE(std::ranges::count_if(std::as_const(s), [](unsigned k){return k % 3 == 0;}) == 3);
  auto filtered = s | std::views::filter([](unsigned k){return k > 5;});
  std::unordered_set<unsigned> out;
  for (auto k : filtered) out.insert(k);
  REQUIRE(out == std::unordered_set<unsigned>{9u, 15u});
}
