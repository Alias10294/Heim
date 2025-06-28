#include "doctest.h"

#include "composition.hpp"

#include <string>

TEST_CASE("composition: map entity to component")
{
  heim::composition<std::string> c;
  heim::entity e = 0;
  c.emplace(e, "e");
  CHECK(c.get(e) == "e");
}

TEST_CASE("composition: maps entities to components correctly")
{
  heim::composition<std::string> c;
  heim::entity e1 = 0;
  heim::entity e2 = 1;
  c.emplace(e1, "e1");
  c.emplace(e2, "e2");
  CHECK(c.get(e1) == "e1");
  CHECK(c.get(e2) == "e2");
}

TEST_CASE("composition: erase components correctly")
{
  heim::composition<std::string> c;
  heim::entity e = 0;
  c.emplace(e, "e");
  c.erase(e);
  CHECK(c.contains(e) == false);
}

TEST_CASE("composition: emplace on same entity does nothing")
{
  heim::composition<std::string> c;
  heim::entity e = 0;
  c.emplace(e, "e1");
  c.emplace(e, "e2");
  CHECK(c.get(e) == "e1");
  CHECK(c.size() == 1);
}

TEST_CASE("composition: erase twice does nothing")
{
  heim::composition<std::string> c;
  heim::entity e = 7;
  c.emplace(e, "x");
  c.erase(e);
  c.erase(e);
  CHECK_FALSE(c.contains(e));
  CHECK(c.size() == 0);
}

TEST_CASE("composition: contains on unknown entity is false")
{
  heim::composition<int> c;
  CHECK_FALSE(c.contains(99999));
}

TEST_CASE("composition: reserve works as expected")
{
  heim::composition<std::string> c;
  c.reserve(10);
  CHECK_FALSE(c.contains(5));
  c.emplace(5, "ok");
  CHECK(c.get(5) == "ok");
}

TEST_CASE("composition: sort components maintains entity-component mapping")
{
  heim::composition<int> c;
  c.emplace(0, 5);
  c.emplace(1, 2);
  c.emplace(2, 7);

  c.sort([](const int& a, const int& b) { return a < b; });

  CHECK(c.get(c.composed(0)) == 2);
  CHECK(c.get(c.composed(1)) == 5);
  CHECK(c.get(c.composed(2)) == 7);

  CHECK(c.contains(0));
  CHECK(c.contains(1));
  CHECK(c.contains(2));
}

TEST_CASE("composition: index is updated after sort")
{
  heim::composition<int> c;
  c.emplace(100, 3);
  c.emplace(200, 1);
  c.sort([](const int& a, const int& b) { return a < b; });

  CHECK(c.index(200) == 0);
  CHECK(c.index(100) == 1);
}

TEST_CASE("composition: get is noexcept when entity is valid")
{
  heim::composition<int> c;
  c.emplace(1, 42);
  static_assert(noexcept(c.get(1)), "get() should be noexcept");
}

TEST_CASE("composition: works with unique_ptr")
{
  heim::composition<std::unique_ptr<int>> c;
  c.emplace(0, std::make_unique<int>(42));
  CHECK(*c.get(0) == 42);
}

TEST_CASE("composition: sparse vector expands correctly")
{
  heim::composition<std::string> c;
  heim::entity e = 1'000;
  c.emplace(e, "hello");
  CHECK(c.get(e) == "hello");
  CHECK(c.contains(e));
}

TEST_CASE("composition: erase and re-emplace reuses slot")
{
  heim::composition<int> c;
  heim::entity e = 0;
  c.emplace(e, 42);
  c.erase(e);
  c.emplace(e, 99);
  CHECK(c.get(e) == 99);
  CHECK(c.index(e) == c.size() - 1);
}

TEST_CASE("composition: sort empty composition")
{
  heim::composition<int> c;
  CHECK_NOTHROW(c.sort([](const int& a, const int& b) { return a < b; }));
}

TEST_CASE("composition: composed returns correct entity")
{
  heim::composition<std::string> c;
  c.emplace(10, "a");
  c.emplace(20, "b");

  auto val0 = c.get(c.composed(0));
  CHECK((val0 == "a" || val0 == "b"));
  auto val1 = c.get(c.composed(1));
  CHECK((val1 == "a" || val1 == "b"));

}

TEST_CASE("composition: reserve preserves data")
{
  heim::composition<std::string> c;
  c.emplace(2, "yo");
  c.reserve(1000);
  CHECK(c.get(2) == "yo");
  CHECK(c.contains(2));
}

TEST_CASE("composition_handle: basic use")
{
  auto handle = heim::make_handle<std::string>();
  heim::entity e = 0;
  handle.reserve(10);
  handle.erase(e); // doit être no-op
  CHECK_FALSE(handle.contains(e));
}

TEST_CASE("composition: handles many entities")
{
  heim::composition<int> c;
  for (heim::entity e = 0; e < 1'000'000; ++e)
    c.emplace(e, e);
  CHECK(c.size() == 1'000'000);
  CHECK(c.get(123456) == 123456);
}

TEST_CASE("composition: get on absent entity is undefined behavior")
{
  heim::composition<std::string> c;
  heim::entity e = 42;
  CHECK_FALSE(c.contains(e));
}

TEST_CASE("composition: emplace triggers sparse resize")
{
  heim::composition<std::string> c;
  c.emplace(99, "yo");
  CHECK(c.contains(99));
  CHECK(c.get(99) == "yo");
}

TEST_CASE("composition: sort with duplicate component values")
{
  heim::composition<int> c;
  c.emplace(1, 5);
  c.emplace(2, 5);
  c.emplace(3, 5);
  c.sort([](int a, int b) { return a < b; });

  CHECK(c.size() == 3);
  CHECK(c.contains(1));
  CHECK(c.contains(2));
  CHECK(c.contains(3));
}

struct MoveOnly {
  int value;
  MoveOnly(int v) : value(v) {}
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly& operator=(MoveOnly&&) = default;
};

TEST_CASE("composition: works with move-only types")
{
  heim::composition<MoveOnly> c;
  c.emplace(0, MoveOnly{42});
  CHECK(c.get(0).value == 42);
}

TEST_CASE("composition: handles sparse entity IDs")
{
  heim::composition<int> c;
  c.emplace(10, 10);
  c.emplace(1'000, 1'000);
  c.emplace(500'000, 500'000);
  CHECK(c.get(1'000) == 1'000);
  CHECK(c.get(10) == 10);
}

TEST_CASE("composition: default constructed is empty")
{
  heim::composition<int> c;
  CHECK(c.size() == 0);
}
