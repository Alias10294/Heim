#include <heim/common/sparse.hpp>

#include "support.hpp"

namespace hs = heim_test::sparse_set;

namespace
{
template<class T, bool Copy, bool Move, bool Swap>
struct mode_allocator
{
  using value_type = T;
  using propagate_on_container_copy_assignment = std::bool_constant<Copy>;
  using propagate_on_container_move_assignment = std::bool_constant<Move>;
  using propagate_on_container_swap = std::bool_constant<Swap>;
  using is_always_equal = std::false_type;
  template<class U> struct rebind { using other = mode_allocator<U, Copy, Move, Swap>; };

  std::shared_ptr<hs::allocation_log> log;
  int owner{};

  mode_allocator() : log(std::make_shared<hs::allocation_log>()) {}
  mode_allocator(std::shared_ptr<hs::allocation_log> l, int id) : log(std::move(l)), owner(id) {}
  template<class U> mode_allocator(mode_allocator<U, Copy, Move, Swap> const &other) noexcept
    : log(other.log), owner(other.owner) {}

  T* allocate(std::size_t n) { return hs::tracked_allocator<T, false>{log, owner}.allocate(n); }
  void deallocate(T* p, std::size_t n) noexcept
  { hs::tracked_allocator<T, false>{log, owner}.deallocate(p, n); }

  mode_allocator select_on_container_copy_construction() const
  { return mode_allocator{log, owner + 100}; }

  template<class, bool, bool, bool> friend struct mode_allocator;
  template<class U> bool operator == (mode_allocator<U, Copy, Move, Swap> const &rhs) const noexcept
  { return owner == rhs.owner && log == rhs.log; }
};

template<bool C, bool M, bool S>
using alloc = mode_allocator<unsigned, C, M, S>;
template<bool C, bool M, bool S>
using set = heim::sparse_set<unsigned, 8, std::vector<unsigned, alloc<C, M, S>>, alloc<C, M, S>>;
template<bool C, bool M, bool S>
using pair_alloc = mode_allocator<std::pair<unsigned, int>, C, M, S>;
template<bool C, bool M, bool S>
using map = heim::sparse_map<unsigned, int, 8,
  std::vector<unsigned, alloc<C, M, S>>,
  std::vector<int, mode_allocator<int, C, M, S>>,
  pair_alloc<C, M, S>>;
}

TEST_CASE(
    "independent POCCA enables copy propagation even when POCS is false",
    "[common][sparse_set][sparse_map][allocator][traits]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    set<true, false, false> src{alloc<true, false, false>{log, 1}}, dst{alloc<true, false, false>{log, 2}};
    src.insert(1u);
    src.insert(17u);
    dst.insert(100u);
    dst = src;
    REQUIRE(dst.get_allocator().owner == 1);
    hs::verify(dst, {1u, 17u}, 101);

    map<true, false, false> ms{pair_alloc<true, false, false>{log, 3}}, md{pair_alloc<true, false,
        false>{log, 4}};
    ms.try_emplace(1u, 10);
    ms.try_emplace(17u, 170);
    md.try_emplace(100u, 100);
    md = ms;
    REQUIRE(md.get_allocator().owner == 3);
    REQUIRE(md.size() == 2);
    REQUIRE(md.at(17u) == 170);
    REQUIRE_FALSE(md.contains(100u));
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "independent POCMA enables move propagation without POCCA",
    "[common][sparse_set][sparse_map][allocator][traits]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    set<false, true, false> src{alloc<false, true, false>{log, 1}}, dst{alloc<false, true, false>{log, 2}};
    src.insert(1u);
    src.insert(17u);
    dst.insert(100u);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().owner == 1);
    hs::verify(dst, {1u, 17u}, 101);
    hs::verify(src, {}, 101, {1u, 17u});

    map<false, true, false> ms{pair_alloc<false, true, false>{log, 3}}, md{pair_alloc<false, true,
        false>{log, 4}};
    ms.try_emplace(1u, 10);
    ms.try_emplace(17u, 170);
    md.try_emplace(100u, 100);
    md = std::move(ms);
    REQUIRE(md.get_allocator().owner == 3);
    REQUIRE(md.size() == 2);
    REQUIRE(md.at(17u) == 170);
    REQUIRE_FALSE(ms.contains(1u));
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "independent POCS allows swapping unequal allocator owners",
    "[common][sparse_set][sparse_map][allocator][traits]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    set<false, false, true> left{alloc<false, false, true>{log, 1}}, right{alloc<false, false, true>{log,
        2}};
    left.insert(1u);
    right.insert(17u);
    left.swap(right);
    REQUIRE(left.get_allocator().owner == 2);
    REQUIRE(right.get_allocator().owner == 1);
    hs::verify(left, {17u}, 18);
    hs::verify(right, {1u}, 18);

    map<false, false, true> ml{pair_alloc<false, false, true>{log, 3}}, mr{pair_alloc<false, false,
        true>{log, 4}};
    ml.try_emplace(1u, 10);
    mr.try_emplace(17u, 170);
    ml.swap(mr);
    REQUIRE(ml.get_allocator().owner == 4);
    REQUIRE(mr.get_allocator().owner == 3);
    REQUIRE(ml.at(17u) == 170);
    REQUIRE(mr.at(1u) == 10);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "select_on_container_copy_construction is honored by all sparse storages",
    "[common][sparse_set][sparse_map][allocator][soccc]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    set<false, false, false> src{alloc<false, false, false>{log, 5}};
    src.insert(1u);
    src.insert(17u);
    set<false, false, false> copied{src};
    REQUIRE(copied.get_allocator().owner == 105);
    hs::verify(copied, {1u, 17u}, 18);

    map<false, false, false> ms{pair_alloc<false, false, false>{log, 7}};
    ms.try_emplace(1u, 10);
    ms.try_emplace(17u, 170);
    map<false, false, false> mc{ms};
    REQUIRE(mc.get_allocator().owner == 107);
    REQUIRE(mc.at(17u) == 170);
  }
  REQUIRE(log->balanced());
}
