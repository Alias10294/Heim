#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set copy construction produces equal sets", "[common][sparse_set][lifecycle]")
{
  hs::set<> original{1u, 8u, 9u, 15u};
  hs::set<> copy{original};
  REQUIRE(original == copy);
  hs::verify(copy, {1u, 8u, 9u, 15u});
}

TEST_CASE("sparse_set copy construction gives independent membership", "[common][sparse_set][lifecycle]")
{
  hs::set<> original{1u, 8u, 9u, 15u};
  hs::set<> copy{original};
  REQUIRE(copy.try_erase(8u));
  REQUIRE(copy.insert(55u).second);
  hs::verify(original, {1u, 8u, 9u, 15u});
  hs::verify(copy, {1u, 9u, 15u, 55u});
}

TEST_CASE("sparse_set copy assignment overwrites a populated destination", "[common][sparse_set][lifecycle]")
{
  hs::set<> original{1u, 8u, 9u, 15u};
  hs::set<> copy{999u, 888u};
  copy = original;
  hs::verify(copy, {1u, 8u, 9u, 15u});
  REQUIRE(original == copy);
}

TEST_CASE("sparse_set self-copy assignment preserves membership", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 8u, 9u};
  auto &alias = s;
  s = alias;
  hs::verify(s, {1u, 8u, 9u});
}

TEST_CASE("sparse_set move construction preserves destination membership", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 8u, 9u};
  hs::set<> moved{std::move(s)};
  hs::verify(moved, {1u, 8u, 9u});
}

TEST_CASE("sparse_set move assignment replaces destination membership", "[common][sparse_set][lifecycle]")
{
  hs::set<> src{1u, 8u, 9u};
  hs::set<> dst{88u, 99u};
  dst = std::move(src);
  hs::verify(dst, {1u, 8u, 9u});
}

TEST_CASE("sparse_set self-move assignment remains valid", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 8u, 9u};
  auto &alias = s;
  s = std::move(alias);
  // Standard-container self-move assignment may leave a valid but unspecified value.
  std::unordered_set<unsigned> observed{s.begin(), s.end()};
  hs::verify(s, observed);
  REQUIRE(s.insert(123u).second);
  REQUIRE(s.contains(123u));
}

TEST_CASE("sparse_set swaps two populated sets", "[common][sparse_set][lifecycle]")
{
  hs::set<> a{1u, 3u};
  hs::set<> b{8u, 9u, 10u};
  using std::swap;
  swap(a, b);
  hs::verify(a, {8u, 9u, 10u});
  hs::verify(b, {1u, 3u});
}

TEST_CASE("sparse_set swaps an empty and populated set", "[common][sparse_set][lifecycle]")
{
  hs::set<> a;
  hs::set<> b{8u, 9u, 10u};
  a.swap(b);
  hs::verify(a, {8u, 9u, 10u});
  hs::verify(b, {});
}

TEST_CASE("sparse_set equality ignores insertion order", "[common][sparse_set][lifecycle]")
{
  hs::set<> a{2u, 3u, 8u};
  hs::set<> b{8u, 2u, 3u};
  REQUIRE(a == b);
  REQUIRE_FALSE(a != b);
}

TEST_CASE("sparse_set equality detects missing or additional key", "[common][sparse_set][lifecycle]")
{
  hs::set<> a{2u, 3u, 8u};
  hs::set<> b{2u, 3u, 10u};
  hs::set<> c{2u, 3u};
  REQUIRE_FALSE(a == b);
  REQUIRE_FALSE(a == c);
  REQUIRE_FALSE(c == a);
}

TEST_CASE("sparse_set replacement deduplicates a dense container", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 3u, 8u};
  s.replace(std::vector<unsigned>{2u, 2u, 7u, 8u, 7u});
  hs::verify(s, {2u, 7u, 8u});
}

TEST_CASE("sparse_set unique-tag replacement accepts already unique input", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 3u, 8u};
  s.replace(heim::unique, std::vector<unsigned>{2u, 7u, 9u});
  hs::verify(s, {2u, 7u, 9u});
}

TEST_CASE("sparse_set replacement by an empty container clears membership", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 3u, 8u};
  s.replace(std::vector<unsigned>{});
  hs::verify(s, {});
}

TEST_CASE("sparse_set replacement may reintroduce previously erased keys", "[common][sparse_set][lifecycle]")
{
  hs::set<> s{1u, 3u, 8u};
  REQUIRE(s.try_erase(3u));
  s.replace(std::vector<unsigned>{3u, 5u});
  hs::verify(s, {3u, 5u});
}
