#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set uses the stateful allocator provided to its constructor", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    auto alloc = hs::tracked_allocator<unsigned>{log, 13};
    hs::tracked_set<8> s{alloc};
    REQUIRE(s.get_allocator() == alloc);
    s.insert(1u); s.insert(17u); s.insert(33u);
    hs::verify(s, {1u, 17u, 33u});
    REQUIRE(log->allocations > 0);
    REQUIRE(log->page_allocations == 3);
    REQUIRE(log->live_pages() == 3);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set repeated insertions in one page do not allocate another page", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 6}};
    for (unsigned k = 0; k < 8; ++k) s.insert(k);
    REQUIRE(log->page_allocations == 1);
    s.insert(8u);
    REQUIRE(log->page_allocations == 2);
    hs::verify(s, {0u,1u,2u,3u,4u,5u,6u,7u,8u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set keeps sparse pages after key erasure until clear", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 6}};
    s.insert(1u); s.insert(9u); s.insert(17u);
    REQUIRE(log->live_pages() == 3);
    s.try_erase(1u); s.try_erase(9u); s.try_erase(17u);
    REQUIRE(log->live_pages() == 3);
    s.clear();
    REQUIRE(log->live_pages() == 0);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set contiguous index does not allocate pages", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<0> s{hs::tracked_allocator<unsigned>{log, 2}};
    for (unsigned k : {0u, 8u, 19u, 100u}) s.insert(k);
    REQUIRE(log->page_allocations == 0);
    REQUIRE(log->live_pages() == 0);
    hs::verify(s, {0u, 8u, 19u, 100u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set copy construction deep-copies every sparse page", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> original{hs::tracked_allocator<unsigned>{log, 5}};
    for (unsigned k : {1u, 9u, 33u}) original.insert(k);
    REQUIRE(log->live_pages() == 3);
    {
      hs::tracked_set<8> copy{original};
      REQUIRE(log->live_pages() == 6);
      copy.try_erase(9u);
      hs::verify(original, {1u, 9u, 33u});
      hs::verify(copy, {1u, 33u});
    }
    REQUIRE(log->live_pages() == 3);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set copy construction with allocator uses specified owner", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    hs::tracked_set<8> src{alloc{log, 1}};
    src.insert(1u); src.insert(30u);
    hs::tracked_set<8> dst{src, alloc{log, 2}};
    REQUIRE(dst.get_allocator().owner == 2);
    hs::verify(dst, {1u, 30u});
    REQUIRE(dst.try_erase(30u));
    REQUIRE(src.contains(30u));
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set move construction with a different allocator retains ownership", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    hs::tracked_set<8> src{alloc{log, 1}};
    src.insert(1u); src.insert(9u);
    hs::tracked_set<8> dst{std::move(src), alloc{log, 2}};
    REQUIRE(dst.get_allocator().owner == 2);
    hs::verify(dst, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set copy assignment without allocator propagation retains owner", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, false>;
    hs::tracked_set<8, false> src{alloc{log, 1}}, dst{alloc{log, 2}};
    src.insert(1u); src.insert(9u);
    dst.insert(42u);
    dst = src;
    REQUIRE(dst.get_allocator().owner == 2);
    REQUIRE(src.get_allocator().owner == 1);
    hs::verify(dst, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set copy assignment with allocator propagation changes owner", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, true>;
    hs::tracked_set<8, true> src{alloc{log, 1}}, dst{alloc{log, 2}};
    src.insert(1u); src.insert(9u);
    dst.insert(42u);
    dst = src;
    REQUIRE(dst.get_allocator().owner == 1);
    hs::verify(dst, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set move assignment with allocator propagation changes owner", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, true>;
    hs::tracked_set<8, true> src{alloc{log, 1}}, dst{alloc{log, 2}};
    src.insert(1u); src.insert(9u);
    dst.insert(42u);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().owner == 1);
    hs::verify(dst, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set move assignment without propagation retains destination owner", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, false>;
    hs::tracked_set<8, false> src{alloc{log, 1}}, dst{alloc{log, 2}};
    src.insert(1u); src.insert(9u);
    dst.insert(42u);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().owner == 2);
    hs::verify(dst, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set swap with same owner exchanges membership", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, false>;
    hs::tracked_set<8> a{alloc{log, 1}}, b{alloc{log, 1}};
    a.insert(1u); a.insert(9u); b.insert(100u);
    a.swap(b);
    hs::verify(a, {100u});
    hs::verify(b, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set swap with propagation exchanges allocator owners", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned, true>;
    hs::tracked_set<8, true> a{alloc{log, 1}}, b{alloc{log, 2}};
    a.insert(1u); a.insert(9u); b.insert(100u);
    a.swap(b);
    REQUIRE(a.get_allocator().owner == 2);
    REQUIRE(b.get_allocator().owner == 1);
    hs::verify(a, {100u});
    hs::verify(b, {1u, 9u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set replacement retains its allocator", "[common][sparse_set][allocator]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    hs::tracked_set<8> s{alloc{log, 33}};
    s.insert(1u);
    std::vector<unsigned, alloc> values{{9u, 9u, 18u}, alloc{log, 33}};
    s.replace(std::move(values));
    REQUIRE(s.get_allocator().owner == 33);
    hs::verify(s, {9u, 18u});
  }
  REQUIRE(log->balanced());
}
