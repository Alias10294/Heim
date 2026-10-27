#include "support.hpp"

namespace hs = heim_test::sparse_set;

namespace {
template<std::size_t Page>
void run_model(std::uint32_t seed, unsigned steps, unsigned universe)
{
  hs::set<unsigned, Page> actual;
  std::unordered_set<unsigned> oracle;
  hs::rng random{seed};
  for (unsigned step = 0; step < steps; ++step)
  {
    unsigned const key = random.bounded(universe);
    unsigned const op = random.bounded(10);
    INFO("page=" << Page << " seed=" << seed << " step=" << step << " op=" << op << " key=" << key);
    if (op < 4)
      REQUIRE(actual.insert(key).second == oracle.insert(key).second);
    else if (op < 7)
      REQUIRE(actual.try_erase(key) == (oracle.erase(key) != 0));
    else if (op == 7)
    {
      auto [at, added] = actual.try_emplace(key);
      REQUIRE(added == oracle.insert(key).second);
      REQUIRE(*at == key);
    }
    else if (op == 8)
    {
      auto pos = actual.find(key);
      REQUIRE((pos != actual.end()) == oracle.contains(key));
    }
    else
    {
      REQUIRE(actual.contains(key) == oracle.contains(key));
    }
    if (step % 89 == 0)
    {
      hs::verify(actual, oracle);
      hs::verify_dense_iterators(actual);
    }
  }
  hs::verify(actual, oracle);
}

template<std::size_t Page>
void run_replace_model(std::uint32_t seed)
{
  hs::set<unsigned, Page> actual;
  std::unordered_set<unsigned> oracle;
  hs::rng random{seed};
  for (unsigned round = 0; round < 100; ++round)
  {
    INFO("page=" << Page << " seed=" << seed << " round=" << round);
    std::vector<unsigned> input;
    for (unsigned k = 0; k < 30; ++k)
      input.push_back(random.bounded(201));
    actual.replace(std::move(input));
    oracle.clear();
    // Regenerate the same source deterministically for the independent oracle.
    hs::rng replay{seed};
    for (unsigned previous = 0; previous < round; ++previous)
      for (unsigned k = 0; k < 30; ++k) (void)replay.next();
    for (unsigned k = 0; k < 30; ++k)
      oracle.insert(replay.bounded(201));
    hs::verify(actual, oracle);
  }
}
}

TEST_CASE("sparse_set paged model regression with page size 1", "[common][sparse_set][model]")
{ run_model<1>(0x11223344u, 9000, 1200); }

TEST_CASE("sparse_set paged model regression with page size 7", "[common][sparse_set][model]")
{ run_model<7>(0x5E7A42u, 11000, 1200); }

TEST_CASE("sparse_set paged model regression with page size 8", "[common][sparse_set][model]")
{ run_model<8>(0xABCD1234u, 11000, 1200); }

TEST_CASE("sparse_set paged model regression with page size 64", "[common][sparse_set][model]")
{ run_model<64>(0x12345678u, 10000, 1200); }

TEST_CASE("sparse_set contiguous model regression", "[common][sparse_set][model]")
{ run_model<0>(0xDEADBEEFu, 11000, 1200); }

TEST_CASE("sparse_set page size 8 random replacement deduplicates every round", "[common][sparse_set][model]")
{ run_replace_model<8>(0x5AFEFEE1u); }

TEST_CASE("sparse_set contiguous random replacement deduplicates every round", "[common][sparse_set][model]")
{ run_replace_model<0>(0x99881122u); }

TEST_CASE("sparse_set destructive churn preserves exact membership", "[common][sparse_set][model]")
{
  hs::set<> s;
  for (unsigned round = 0; round < 20; ++round)
  {
    for (unsigned k = 0; k < 700; ++k) REQUIRE(s.insert(k).second);
    for (unsigned k = 0; k < 700; ++k) REQUIRE(s.try_erase(k));
    REQUIRE(s.empty());
    REQUIRE(s.begin() == s.end());
  }
}

TEST_CASE("sparse_set shuffled insertion and shuffled removal leave empty set", "[common][sparse_set][model]")
{
  hs::rng rng{0xBB11CC22u};
  hs::set<> s;
  std::vector<unsigned> ids(500);
  std::iota(ids.begin(), ids.end(), 0u);
  for (unsigned round = 0; round < 10; ++round)
  {
    for (std::size_t i = ids.size(); i > 1; --i)
      std::swap(ids[i-1], ids[rng.bounded(static_cast<std::uint32_t>(i))]);
    for (auto key : ids) REQUIRE(s.insert(key).second);
    for (std::size_t i = ids.size(); i > 1; --i)
      std::swap(ids[i-1], ids[rng.bounded(static_cast<std::uint32_t>(i))]);
    for (auto key : ids) REQUIRE(s.try_erase(key));
    REQUIRE(s.empty());
  }
}

TEST_CASE("sparse_set independent paged and contiguous implementations agree during mutations", "[common][sparse_set][model]")
{
  hs::set<unsigned, 0> contiguous;
  hs::set<unsigned, 8> paged;
  hs::rng r{0xFBADCAFEu};
  std::unordered_set<unsigned> oracle;
  for (unsigned i = 0; i < 12000; ++i)
  {
    auto key = r.bounded(2000);
    if (r.bounded(3) == 0)
    {
      auto removed = (oracle.erase(key) != 0);
      REQUIRE(contiguous.try_erase(key) == removed);
      REQUIRE(paged.try_erase(key) == removed);
    }
    else
    {
      auto inserted = oracle.insert(key).second;
      REQUIRE(contiguous.insert(key).second == inserted);
      REQUIRE(paged.insert(key).second == inserted);
    }
    if (i % 113 == 0)
    {
      hs::verify(paged, oracle);
      hs::verify(contiguous, oracle);
    }
  }
}
