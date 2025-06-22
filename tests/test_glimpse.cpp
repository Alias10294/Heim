#include "doctest.h"

#include "heim/glimpse.hpp"

TEST_CASE("glimpse: single composition iteration")
{
  heim::composition<int> ints;
  ints.emplace(0, 10);
  ints.emplace(1, 20);

  heim::basic_glimpse<int> g(ints);
  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{0, 1});
}

TEST_CASE("glimpse: iterates only entities present in both compositions")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(0, 1);
  ints.emplace(1, 2);
  ints.emplace(2, 3);

  floats.emplace(1, 1.0f);
  floats.emplace(2, 2.0f);
  floats.emplace(3, 3.0f);

  heim::basic_glimpse<int, float> g(ints, floats);
  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{1, 2});
}

TEST_CASE("glimpse: no entity has all components")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(0, 1);
  floats.emplace(1, 1.0f);

  heim::basic_glimpse<int, float> g(ints, floats);
  CHECK(g.begin() == g.end());
}

TEST_CASE("glimpse: works with three components")
{
  heim::composition<int>    ints;
  heim::composition<float>  floats;
  heim::composition<double> doubles;

  ints.emplace(1, 1);
  floats.emplace(1, 1.0f);
  doubles.emplace(1, 1.0);

  ints.emplace(2, 2);
  floats.emplace(2, 2.0f);

  heim::basic_glimpse<int, float, double> g(ints, floats, doubles);
  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{1});
}

TEST_CASE("glimpse: handles sparse entity IDs")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(1000, 1);
  floats.emplace(1000, 1.0f);
  ints.emplace(500000, 2);
  floats.emplace(500000, 2.0f);

  heim::basic_glimpse<int, float> g(ints, floats);
  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{1000, 500000});
}

TEST_CASE("glimpse: empty composition gives empty iteration")
{
  heim::composition<int> ints;
  heim::basic_glimpse<int> g(ints);

  CHECK(g.begin() == g.end());
}

TEST_CASE("glimpse: skips erased entities correctly")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(0, 1);
  ints.emplace(1, 2);
  ints.emplace(2, 3);
  ints.erase(1);

  floats.emplace(0, 1.0f);
  floats.emplace(2, 2.0f);

  heim::basic_glimpse<int, float> g(ints, floats);
  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{0, 2});
}

TEST_CASE("glimpse: does not access out-of-bounds")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  for (int i = 0; i < 100; ++i)
    ints.emplace(i, i);

  heim::basic_glimpse<int, float> g(ints, floats);
  CHECK(g.begin() == g.end());
}

TEST_CASE("glimpse: preserves reference composition order")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(2, 1);
  ints.emplace(0, 1);
  ints.emplace(1, 1);

  floats.emplace(0, 1.0f);
  floats.emplace(1, 2.0f);
  floats.emplace(2, 3.0f);

  heim::basic_glimpse<int, float> g(ints, floats);

  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{2, 0, 1});
}

TEST_CASE("glimpse: filters out entities missing only one component")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(0, 1);
  ints.emplace(1, 2);
  floats.emplace(1, 2.0f);

  heim::basic_glimpse<int, float> g(ints, floats);

  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{1});
}

TEST_CASE("glimpse: does not reflect post-creation insertions")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(1, 10);
  floats.emplace(1, 1.0f);

  heim::basic_glimpse<int, float> g(ints, floats);

  ints.emplace(2, 20);
  floats.emplace(2, 2.0f);

  std::vector<heim::entity> result;
  for (heim::entity e : g)
    result.push_back(e);

  CHECK(result == std::vector<heim::entity>{1});
}

TEST_CASE("glimpse: empty on disjoint entity sets")
{
  heim::composition<int> ints;
  heim::composition<float> floats;

  ints.emplace(1, 1);
  floats.emplace(2, 2.0f);

  heim::basic_glimpse<int, float> g(ints, floats);
  CHECK(g.begin() == g.end());
}

