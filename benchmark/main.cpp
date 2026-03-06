#include "creation.hpp"

int main()
{
  heim::benchmark::benchmark<4096, 2, 100'000>();
}