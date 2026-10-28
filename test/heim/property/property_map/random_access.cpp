#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>

#include <array>
#include <concepts>
#include <deque>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
namespace prop = heim::properties;

template<typename Range>
void verify_mutable_range(Range &range)
{
  auto tagged = heim::make_property_map(heim::random_access_range_arg, range);
  auto automatic = heim::make_property_map(range);
  static_assert(prop::map_for<decltype(tagged) &, int, int>);
  static_assert(prop::map_for<decltype(automatic) &, int, int>);
  REQUIRE(&prop::get(tagged, 1) == &range[1]);
  REQUIRE(&prop::get(automatic, 2) == &range[2]);
  prop::set(tagged, 1, 41);
  prop::set(automatic, 2, 42);
  REQUIRE(range[1] == 41);
  REQUIRE(range[2] == 42);
}

template<typename Range>
void verify_const_range(Range const &range)
{
  auto tagged = heim::make_property_map(heim::random_access_range_arg, range);
  auto automatic = heim::make_property_map(range);
  static_assert(prop::readable_map_for<decltype(tagged) &, int>);
  static_assert(prop::readable_map_for<decltype(automatic) &, int>);
  static_assert(!prop::writable_map_for<decltype(tagged) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(automatic) &, int, int>);
  REQUIRE(&prop::get(tagged, 1) == &range[1]);
  REQUIRE(&prop::get(automatic, 2) == &range[2]);
}
}

TEST_CASE("random-access range adapters support arrays", "[property][property_map][random_access]")
{
  std::array<int, 4> array{{0, 10, 20, 30}};
  verify_mutable_range(array);
  verify_const_range(array);
  int c_array[]{0, 10, 20, 30};
  verify_mutable_range(c_array);
  verify_const_range(c_array);
}

TEST_CASE("random-access range adapters support vectors and deques", "[property][property_map][random_access]")
{
  std::vector<int> vector{0, 10, 20, 30};
  std::deque<int> deque{0, 10, 20, 30};
  verify_mutable_range(vector);
  verify_mutable_range(deque);
  verify_const_range(vector);
  verify_const_range(deque);
}

TEST_CASE("span adapter reads and writes elements without owning them", "[property][property_map][random_access]")
{
  int values[]{2, 4, 6};
  auto dynamic = heim::make_property_map(std::span<int>{values});
  auto fixed = heim::make_property_map(std::span<int, 3>{values});
  prop::set(dynamic, 1, 44);
  REQUIRE(values[1] == 44);
  REQUIRE(&prop::get(fixed, 1) == &values[1]);
  auto read_only = heim::make_property_map(std::span<int const>{values});
  static_assert(!prop::writable_map_for<decltype(read_only) &, int, int>);
  REQUIRE(prop::get(read_only, 2) == 6);
}

TEST_CASE("random-access iterator adapter works with iterators and pointers", "[property][property_map][random_access]")
{
  std::vector<int> values{2, 4, 6, 8};
  auto tagged = heim::make_property_map(heim::random_access_iterator_arg, values.begin());
  auto automatic = heim::make_property_map(values.begin());
  REQUIRE(&prop::get(tagged, 0) == &values[0]);
  prop::set(tagged, 1, 40);
  prop::set(automatic, 2, 60);
  REQUIRE((values == std::vector<int>{2, 40, 60, 8}));

  int array[]{1, 3, 5};
  auto pointer = heim::make_property_map(heim::random_access_iterator_arg, array);
  prop::set(pointer, 2, 50);
  REQUIRE(prop::get(pointer, 2) == 50);
}

TEST_CASE("random-access adapters accept an external index map", "[property][property_map][random_access]")
{
  std::vector<int> values{10, 20, 30, 40, 50, 60};
  auto index = heim::make_property_map(heim::invocable_arg, [](int key) noexcept { return key * 2; });
  auto range_tagged = heim::make_property_map(heim::random_access_range_arg, values, index);
  auto range_auto = heim::make_property_map(values, index);
  auto iter_tagged = heim::make_property_map(heim::random_access_iterator_arg, values.begin(), index);
  auto iter_auto = heim::make_property_map(values.begin(), index);
  REQUIRE(prop::get(range_tagged, 2) == 50);
  REQUIRE(prop::get(range_auto, 1) == 30);
  REQUIRE(prop::get(iter_tagged, 1) == 30);
  prop::set(range_tagged, 1, 31);
  prop::set(range_auto, 2, 51);
  prop::set(iter_tagged, 0, 11);
  prop::set(iter_auto, 1, 32);
  REQUIRE((values == std::vector<int>{11, 20, 32, 40, 51, 60}));
}

TEST_CASE("span and array adapters support external index maps", "[property][property_map][random_access]")
{
  std::array<int, 4> values{1, 2, 3, 4};
  auto index = heim::make_property_map(heim::invocable_arg, [](int k) { return 3 - k; });
  auto array_map = heim::make_property_map(values, index);
  auto span_map = heim::make_property_map(std::span<int>{values}, index);
  int c_values[]{1, 2, 3, 4};
  auto c_map = heim::make_property_map(c_values, index);
  REQUIRE(prop::get(array_map, 0) == 4);
  REQUIRE(prop::get(span_map, 1) == 3);
  REQUIRE(prop::get(c_map, 2) == 2);
  prop::set(array_map, 3, 91);
  prop::set(span_map, 0, 94);
  prop::set(c_map, 2, 92);
  REQUIRE(values[0] == 91);
  REQUIRE(values[3] == 94);
  REQUIRE(c_values[1] == 92);
}

TEST_CASE("random-access adapter supports vector<bool> proxy references", "[property][property_map][random_access]")
{
  std::vector<bool> bits{false, true, false};
  auto map = heim::make_property_map(bits);
  REQUIRE_FALSE(static_cast<bool>(prop::get(map, 0)));
  REQUIRE(static_cast<bool>(prop::get(map, 1)));
  prop::set(map, 0, true);
  REQUIRE(bits[0]);
}

TEST_CASE("random-access iterator adapter supports const iterators", "[property][property_map][random_access]")
{
  std::vector<int> values{10, 20, 30};
  auto map = heim::make_property_map(heim::random_access_iterator_arg, values.cbegin());
  static_assert(prop::readable_map_for<decltype(map) &, int>);
  static_assert(!prop::writable_map_for<decltype(map) &, int, int>);
  REQUIRE(&prop::get(map, 1) == &values[1]);
  auto pointer = heim::make_property_map(values.data());
  prop::set(pointer, 2, 33);
  REQUIRE(values[2] == 33);
}

TEST_CASE("indexed random-access ranges preserve const access", "[property][property_map][random_access]")
{
  auto index = heim::make_property_map(heim::invocable_arg, [](int k) noexcept { return k + 1; });
  std::vector<int> const v{11, 22, 33};
  std::deque<int> const d{11, 22, 33};
  std::array<int, 3> const a{11, 22, 33};
  int const c[]{11, 22, 33};
  auto vm = heim::make_property_map(v, index);
  auto dm = heim::make_property_map(d, index);
  auto am = heim::make_property_map(a, index);
  auto cm = heim::make_property_map(c, index);
  auto sm = heim::make_property_map(std::span<int const>{v.data(), v.size()}, index);
  static_assert(!prop::writable_map_for<decltype(vm) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(dm) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(am) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(cm) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(sm) &, int, int>);
  REQUIRE(prop::get(vm, 1) == 33);
  REQUIRE(prop::get(dm, 1) == 33);
  REQUIRE(prop::get(am, 1) == 33);
  REQUIRE(prop::get(cm, 1) == 33);
  REQUIRE(prop::get(sm, 1) == 33);
}
