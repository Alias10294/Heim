#include "test.hpp"
#include "benchmark.hpp"

int main()
{
  heim::test     ::test();
  heim::benchmark::benchmark<256, 2, 10'000>();
}