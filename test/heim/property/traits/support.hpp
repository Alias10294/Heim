#ifndef HEIM_TEST_PROPERTY_TRAITS_SUPPORT_HPP
#define HEIM_TEST_PROPERTY_TRAITS_SUPPORT_HPP

#include <type_traits>

#include <heim/property.hpp>

namespace heim_test::property_traits
{
struct read_write
{
  int value{};
  constexpr int &get(int) noexcept { return value; }
  constexpr void set(int, int v) noexcept { value = v; }
};

struct read_only
{
  int value{};
  constexpr int const &get(int) const noexcept { return value; }
};

struct write_only
{
  int value{};
  constexpr void set(int, int v) noexcept { value = v; }
};

struct read_reference_write_reference
{
  int value{};
  constexpr int &get(int) noexcept { return value; }
  constexpr void set(int, int &v) noexcept { value = v; }
};

struct read_value_write_reference
{
  int get(int) const noexcept { return 7; }
  void set(int, int &) noexcept {}
};

struct key_sensitive
{
  constexpr int get(int) const noexcept { return 1; }
  constexpr void set(int, int) noexcept {}
};

struct variadic_writer
{
  constexpr void set(int, int, int) noexcept {}
};

struct empty
{
};

namespace adl
{
struct map
{
  int value{10};
  friend constexpr int &get(map &m, int) noexcept { return m.value; }
  friend constexpr void set(map &m, int, int v) noexcept { m.value = v; }
};
}
}

#endif // HEIM_TEST_PROPERTY_TRAITS_SUPPORT_HPP
