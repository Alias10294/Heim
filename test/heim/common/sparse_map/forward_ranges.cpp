#include "support.hpp"
#include <forward_list>

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map constructs from a forward-only range", "[common][sparse_map][range]")
{
  std::forward_list<std::pair<unsigned, int>> entries{
      {3u, 30}, {17u, 170}, {3u, 300}, {4096u, 40}};
  map<> m{std::from_range, entries};
  verify(m, {{3u, 30}, {17u, 170}, {4096u, 40}}, 4097);
}

TEST_CASE("sparse_map inserts from a forward-only range", "[common][sparse_map][range]")
{
  std::forward_list<std::pair<unsigned, int>> entries{
      {5u, 50}, {9u, 90}, {5u, 500}};
  map<> m;
  m.insert_range(entries);
  verify(m, {{5u, 50}, {9u, 90}}, 10);
}
