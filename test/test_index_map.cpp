#include "doctest.h"
#include "index_map.hpp"

#include <iterator>
#include <string>
#include <type_traits>

TEST_CASE("heim::index_map: construction")
{
  heim::index_map<unsigned, std::string> map{};

  static_assert(std::is_copy_constructible_v<decltype(map)>);
  static_assert(std::is_copy_assignable_v   <decltype(map)>);
  static_assert(std::is_move_constructible_v<decltype(map)>);
  static_assert(std::is_move_assignable_v   <decltype(map)>);

  CHECK_EQ(map.empty(), true);
  CHECK_EQ(map.size() , 0);
  CHECK_EQ(map.begin(), map.end());

  auto [it, b] = map.emplace(0, "0");
  CHECK_EQ((*it).first , 0);
  CHECK_EQ((*it).second, "0");
  CHECK_EQ(b, true);

  CHECK_EQ(map.size() , 1);
  CHECK_EQ(map.empty(), false);
  CHECK_EQ(map.begin(), it);
  CHECK_EQ(std::ranges::distance(map.begin(), map.end()), 1);
  CHECK_EQ(map.contains(0), true);
  CHECK_EQ(map.find(0), it);
  CHECK_EQ(map[0]   , "0");
  CHECK_EQ(map.at(0), "0");

  bool r = map.erase(0);
  CHECK_EQ(r, true);

  CHECK_EQ(map.size(), 0);
  CHECK_EQ(map.empty(), true);
  CHECK_EQ(map.begin(), map.end());
  CHECK_EQ(std::ranges::distance(map.begin(), map.end()), 0);
  CHECK_EQ(map.contains(0), false);
  CHECK_EQ(map.find(0), map.end());

  map.emplace(0, "0");
  decltype(map) copied{map};
  CHECK_EQ(map   .size(), 1);
  CHECK_EQ(copied.size(), 1);

  decltype(map) moved{std::move(map)};
  CHECK_EQ(map  .size(), 0);
  CHECK_EQ(moved.size(), 1);
  CHECK_EQ(moved, copied);

  decltype(map) listed{{{0, "0"}}};
  CHECK_EQ(listed.size(), 1);
  CHECK_EQ(listed.contains(0), true);

  swap(moved, map);
  CHECK_EQ(map  .size(), 1);
  CHECK_EQ(moved.size(), 0);

  copied.clear();
  listed.clear();
  CHECK_EQ(copied.size(), 0);
  CHECK_EQ(listed.size(), 0);

  copied = map;
  CHECK_EQ(map   .size(), 1);
  CHECK_EQ(copied.size(), 1);

  moved = std::move(map);
  CHECK_EQ(map  .size(), 0);
  CHECK_EQ(moved.size(), 1);

  listed = {{0, "0"}};
  CHECK_EQ(listed.size(), 1);
  CHECK_EQ(listed.contains(0), true);

  swap(moved, map);

  CHECK_EQ(listed, map);
  CHECK_EQ(copied, map);


  [[maybe_unused]] heim::index_map<unsigned, std::unique_ptr<int>> umap{};

  static_assert(!std::is_copy_constructible_v<decltype(umap)>);
  static_assert(!std::is_copy_assignable_v   <decltype(umap)>);
  static_assert( std::is_move_constructible_v<decltype(umap)>);
  static_assert( std::is_move_assignable_v   <decltype(umap)>);
}

TEST_CASE("heim::index_map: insertion & deletion")
{
  heim::index_map<unsigned, std::string> map{};

  auto r1 = map.emplace(0, "0");
  CHECK_EQ(r1.second, true);
  CHECK_EQ(map[0], "0");

  auto r2 = map.emplace(0, "1");
  CHECK_EQ(r2.second, false);
  CHECK_EQ(map[0], "0");

  auto r3 = map.emplace_or_assign(0, "1");
  CHECK_EQ(r3.second, false);
  CHECK_EQ(map[0], "1");

  auto r4 = map.insert(1, std::string{"1"});
  CHECK_EQ(r4.second, true);
  CHECK_EQ(map[1], "1");

  std::string const str{"2"};
  auto r5 = map.insert(2, str);
  CHECK_EQ(r5.second, true);
  CHECK_EQ(map[2], "2");

  map.insert({{3, "3"}, {4, "4"}});
  CHECK_EQ(map[3], "3");
  CHECK_EQ(map[4], "4");

  decltype(map) other{};
  other.insert(map.begin(), map.end());
  CHECK_EQ(map  .contains(0), true);
  CHECK_EQ(other.contains(0), true);
  CHECK_EQ(map  .size(), 5);
  CHECK_EQ(other.size(), 5);

  map.erase(4);
  CHECK_EQ(map.size(), 4);
  CHECK_EQ(map.contains(4), false);

  map.erase(0);
  CHECK_EQ(map.size(), 3);
  CHECK_EQ(map.contains(0), false);
  CHECK_EQ((*map.begin()).first , 3);
  CHECK_EQ((*map.begin()).second, "3");

  map.erase(map.begin());
  CHECK_EQ(map.size(), 2);
  CHECK_EQ(map.contains(3), false);
  CHECK_EQ((*map.begin()).first , 2);
  CHECK_EQ((*map.begin()).second, "2");

  map.clear();
  CHECK_EQ(map.size(), 0);
  CHECK_EQ(map.contains(1), false);
  CHECK_EQ(map.contains(2), false);

  other.erase(other.begin() + 2, other.end());
  CHECK_EQ(other.size(), 2);
  CHECK_EQ(other.contains(0), true);
  CHECK_EQ(other.contains(1), true);
  CHECK_EQ(other.contains(2), false);
  CHECK_EQ(other.contains(3), false);
  CHECK_EQ(other.contains(4), false);

  other.erase(other.begin(), other.end());
  CHECK_EQ(other.size(), 0);
}

TEST_CASE("heim::index_map: iteration")
{
  heim::index_map<std::size_t, std::size_t> map{};

  CHECK_EQ(std::random_access_iterator<decltype(map)::iterator>, true);

  for (std::size_t i = 0; i < 10; ++i)
    map.insert(i, i);

  std::size_t cpt = 0;
  for (auto [idx, val] : map)
  {
    CHECK_EQ(idx, cpt);
    CHECK_EQ(val, cpt);
    ++cpt;
  }

  for (auto [idx, val] : std::ranges::reverse_view{map})
  {
    --cpt;
    CHECK_EQ(idx, cpt);
    CHECK_EQ(val, cpt);
  }

  iter_swap(map.begin(), map.end() - 1);

  auto first = map.begin();
  auto last  = map.end() - 1;
  CHECK_EQ((*first).first , 9);
  CHECK_EQ((*first).second, 9);
  CHECK_EQ((*last).first , 0);
  CHECK_EQ((*last).second, 0);
}