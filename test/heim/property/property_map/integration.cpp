#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
namespace prop = heim::properties;
struct entity
{
  int hp;
};

namespace external
{
struct custom_map
{
  std::vector<int> *values;
  friend int &get(custom_map &m, int k) noexcept { return (*m.values)[static_cast<std::size_t>(k)]; }
  friend void set(custom_map &m, int k, int v) noexcept { (*m.values)[static_cast<std::size_t>(k)] = v; }
};
}

static_assert(prop::map_for<external::custom_map &, int, int>);
static_assert(prop::reflective_map_for<external::custom_map &, int>);
}

TEST_CASE("documentation-style vector and member composition", "[property][property_map][integration]")
{
  std::unordered_map<int, int> map{{0, 100}, {1, 80}, {2, 60}};
  std::vector<entity> entities{{100}, {80}, {60}};
  auto map_pm = heim::make_property_map(map);
  auto vec_pm = heim::make_property_map(heim::composable_arg,
                                        heim::make_property_map(heim::member_arg, &entity::hp),
                                        heim::make_property_map(entities));
  REQUIRE(prop::get(map_pm, 1) == 80);
  REQUIRE(prop::get(vec_pm, 1) == 80);
  prop::set(map_pm, 1, 20);
  prop::set(vec_pm, 1, 20);
  REQUIRE(map.at(1) == 20);
  REQUIRE(entities[1].hp == 20);
}

TEST_CASE(
    "external custom map participates in generic property algorithms",
    "[property][property_map][integration]")
{
  std::vector<int> values{3, 6, 9};
  external::custom_map map{&values};
  auto increment = []<typename M>(M &m, int key)
    requires prop::map_for<M &, int, int> { prop::set(m, key, prop::get(m, key) + 1); };
  increment(map, 1);
  auto adapted = heim::make_property_map(values);
  increment(adapted, 2);
  REQUIRE((values == std::vector<int>{3, 7, 10}));
}

TEST_CASE(
    "adapters continue referring to their original resources after copying",
    "[property][property_map][integration]")
{
  std::map<int, int> values{{1, 2}};
  auto original = heim::make_property_map(values);
  auto copied = original;
  prop::set(copied, 1, 100);
  REQUIRE(prop::get(original, 1) == 100);
  values.insert_or_assign(2, 200);
  REQUIRE(prop::get(copied, 2) == 200);
}

TEST_CASE(
    "read-only and write-only access supports external generic algorithms",
    "[property][property_map][integration]")
{
  std::vector<int> values{10, 20};
  auto read = heim::make_readable_property_map(values);
  auto write = heim::make_writable_property_map(values);
  auto copy_value = []<typename R, typename W>(R &src, W &dst)
    requires prop::readable_map_for<R &, int> && prop::writable_map_for<W &, int, int>
  { prop::set(dst, 1, prop::get(src, 0)); };
  copy_value(read, write);
  REQUIRE((values == std::vector<int>{10, 10}));
}
