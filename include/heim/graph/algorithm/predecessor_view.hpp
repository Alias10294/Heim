#ifndef HEIM_GRAPH_ALGORITHM_PREDECESSOR_VIEW_HPP
#define HEIM_GRAPH_ALGORITHM_PREDECESSOR_VIEW_HPP

#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"

namespace heim::graphs
{
template<
    typename PredecessorMap,
    typename Vertex>
class predecessor_view
  : public std::ranges::view_interface<predecessor_view<PredecessorMap, Vertex>>
{
public:
  class iterator
  {
    friend predecessor_view;

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Vertex;
    using reference         = Vertex;

    class pointer
    {
    public:
      reference r;

    public:
      constexpr
      reference const *
      operator->() const
      noexcept
      { return std::addressof(r); }
    };

  private:
    std::optional<std::ranges::single_view<PredecessorMap>> m_pm; // ensures at least std::input_iterator
    value_type                                              m_v;
    bool                                                    m_ended;

  private:
    constexpr
    iterator(PredecessorMap pm, value_type v, bool ended)
    requires std::move_constructible<value_type>
      : m_pm   {std::in_place, std::move(pm)}
      , m_v    {std::move(v)}
      , m_ended{std::move(ended)}
    { }

  public:
    constexpr
    iterator()
      : m_pm   {}
      , m_v    {}
      , m_ended{true}
    { }

    [[nodiscard]] friend constexpr
    bool
    operator==(iterator const &lhs, iterator const &rhs)
    { return lhs.m_ended == rhs.m_ended && (lhs.m_ended || lhs.m_v == rhs.m_v); }

    [[nodiscard]] friend constexpr
    bool
    operator==(iterator const &it, std::default_sentinel_t)
    noexcept
    { return it.m_ended; }


    constexpr
    reference
    operator*() const
    { return this->m_v; }

    constexpr
    pointer
    operator->() const
    { return pointer{**this}; }


    constexpr
    iterator &
    operator++()
    requires
        properties::readable_map_for<PredecessorMap &, value_type &>
     && std::constructible_from <value_type, properties::readable_map_result_t<PredecessorMap &, value_type &>>
     && std::equality_comparable<value_type>
    {
      value_type next{properties::get(*this->m_pm->data(), this->m_v)};

      if (this->m_v == next)
        this->m_ended = true;
      else
        this->m_v = std::move(next);

      return *this;
    }

    constexpr
    iterator
    operator++(int)
    { iterator tmp{*this}; ++*this; return tmp; }
  };

private:
  std::ranges::single_view<PredecessorMap> m_pm; // ensures std::movable
  Vertex                                   m_v;

public:
  constexpr
  predecessor_view()
  = default;

  constexpr
  predecessor_view(PredecessorMap pm, Vertex v)
  requires std::move_constructible<Vertex>
    : m_pm{std::move(pm)}
    , m_v {std::move(v )}
  { }


  [[nodiscard]] constexpr
  iterator
  begin() const
  { return iterator{*this->m_pm.data(), this->m_v, false}; }

  [[nodiscard]] constexpr
  std::default_sentinel_t
  end() const
  { return std::default_sentinel; }
};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_PREDECESSOR_VIEW_HPP
