#include "support.hpp"
using namespace heim_test::sparse_map;

namespace {
constexpr bool empty_map_is_empty()
{
  map<> m;
  return m.empty() && m.size()==0 && !m.contains(1u);
}

constexpr bool mutate_map()
{
  map<> m;
  if(!m.try_emplace(3u,30).second) return false;
  if(!m.try_emplace(17u,170).second) return false;
  if(m.at(17u)!=170) return false;
  if(m.try_emplace(3u,999).second) return false;
  if(!m.try_erase(3u)) return false;
  if(m.contains(3u)) return false;
  if(m.at(17u)!=170) return false;
  m.clear();
  return m.empty();
}

constexpr bool assignments_in_constexpr()
{
  map<> m;
  if(!m.insert_or_assign(1u,5).second) return false;
  if(m.insert_or_assign(1u,6).second) return false;
  return m.at(1u)==6;
}
}

static_assert(empty_map_is_empty());
// GCC 16 with Contracts can reject a valid constexpr predicate involving
// contract_assert (GCC PR125459). Keep runtime coverage under GCC 16.
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 17
static_assert(mutate_map());
static_assert(assignments_in_constexpr());
#endif

TEST_CASE("sparse_map empty constexpr construction", "[common][sparse_map][constexpr]")
{ REQUIRE(empty_map_is_empty()); }

TEST_CASE("sparse_map constexpr insertion erasure and clear at runtime", "[common][sparse_map][constexpr]")
{ REQUIRE(mutate_map()); }

TEST_CASE("sparse_map constexpr insertion and assignment at runtime", "[common][sparse_map][constexpr]")
{ REQUIRE(assignments_in_constexpr()); }
