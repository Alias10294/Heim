#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map strings preserve content across erasure", "[common][sparse_map][mapped]")
{
  map<unsigned, std::string> m;
  REQUIRE(m.try_emplace(2u,"alpha").second);
  REQUIRE(m.try_emplace(10u,"beta").second);
  REQUIRE(m.try_emplace(18u,"gamma").second);
  REQUIRE(m.try_erase(10u));
  REQUIRE(m.at(2u) == "alpha");
  REQUIRE(m.at(18u) == "gamma");
  REQUIRE(m.try_emplace(10u,"delta").second);
  REQUIRE(m.at(10u) == "delta");
}

TEST_CASE("sparse_map insert_or_assign accepts string lvalues and rvalues", "[common][sparse_map][mapped]")
{
  map<unsigned, std::string> m;
  std::string a = "initial";
  REQUIRE(m.insert_or_assign(5u, a).second);
  REQUIRE_FALSE(m.insert_or_assign(5u, std::string{"next"}).second);
  REQUIRE(m.at(5u) == "next");
  REQUIRE(a == "initial");
}

TEST_CASE("sparse_map values may be move-only", "[common][sparse_map][mapped]")
{
  map<unsigned, std::unique_ptr<int>> m;
  REQUIRE(m.try_emplace(7u, std::make_unique<int>(70)).second);
  REQUIRE(m.try_emplace(11u, std::make_unique<int>(110)).second);
  REQUIRE(*m.at(7u) == 70);
  REQUIRE(*m.at(11u) == 110);
  REQUIRE(m.try_erase(7u));
  REQUIRE(*m.at(11u) == 110);
}

TEST_CASE(
    "sparse_map duplicate try_emplace does not consume move-only argument",
    "[common][sparse_map][mapped]")
{
  map<unsigned, std::unique_ptr<int>> m;
  m.try_emplace(1u, std::make_unique<int>(10));
  auto candidate = std::make_unique<int>(100);
  auto [it, created] = m.try_emplace(1u, std::move(candidate));
  REQUIRE_FALSE(created);
  REQUIRE(candidate);
  REQUIRE(*candidate == 100);
  REQUIRE(*it->second == 10);
}

TEST_CASE(
    "sparse_map insert_or_assign transfers ownership of move-only values",
    "[common][sparse_map][mapped]")
{
  map<unsigned, std::unique_ptr<int>> m;
  REQUIRE(m.insert_or_assign(1u, std::make_unique<int>(10)).second);
  REQUIRE_FALSE(m.insert_or_assign(1u, std::make_unique<int>(20)).second);
  REQUIRE(*m.at(1u) == 20);
}

TEST_CASE("sparse_map accepts non-default-constructible mapped objects", "[common][sparse_map][mapped]")
{
  struct no_default
  {
    int n;
    no_default() = delete;
    explicit no_default(int x) : n(x) { }
    no_default(no_default const&) = default;
    no_default(no_default && ) = default;
    no_default& operator= (no_default const&) = default;
    no_default& operator= (no_default && ) = default;
  };
  map<unsigned, no_default> m;
  REQUIRE(m.try_emplace(3u, 42).second);
  REQUIRE(m.at(3u).n == 42);
}

TEST_CASE(
    "sparse_map mapped values can be modified through structured bindings",
    "[common][sparse_map][mapped]")
{
  map<> m{{1u, 10}, {3u, 30}, {5u, 50}};
  for (auto [key, val]:m)
    val = -static_cast<int>(key);
  verify(m, {{1u, -1}, {3u, -3}, {5u, -5}});
}

TEST_CASE("sparse_map large mapped strings survive swap-pop", "[common][sparse_map][mapped]")
{
  map<unsigned, std::string> m;
  for (unsigned i = 0;i<70;++i)
    m.try_emplace(i*17u, std::string(200, static_cast<char>('a'+i%26)));
  for (unsigned i = 0;i<70;i += 2)
    REQUIRE(m.try_erase(i*17u));
  for (unsigned i = 1;i<70;i += 2) REQUIRE(m.at(i*17u)
    == std::string(200, static_cast<char>('a'+i%26)));
}

TEST_CASE("sparse_map move-only mapped values survive map movement", "[common][sparse_map][mapped]")
{
  map<unsigned, std::unique_ptr<int>> src;
  src.try_emplace(10u, std::make_unique<int>(42));
  src.try_emplace(20u, std::make_unique<int>(84));
  map<unsigned, std::unique_ptr<int>> dst{std::move(src)};
  REQUIRE(*dst.at(10u) == 42);
  REQUIRE(*dst.at(20u) == 84);
}

TEST_CASE("sparse_map mapped values may be aligned objects", "[common][sparse_map][mapped]")
{
  struct alignas(128) over_aligned
  {
    int value{};
    explicit over_aligned(int n = 0) : value(n) { }
  };
  map<unsigned, over_aligned> m;
  REQUIRE(m.try_emplace(3u, 99).second);
  REQUIRE(reinterpret_cast<std::uintptr_t>(std::addressof(m.at(3u))) % alignof(over_aligned) == 0);
  REQUIRE(m.at(3u).value == 99);
}

TEST_CASE(
    "sparse_map mapped object destructors run when erased and cleared",
    "[common][sparse_map][mapped]")
{
  struct tracked
  {
    int *count;
    explicit tracked(int *n) : count(n)
    {
      ++*count;
    }
    tracked(tracked && other) noexcept:count(std::exchange(other.count, nullptr)) { }
    tracked &operator= (tracked && other) noexcept
    { if (this != &other) { if (count) --*count;
    count = std::exchange(other.count, nullptr); } return *this; }
    ~tracked()
    {
      if (count)
        --*count;
    }
  };
  int count = 0;
  {
    map<unsigned, tracked> m;
    m.try_emplace(1u, &count);
    m.try_emplace(2u, &count);
    m.try_emplace(3u, &count);
    REQUIRE(count == 3);
    REQUIRE(m.try_erase(2u));
    REQUIRE(count == 2);
    m.clear();
    REQUIRE(count == 0);
  }
  REQUIRE(count == 0);
}
