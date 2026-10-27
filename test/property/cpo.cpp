#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>
#include <type_traits>

namespace {
struct member_map {
  int data[3]{};
  int& get(unsigned i) { return data[i]; }
  void set(unsigned i, int value) { data[i] = value; }
};
namespace adl {
struct map { int data[3]{}; };
int& get(map& m, unsigned i) { return m.data[i]; }
void set(map& m, unsigned i, int value) { m.data[i] = value; }
}
struct preferred_member {
  int value = 4;
  int get(unsigned) const { return value; }
  void set(unsigned, int v) { value = v; }
  friend int get(preferred_member&, unsigned) { return -1; }
  friend void set(preferred_member&, unsigned, int) {}
};
}

TEST_CASE("property CPO uses member customization", "[property][cpo]")
{
  member_map m;
  heim::properties::set(m, 2u, 19);
  REQUIRE(heim::properties::get(m, 2u) == 19);
}

TEST_CASE("property CPO uses ADL customization", "[property][cpo]")
{
  adl::map m;
  heim::properties::set(m, 1u, 77);
  REQUIRE(heim::properties::get(m, 1u) == 77);
}

TEST_CASE("member customization wins over ADL customization", "[property][cpo]")
{
  preferred_member m;
  REQUIRE(heim::properties::get(m, 0u) == 4);
  heim::properties::set(m, 0u, 8);
  REQUIRE(m.value == 8);
}
