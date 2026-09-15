#ifndef HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP
#define HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>
#include "heim/property/primitives.hpp"

namespace heim
{
template<
    std::size_t Arity,
    typename    Container,
    typename    PriorityMap,
    typename    IndexMap,
    typename    Compare>
requires (Arity >= 2)
class indirect_d_ary_heap
{
private:
  using size_type
  = std::ranges::range_size_t<Container>;

private:
  [[no_unique_address]] Container &&m_cont;
  [[no_unique_address]] PriorityMap m_p_map;
  [[no_unique_address]] IndexMap    m_i_map;
  [[no_unique_address]] Compare     m_cmp;

private:
  constexpr
  void
  heapify_up(std::ranges::range_size_t<Container> pos)
  {
    while (pos != size_type{})
    {
      size_type parent{(pos - 1) / Arity};

      if (!std::invoke(
          this->m_cmp,
          properties::get(this->m_p_map, this->m_cont[pos]),
          properties::get(this->m_p_map, this->m_cont[parent])))
        break;

      using std::swap;

      swap(this->m_cont[pos], this->m_cont[parent]);
      properties::set(this->m_i_map, this->m_cont[pos]   , pos);
      properties::set(this->m_i_map, this->m_cont[parent], parent);

      pos = std::move(parent);
    }
  }

  constexpr
  void
  heapify_down(std::ranges::range_size_t<Container> pos)
  {
    size_type size{std::ranges::size(this->m_cont)};

    for (;;)
    {
      size_type begin{Arity * pos + 1};

      if (begin >= size)
        return;

      size_type best{begin};
      size_type end {begin + std::min(Arity, size - best)};

      for (size_type child{begin + 1}; child < end; ++child)
      {
        if (std::invoke(
            this->m_cmp,
            properties::get(this->m_p_map, this->m_cont[child]),
            properties::get(this->m_p_map, this->m_cont[best])))
          best = child;
      }

      if (!std::invoke(
          this->m_cmp,
          properties::get(this->m_p_map, this->m_cont[best]),
          properties::get(this->m_p_map, this->m_cont[pos])))
        return;

      using std::swap;

      swap(this->m_cont[best], this->m_cont[pos]);
      properties::set(this->m_i_map, this->m_cont[best], best);
      properties::set(this->m_i_map, this->m_cont[pos] , pos);

      pos = std::move(best);
    }
  }

public:
  constexpr
  indirect_d_ary_heap()
  = default;

  constexpr
  indirect_d_ary_heap(
      std::integral_constant<std::size_t, Arity>,
      Container &&cont,
      PriorityMap p_map,
      IndexMap    i_map,
      Compare     cmp)
    : m_cont {std::forward<Container>(cont)}
    , m_p_map{std::move(p_map)}
    , m_i_map{std::move(i_map)}
    , m_cmp  {std::move(cmp)}
  { }


  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return std::ranges::empty(this->m_cont); }


  template<typename Vertex>
  constexpr
  void
  insert(Vertex v)
  {
    size_type pos{std::ranges::size(this->m_cont)};

    this->m_cont.push_back(std::move(v));
    properties::set(this->m_i_map, this->m_cont.back(), pos);

    heapify_up(std::move(pos));
  }

  constexpr
  auto
  extract_min()
  {
    contract_assert(!std::ranges::empty(this->m_cont));

    size_type size{std::ranges::size(this->m_cont)};
    auto      res {std::move(this->m_cont.front())};

    if (size == size_type{1})
      this->m_cont.pop_back();
    else
    {
      this->m_cont.front() = std::move(this->m_cont.back());
      this->m_cont.pop_back();
      properties::set(this->m_i_map, this->m_cont.front(), size_type{0});

      heapify_down(size_type{0});
    }

    return res;
  }

  template<typename Vertex>
  constexpr
  void
  decrease_key(Vertex v)
  { heapify_up(properties::get(this->m_i_map, v)); }
};

} // namespace heim

#endif // HEIM_GRAPH_ALGORITHM_INDIRECT_D_ARY_HEAP_HPP
