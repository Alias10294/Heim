#include "support.hpp"
#include <memory_resource>

namespace hs = heim_test::sparse_set;

namespace {
using pmr_set = heim::sparse_set<
    unsigned, 8,
    std::pmr::vector<unsigned>,
    std::pmr::polymorphic_allocator<unsigned>>;

struct counting_resource : std::pmr::memory_resource
{
  std::pmr::memory_resource *upstream = std::pmr::new_delete_resource();
  std::size_t allocated{};
  std::size_t deallocated{};
  std::size_t live_bytes{};

  void *do_allocate(std::size_t bytes, std::size_t alignment) override
  {
    void *p = upstream->allocate(bytes, alignment);
    ++allocated;
    live_bytes += bytes;
    return p;
  }
  void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) override
  {
    upstream->deallocate(p, bytes, alignment);
    ++deallocated;
    live_bytes -= bytes;
  }
  bool do_is_equal(std::pmr::memory_resource const &r) const noexcept override
  { return this == &r; }
};
}

TEST_CASE("sparse_set supports pmr polymorphic_allocator", "[common][sparse_set][pmr]")
{
  counting_resource resource;
  {
    pmr_set s{std::pmr::polymorphic_allocator<unsigned>{&resource}};
    s.insert(1u);
    s.insert(9u);
    s.insert(129u);
    hs::verify(s, {1u, 9u, 129u});
    REQUIRE(resource.allocated > 0);
    REQUIRE(s.get_allocator().resource() == &resource);
  }
  REQUIRE(resource.allocated == resource.deallocated);
  REQUIRE(resource.live_bytes == 0);
}

TEST_CASE("sparse_set clears pmr pages and remains reusable", "[common][sparse_set][pmr]")
{
  counting_resource resource;
  {
    pmr_set s{std::pmr::polymorphic_allocator<unsigned>{&resource}};
    for (auto k : {1u, 9u, 17u, 25u})
      s.insert(k);
    s.clear();
    REQUIRE(s.empty());
    REQUIRE(s.insert(17u).second);
    hs::verify(s, {17u});
  }
  REQUIRE(resource.live_bytes == 0);
}

TEST_CASE(
    "sparse_set pmr copy with explicit resource owns independent storage",
    "[common][sparse_set][pmr]")
{
  counting_resource resource_a, resource_b;
  {
    pmr_set a{std::pmr::polymorphic_allocator<unsigned>{&resource_a}};
    a.insert(1u);
    a.insert(9u);
    pmr_set b{a, std::pmr::polymorphic_allocator<unsigned>{&resource_b}};
    REQUIRE(a.get_allocator().resource() == &resource_a);
    REQUIRE(b.get_allocator().resource() == &resource_b);
    REQUIRE(b.try_erase(1u));
    hs::verify(a, {1u, 9u});
    hs::verify(b, {9u});
  }
  REQUIRE(resource_a.live_bytes == 0);
  REQUIRE(resource_b.live_bytes == 0);
}

TEST_CASE("sparse_set pmr replacement uses the destination resource", "[common][sparse_set][pmr]")
{
  counting_resource resource;
  {
    auto alloc = std::pmr::polymorphic_allocator<unsigned>{&resource};
    pmr_set s{alloc};
    s.insert(1u);
    std::pmr::vector<unsigned> values{{9u, 9u, 18u}, alloc};
    s.replace(std::move(values));
    hs::verify(s, {9u, 18u});
    REQUIRE(s.get_allocator().resource() == &resource);
  }
  REQUIRE(resource.live_bytes == 0);
}
