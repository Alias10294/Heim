#include "doctest.h"
#include "entity_manager.hpp"

TEST_CASE("heim::entity_manager")
{
  heim::entity_manager<std::size_t, std::allocator<std::size_t>> em;
  CHECK_EQ(em.size(), 0);

  auto e0 = em.generate();
  CHECK_EQ(em.size(), 1);
  CHECK_EQ(em.valid(e0), true);
  CHECK_EQ(heim::entity_traits<decltype(e0)>::index(e0), 0);

  auto e1 = em.generate();
  CHECK_EQ(em.size(), 2);
  CHECK_EQ(em.valid(e1), true);
  CHECK_EQ(heim::entity_traits<decltype(e1)>::index(e1), 1);

  auto e2 = em.generate();
  CHECK_EQ(em.size(), 3);
  CHECK_EQ(em.valid(e2), true);
  CHECK_EQ(heim::entity_traits<decltype(e2)>::index(e2), 2);

  em.invalidate(e0);
  CHECK_EQ(em.size(), 3);
  CHECK_EQ(em.valid(e0), false);

  em.invalidate(e2);
  CHECK_EQ(em.size(), 3);
  CHECK_EQ(em.valid(e2), false);

  auto e3 = em.generate();
  CHECK_EQ(em.size(), 3);
  CHECK_EQ(em.valid(e3), true);
  CHECK_EQ(
      heim::entity_traits<decltype(e3)>::index(e3),
      heim::entity_traits<decltype(e2)>::index(e2));
  CHECK_EQ(
      heim::entity_traits<decltype(e3)>::generation(e3),
      heim::entity_traits<decltype(e2)>::generation(e2) + 1);
  CHECK_EQ(em.valid(e2), false);

  auto e4 = em.generate();
  CHECK_EQ(em.size(), 3);
  CHECK_EQ(em.valid(e4), true);
  CHECK_EQ(
      heim::entity_traits<decltype(e4)>::index(e4),
      heim::entity_traits<decltype(e0)>::index(e0));
  CHECK_EQ(
      heim::entity_traits<decltype(e4)>::generation(e4),
      heim::entity_traits<decltype(e0)>::generation(e0) + 1);
  CHECK_EQ(em.valid(e0), false);

  auto e5 = em.generate();
  CHECK_EQ(em.size(), 4);
  CHECK_EQ(em.valid(e5), true);
  CHECK_EQ(heim::entity_traits<decltype(e5)>::index(e5), 3);
}