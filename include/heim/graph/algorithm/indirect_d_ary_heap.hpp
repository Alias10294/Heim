#ifndef HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP
#define HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/property/primitives.hpp"

namespace heim::graphs::detail
{
template<
    std::size_t Arity,
    typename    Cont,
    typename    PrioMap,
    typename    IdxMap,
    typename    Cmp>
requires (Arity >= 2)
class indirect_d_ary_heap
{
public:
  using size_type  = std::ranges::range_size_t <Cont>;
  using value_type = std::ranges::range_value_t<Cont>;

private:
  Cont                                             m_cnt;
  deduced_storage_t<value_type, size_type, IdxMap> m_i_rstorage;
  [[no_unique_address]] PrioMap                    m_pm;
  [[no_unique_address]] Cmp                        m_cmp;

private:
  constexpr
  bool
  precedes(value_type const &lhs, value_type const &rhs)
  {
    return std::invoke(
        this->m_cmp,
        properties::get(this->m_pm, lhs),
        properties::get(this->m_pm, rhs));
  }

  constexpr
  void
  assign_at(size_type pos, value_type &&val)
  {
    this->m_cnt[pos] = std::move(val);
    properties::set(this->m_i_rstorage.property_map(), this->m_cnt[pos], pos);
  }

  constexpr
  void
  heapify_up(size_type pos)
  {
    value_type val{std::move(this->m_cnt[pos])};

    while (pos != size_type{})
    {
      size_type parent{(pos - 1) / Arity};

      if (!this->precedes(val, this->m_cnt[parent]))
        break;

      this->assign_at(pos, std::move(this->m_cnt[parent]));
      pos = parent;
    }

    this->assign_at(pos, std::move(val));
  }

  constexpr
  void
  heapify_down(size_type pos)
  {
    size_type  size{std::ranges::size(this->m_cnt)};
    value_type val {std::move(this->m_cnt[pos])};

    while (!(size <= size_type{1} || pos > (size - 2) / Arity))
    {
      size_type first{Arity * pos + 1};
      size_type last {first + std::min<size_type>(Arity, size - first)};
      size_type best {first};

      for (size_type child{first + 1}; child < last; ++child)
      {
        if (this->precedes(this->m_cnt[child], this->m_cnt[best]))
          best = child;
      }

      if (!this->precedes(this->m_cnt[best], val))
        break;

      this->assign_at(pos, std::move(this->m_cnt[best]));
      pos = best;
    }

    this->assign_at(pos, std::move(val));
  }

public:
  constexpr
  indirect_d_ary_heap(
      std::integral_constant<std::size_t, Arity>,
      Cont  &&cont,
      PrioMap pm,
      IdxMap  im,
      Cmp   &&cmp)
    : m_cnt       {std::forward<Cont>(cont)}
    , m_i_rstorage{std::move(im)}
    , m_pm        {std::move(pm)}
    , m_cmp       {std::forward<Cmp>(cmp)}
  { }


  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return std::ranges::empty(this->m_cnt); }

  [[nodiscard]] constexpr
  value_type const &
  front() const
  {
    contract_assert(!this->empty());
    return this->m_cnt.front();
  }


  template<typename Vertex>
  constexpr
  void
  push(Vertex &&v)
  {
    size_type pos{std::ranges::size(this->m_cnt)};

    this->m_cnt.push_back(std::forward<Vertex>(v));
    this->heapify_up(std::move(pos));
  }

  constexpr
  void
  pop()
  {
    contract_assert(!this->empty());

    if (std::ranges::size(this->m_cnt) == size_type{1})
      this->m_cnt.pop_back();
    else
    {
      this->m_cnt.front() = std::move(this->m_cnt.back());
      this->m_cnt.pop_back();
      this->heapify_down(size_type{0});
    }
  }

  template<typename Vertex>
  constexpr
  void
  decrease_key(Vertex v)
  { this->heapify_up(properties::get(this->m_i_rstorage.property_map(), v)); }
};


template<
    std::size_t Arity,
    typename    Cont,
    typename    PrioMap,
    typename    IdxMap,
    typename    Cmp>
indirect_d_ary_heap(std::integral_constant<std::size_t, Arity>, Cont &&, PrioMap, IdxMap, Cmp &&)
-> indirect_d_ary_heap<Arity, Cont, PrioMap, IdxMap, Cmp>;

} // namespace heim::graphs::detail

#endif // HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP
