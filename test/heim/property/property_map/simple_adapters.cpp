#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
namespace prop = heim::properties;
using identity_map = decltype(heim::make_property_map(heim::identity_arg));
using pointer_map = decltype(heim::make_property_map(heim::pointer_arg));
static_assert(
    std::same_as<decltype(prop::get(std::declval<identity_map &>(), std::declval<int &>())), int &>);
static_assert(std::same_as<decltype(prop::get(std::declval<identity_map &>(), std::declval<int const &>())),
                           int const &>);
static_assert(
    std::same_as<decltype(prop::get(std::declval<identity_map &>(), std::declval<int &&>())), int &&>);
static_assert(!prop::writable_map_for<identity_map &, int &, int>);
static_assert(noexcept(prop::get(std::declval<identity_map &>(), std::declval<int &>())));
static_assert(prop::map_for<pointer_map &, int *, int>);
static_assert(!prop::writable_map_for<pointer_map &, int const *, int>);
}

TEST_CASE("identity_arg forwards the key without copying", "[property][property_map][identity]")
{
  auto map = heim::make_property_map(heim::identity_arg);
  int value{17};
  REQUIRE(&prop::get(map, value) == &value);
  prop::get(map, value) = 33;
  REQUIRE(value == 33);
  REQUIRE(prop::get(map, 41) == 41);
  int const constant{9};
  REQUIRE(&prop::get(map, constant) == &constant);
}

TEST_CASE("constant_arg ignores the key and retains its captured value", "[property][property_map][constant]")
{
  std::string source{"first"};
  auto map = heim::make_property_map(heim::constant_arg, source);
  source = "second";
  REQUIRE(prop::get(map, 0) == "first");
  REQUIRE(prop::get(map, "unrelated key") == "first");
  REQUIRE(&prop::get(map, 0) == &prop::get(map, 100));
  static_assert(!prop::writable_map_for<decltype(map) &, int, std::string>);
}

TEST_CASE("constant_arg can capture move-only values", "[property][property_map][constant]")
{
  auto map = heim::make_property_map(heim::constant_arg, std::make_unique<int>(23));
  REQUIRE(*prop::get(map, 1) == 23);
  REQUIRE(*prop::get(map, 999) == 23);
  static_assert(!std::copy_constructible<decltype(map)>);
}

TEST_CASE("pointer_arg handles raw pointers and const pointers", "[property][property_map][pointer]")
{
  auto map = heim::make_property_map(heim::pointer_arg);
  int value{4};
  int const constant{13};
  REQUIRE(&prop::get(map, &value) == &value);
  REQUIRE(&prop::get(map, &constant) == &constant);
  prop::set(map, &value, 18);
  REQUIRE(value == 18);
}

TEST_CASE("pointer_arg supports pointer-like key objects", "[property][property_map][pointer]")
{
  auto map = heim::make_property_map(heim::pointer_arg);
  auto ptr = std::make_unique<std::string>("old");
  REQUIRE(&prop::get(map, ptr) == ptr.get());
  prop::set(map, ptr, std::string{"new"});
  REQUIRE(*ptr == "new");
}
