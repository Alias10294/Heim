#include "doctest.h"
#include "index_map.hpp"

#include <string>
#include <iterator>

TEST_CASE("heim::index_map")
{
  heim::index_map<unsigned, std::string> map{};
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
}