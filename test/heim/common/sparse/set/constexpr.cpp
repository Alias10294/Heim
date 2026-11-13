#include "support.hpp"

namespace hs = heim_test::sparse_set;

constexpr bool sparse_set_empty_is_constexpr()
{
  hs::set<unsigned, 0> s;
  return s.empty() && s.size() == 0 && !s.contains(0u);
}

constexpr bool sparse_set_contiguous_operations_are_constexpr()
{
  hs::set<unsigned, 0> s;
  if (!s.insert(1u).second || !s.insert(3u).second)
    return false;
  if (s.insert(3u).second || !s.contains(1u) || !s.contains(3u))
    return false;
  if (!s.try_erase(1u) || s.contains(1u))
    return false;
  s.clear();
  return s.empty();
}

constexpr bool sparse_set_paged_operations_are_constexpr()
{
  hs::set<unsigned, 8> s;
  s.insert(1u);
  s.insert(9u);
  if (!s.contains(9u) || s.size() != 2)
    return false;
  s.try_erase(1u);
  return !s.contains(1u) && s.contains(9u);
}

static_assert(sparse_set_empty_is_constexpr());

// GCC PR125459: GCC 16 can reject a constexpr contract assertion when its
// predicate calls a function already invoked during the same evaluation.
// Fixed on the GCC 17 development branch (2026-09-22). Keep exercising both
// functions at runtime below while excluding only the affected static_asserts.
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 17)
static_assert(sparse_set_contiguous_operations_are_constexpr());
static_assert(sparse_set_paged_operations_are_constexpr());
#endif

TEST_CASE("sparse_set empty state is constexpr-capable", "[common][sparse_set][constexpr]")
{
  STATIC_REQUIRE(sparse_set_empty_is_constexpr());
}

TEST_CASE("sparse_set contiguous insertion and erasure work", "[common][sparse_set][constexpr]")
{
  REQUIRE(sparse_set_contiguous_operations_are_constexpr());
}

TEST_CASE("sparse_set paged insertion and erasure work", "[common][sparse_set][constexpr]")
{
  REQUIRE(sparse_set_paged_operations_are_constexpr());
}
