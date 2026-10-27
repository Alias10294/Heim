#include "support.hpp"
using namespace heim_test::sparse_map;

namespace {
using pmr_map = heim::sparse_map<unsigned,int,8,
  std::pmr::vector<unsigned>,std::pmr::vector<int>,
  std::pmr::polymorphic_allocator<std::pair<unsigned,int>>>;

class counting_resource final: public std::pmr::memory_resource
{
public:
  std::size_t allocate_calls{};
  std::size_t deallocate_calls{};
  std::size_t active_bytes{};
  bool reject{};
protected:
  void *do_allocate(std::size_t bytes,std::size_t alignment) override
  {
    if(reject) throw std::bad_alloc{};
    void *p=std::pmr::new_delete_resource()->allocate(bytes,alignment);
    ++allocate_calls;
    active_bytes+=bytes;
    return p;
  }
  void do_deallocate(void *p,std::size_t bytes,std::size_t alignment) override
  {
    ++deallocate_calls;
    active_bytes-=bytes;
    std::pmr::new_delete_resource()->deallocate(p,bytes,alignment);
  }
  bool do_is_equal(std::pmr::memory_resource const &other) const noexcept override
  { return this==std::addressof(other); }
};
}

TEST_CASE("sparse_map PMR uses supplied memory resource", "[common][sparse_map][pmr]")
{
  counting_resource res;
  {
    pmr_map m{pmr_map::allocator_type{&res}};
    for(unsigned i=0;i<80;++i) m.try_emplace(i*17u,int(i));
    REQUIRE(m.get_allocator().resource()==&res);
    REQUIRE(res.allocate_calls>0);
    REQUIRE(res.active_bytes>0);
  }
  REQUIRE(res.active_bytes==0);
  REQUIRE(res.allocate_calls==res.deallocate_calls);
}

TEST_CASE("sparse_map PMR copy with explicitly supplied destination resource", "[common][sparse_map][pmr]")
{
  counting_resource ra,rb;
  {
    pmr_map a{pmr_map::allocator_type{&ra}};
    a.try_emplace(2u,20); a.try_emplace(99u,990);
    pmr_map b(a,pmr_map::allocator_type{&rb});
    REQUIRE(b.get_allocator().resource()==&rb);
    verify(b,{{2u,20},{99u,990}});
    b.at(2u)=-1;
    REQUIRE(a.at(2u)==20);
  }
  REQUIRE(ra.active_bytes==0); REQUIRE(rb.active_bytes==0);
}

TEST_CASE("sparse_map PMR move construction with destination resource", "[common][sparse_map][pmr]")
{
  counting_resource ra,rb;
  {
    pmr_map a{pmr_map::allocator_type{&ra}};
    a.try_emplace(2u,20); a.try_emplace(99u,990);
    pmr_map b(std::move(a),pmr_map::allocator_type{&rb});
    REQUIRE(b.get_allocator().resource()==&rb);
    verify(b,{{2u,20},{99u,990}});
  }
  REQUIRE(ra.active_bytes==0); REQUIRE(rb.active_bytes==0);
}

TEST_CASE("sparse_map PMR failed new allocation preserves existing entries", "[common][sparse_map][pmr]")
{
  counting_resource res;
  {
    pmr_map m{pmr_map::allocator_type{&res}};
    m.try_emplace(1u,10);
    auto before=snapshot(m);
    res.reject=true;
    // New sparse page is required for this key.
    REQUIRE_THROWS_AS(m.try_emplace(100000u,500),std::bad_alloc);
    res.reject=false;
    verify(m,before);
    REQUIRE(m.try_emplace(100000u,500).second);
  }
  REQUIRE(res.active_bytes==0);
}

TEST_CASE("sparse_map PMR copy and swap with the same resource", "[common][sparse_map][pmr]")
{
  counting_resource res;
  {
    pmr_map a{pmr_map::allocator_type{&res}},b{pmr_map::allocator_type{&res}};
    a.try_emplace(1u,2); b.try_emplace(3u,4);
    a.swap(b);
    verify(a,{{3u,4}}); verify(b,{{1u,2}});
  }
  REQUIRE(res.active_bytes==0);
}
