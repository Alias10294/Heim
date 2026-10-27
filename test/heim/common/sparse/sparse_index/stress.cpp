#include "support.hpp"
#include <array>

using namespace heim_test::sparse_index;

template<typename I>
void run_model(std::uint32_t seed, unsigned steps, unsigned domain)
{
  using UInt = key_type<I>;
  REQUIRE(domain > 0);
  REQUIRE(domain <= std::numeric_limits<UInt>::max());

  I idx{typename I::allocator_type{}};
  std::unordered_map<UInt, UInt> model;
  rng rnd{seed};
  auto const size = static_cast<UInt>(domain);

  for (unsigned step = 0; step < steps; ++step)
  {
    auto const key = static_cast<UInt>(rnd.bounded(domain));
    auto const choice = rnd.bounded(16);
    if (choice < 9)
    {
      auto const value = static_cast<UInt>(rnd.bounded(domain + 1));
      set_and_model(idx, model, key, value);
    }
    else if (choice == 9)
    {
      set_and_model(idx, model, key, null_value<I>);
    }
    else if (choice == 10)
    {
      idx.clear();
      model.clear();
    }
    else if (choice == 11)
    {
      std::vector<UInt> keys;
      auto const n = rnd.bounded(20);
      for (unsigned i = 0; i < n; ++i)
        keys.push_back(static_cast<UInt>(rnd.bounded(domain)));
      idx.reset(keys);
      model.clear();
      for (std::size_t i = 0; i < keys.size(); ++i)
        model[keys[i]] = static_cast<UInt>(i);
    }
    else if (choice == 12)
    {
      I copy{idx};
      verify_against(copy, model, size);
      auto const new_value = static_cast<UInt>(rnd.bounded(domain));
      copy.assure(key) = new_value;
      verify_against(idx, model, size);
    }
    else if (choice == 13)
    {
      I assigned{typename I::allocator_type{}};
      assigned.assure(key) = static_cast<UInt>(rnd.bounded(domain));
      assigned = idx;
      verify_against(assigned, model, size);
    }
    else if (choice == 14)
    {
      I source{idx};
      I moved{std::move(source)};
      verify_against(moved, model, size);
    }
    else
    {
      idx = idx;
    }

    auto const probe = static_cast<UInt>(rnd.bounded(domain));
    auto const it = model.find(probe);
    if (it == model.end() || it->second == null_value<I>)
      check_missing(idx, probe);
    else
      check(idx, probe, it->second);

    if (step % 45 == 0)
      verify_against(idx, model, size);
  }
  verify_against(idx, model, size);
}

TEST_CASE("paged index matches a reference model across 4000 operations", "[common][sparse_index][stress]")
{ run_model<index<unsigned, 8>>(0x12345678, 4000, 192); }
TEST_CASE(
    "contiguous index matches a reference model across 4000 operations",
    "[common][sparse_index][stress]")
{ run_model<index<unsigned, 0>>(0x12345678, 4000, 192); }
TEST_CASE("one-element paged index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<unsigned, 1>>(0x31415926, 2000, 190); }
TEST_CASE("odd-sized paged index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<unsigned, 7>>(0x27182818, 2500, 190); }
TEST_CASE("wide paged index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<unsigned, 64>>(0xabcdef01, 2500, 190); }
TEST_CASE("uint8_t paged index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<std::uint8_t, 8>>(0xdeadbeef, 2000, 200); }
TEST_CASE("uint8_t contiguous index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<std::uint8_t, 0>>(0xdeadbeef, 2000, 200); }
TEST_CASE("uint16_t paged index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<std::uint16_t, 8>>(0x99887766, 3000, 250); }
TEST_CASE("uint16_t contiguous index matches a reference model", "[common][sparse_index][stress]")
{ run_model<index<std::uint16_t, 0>>(0x99887766, 3000, 250); }

TEST_CASE(
    "paged page-allocation count tracks distinct pages in a generated stream",
    "[common][sparse_index][stress][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    std::array<bool, 64> visited{};
    rng rnd{0xabcabcab};
    std::size_t pages{};
    for (unsigned i = 0; i < 2000; ++i)
    {
      auto const key = rnd.bounded(512);
      auto const page = key / 8;
      if (!visited[page])
      {
        visited[page] = true;
        ++pages;
      }
      idx.assure(key) = i;
      REQUIRE(state->live_pages() == pages);
      REQUIRE(state->page_allocations == pages);
    }
  }
  REQUIRE(state->balanced());
}

TEST_CASE("repeated deep copies do not share page storage", "[common][sparse_index][stress][copy]")
{
  index<unsigned, 8> src{std::allocator<unsigned>{}};
  for (unsigned k = 0; k < 128; k += 4)
    src.assure(k) = k + 1;
  for (unsigned iteration = 0; iteration < 100; ++iteration)
  {
    index<unsigned, 8> copy{src};
    for (unsigned k = 0; k < 128; k += 4)
    {
      check(copy, k, k + 1);
      REQUIRE(copy.get(k) != src.get(k));
      copy.assure(k) = iteration;
    }
    for (unsigned k = 0; k < 128; k += 4)
      check(src, k, k + 1);
  }
}
