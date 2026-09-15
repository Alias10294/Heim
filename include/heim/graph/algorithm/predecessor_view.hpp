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
  : std::ranges::view_interface<predecessor_view<PredecessorMap, Vertex>>
{
public:
  class iterator
  {
    friend predecessor_view;

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::forward_iterator_tag;
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
    PredecessorMap *m_p;
    value_type      m_v;
    bool            m_ended;

  private:
    constexpr
    iterator(PredecessorMap *p, value_type v)
    requires std::move_constructible<value_type>
      : m_p{p}, m_v{std::move(v)}, m_ended{p == nullptr}
    { }

  public:
    constexpr
    iterator()
      : m_p{}, m_v{}, m_ended{true}
    { }

    [[nodiscard]] friend constexpr
    bool
    operator==(iterator const &, iterator const &)
    = default;

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
      value_type next{properties::get(*this->m_p, this->m_v)};

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
  PredecessorMap *m_p;
  Vertex          m_v;

public:
  constexpr
  predecessor_view()
    : m_p{}, m_v{}
  { }

  constexpr
  predecessor_view(PredecessorMap *p, Vertex v)
  requires std::move_constructible<Vertex>
    : m_p{p}, m_v{std::move(v)}
  { }

  constexpr
  predecessor_view(PredecessorMap &p, Vertex v)
    : predecessor_view{std::addressof(p), std::move(v)}
  { }


  [[nodiscard]] constexpr
  iterator
  begin() const
  requires std::copyable<Vertex>
  { return iterator{m_p, m_v}; }

  [[nodiscard]] constexpr
  std::default_sentinel_t
  end() const
  { return std::default_sentinel; }
};

} // namespace heim::graphs


namespace std::ranges
{
template<
    typename PredecessorMap,
    typename Vertex>
inline constexpr bool
enable_borrowed_range<heim::graphs::predecessor_view<PredecessorMap, Vertex>>
= true;

} // namespace std::ranges

#endif // HEIM_GRAPH_ALGORITHM_PREDECESSOR_VIEW_HPP
