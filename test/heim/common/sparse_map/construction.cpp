#include "support.hpp"
using namespace heim_test::sparse_map;

TEST_CASE("sparse_map initializer_list first occurrence wins", "[common][sparse_map][construction]")
{
  map<> m{{4u,10},{1u,20},{4u,999},{9u,30},{1u,-99}};
  verify(m, {{4u,10},{1u,20},{9u,30}});
}

TEST_CASE("sparse_map iterator-pair construction deduplicates", "[common][sparse_map][construction]")
{
  std::vector<std::pair<unsigned,int>> rows{{0,7},{8,9},{0,20},{16,11}};
  map<> m(rows.begin(), rows.end());
  verify(m, {{0u,7},{8u,9},{16u,11}});
}

TEST_CASE("sparse_map from_range constructor deduplicates", "[common][sparse_map][construction]")
{
  std::vector<std::pair<unsigned,int>> rows{{11,12},{2,4},{11,22},{5,6}};
  map<> m(std::from_range, rows);
  verify(m, {{11u,12},{2u,4},{5u,6}});
}

TEST_CASE("sparse_map container constructor preserves first values", "[common][sparse_map][construction]")
{
  map<> m(std::vector<unsigned>{7,5,7,8}, std::vector<int>{70,50,700,80});
  verify(m, {{7u,70},{5u,50},{8u,80}});
}

TEST_CASE("sparse_map unique tag accepts prevalidated disjoint keys", "[common][sparse_map][construction]")
{
  map<> m(heim::unique, std::vector<unsigned>{2,5,12}, std::vector<int>{20,50,120});
  verify(m, {{2u,20},{5u,50},{12u,120}});
}

TEST_CASE("sparse_map unique initializer and iterator construction", "[common][sparse_map][construction]")
{
  map<> m(heim::unique, {{2u,9},{1u,7},{13u,20}});
  verify(m, {{2u,9},{1u,7},{13u,20}});
  std::vector<std::pair<unsigned,int>> src{{6,4},{13,1},{3,10}};
  map<> n(heim::unique, src.begin(), src.end());
  verify(n, {{6u,4},{13u,1},{3u,10}});
}

TEST_CASE("sparse_map empty construction variants", "[common][sparse_map][construction]")
{
  std::vector<std::pair<unsigned,int>> rows;
  map<> a(rows.begin(), rows.end());
  map<> b(std::from_range, rows);
  map<> c(std::vector<unsigned>{}, std::vector<int>{});
  map<> d(heim::unique, std::vector<unsigned>{}, std::vector<int>{});
  verify(a, {}); verify(b, {}); verify(c, {}); verify(d, {});
}

TEST_CASE("sparse_map container constructor with explicit allocator", "[common][sparse_map][construction]")
{
  using M=map<>;
  M a(std::vector<unsigned>{3,3,4}, std::vector<int>{10,20,30}, M::allocator_type{});
  verify(a, {{3u,10},{4u,30}});
  M b(heim::unique, std::vector<unsigned>{3,4}, std::vector<int>{10,30}, M::allocator_type{});
  verify(b, {{3u,10},{4u,30}});
}

TEST_CASE("sparse_map iterator, range and initializer constructors with allocator", "[common][sparse_map][construction]")
{
  using M=map<>;
  std::vector<M::value_type> rows{{1,12},{6,19},{1,100}};
  M a(rows.begin(),rows.end(),M::allocator_type{});
  M b(std::from_range,rows,M::allocator_type{});
  M c({{1,12},{6,19},{1,100}}, M::allocator_type{});
  verify(a,{{1u,12},{6u,19}}); verify(b,{{1u,12},{6u,19}}); verify(c,{{1u,12},{6u,19}});
}

TEST_CASE("sparse_map unique constructors with explicit allocator", "[common][sparse_map][construction]")
{
  using M=map<>;
  std::vector<M::value_type> rows{{3,9},{2,8}};
  M a(heim::unique, rows.begin(), rows.end(), M::allocator_type{});
  M b(heim::unique, {{3,9},{2,8}}, M::allocator_type{});
  verify(a,{{3u,9},{2u,8}}); verify(b,{{3u,9},{2u,8}});
}
