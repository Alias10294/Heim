#ifndef HEIM_ECS_REGISTRY_VIEWS_HPP
#define HEIM_ECS_REGISTRY_VIEWS_HPP

#include <compare>
#include <concepts>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>

namespace heim::ecs
{
/*!
 * \brief
 *   A view of a specializing registry type instance without the identifiers that fail to match the
 *   specializing expression type.
 *
 * \note
 *   This type should be partially specialized for specific registry types, as this default view
 *   filters throughout all its registry instance.
 */
template<typename Reg, typename Expr>
class match_view
  : public std::ranges::view_interface<match_view<Reg, Expr>>
{
public:
  using registry_type   = Reg;
  using expression_type = Expr;

private:
  struct iterator
  {
  private:
    using iterator_type
    = std::ranges::iterator_t<registry_type>;

  private:
    registry_type *m_reg;
    iterator_type  m_it;

  private:
    [[nodiscard]] constexpr
    iterator_type
    increment(iterator_type it) const
    noexcept
    {
      auto const end{std::ranges::end(*m_reg)};

      while (!m_reg->template matches<Expr>(*it) && it != end)
        ++it;
      return it;
    }

    [[nodiscard]] constexpr
    iterator_type
    decrement(iterator_type it) const
    noexcept
    {
      auto const begin{std::ranges::begin(*m_reg)};

      while (!m_reg->template matches<Expr>(*it) && it != begin)
        --it;
      return it;
    }

  public:
    constexpr
    iterator()
    = default;

    constexpr
    iterator(registry_type *reg, iterator_type it)
      : m_reg{reg}
      , m_it {increment(std::move(it))}
    { }

    friend constexpr
    bool
    operator==(iterator const &lhs, iterator const &rhs)
    noexcept
    requires std::equality_comparable<iterator_type>
    { return lhs.m_it == rhs.m_it; }

    friend constexpr
    auto
    operator<=>(iterator const &lhs, iterator const &rhs)
    noexcept
    requires std::three_way_comparable<iterator_type>
    { return lhs.m_it <=> rhs.m_it; }

    friend constexpr
    std::ranges::range_rvalue_reference_t<registry_type>
    iter_move(iterator const &it)
    noexcept(noexcept(std::ranges::iter_move(it.m_it)))
    { return std::ranges::iter_move(it.m_it); }

    friend constexpr
    void
    iter_swap(iterator const &lhs, iterator const &rhs)
    noexcept(noexcept(std::ranges::iter_swap(lhs.m_it, rhs.m_it)))
    { return std::ranges::iter_swap(lhs.m_it, rhs.m_it); }


    constexpr
    std::ranges::range_reference_t<registry_type>
    operator*() const
    noexcept
    { return *m_it; }


    constexpr
    iterator &
    operator++()
    noexcept
    {
      m_it = increment(std::move(++m_it));
      return *this;
    }

    constexpr
    void
    operator++(int)
    noexcept
    { ++*this; }

    constexpr
    iterator
    operator++(int)
    noexcept
    requires std::forward_iterator<iterator_type>
    {
      iterator tmp{*this};
      ++*this;
      return tmp;
    }

    constexpr
    iterator &
    operator--()
    noexcept
    requires std::bidirectional_iterator<iterator_type>
    {
      m_it = decrement(std::move(--m_it));
      return *this;
    }

    constexpr
    iterator
    operator--(int)
    noexcept
    requires std::bidirectional_iterator<iterator_type>
    {
      iterator tmp{*this};
      --*this;
      return tmp;
    }
  };

private:
  registry_type *m_reg;

public:
  constexpr
  match_view()
  = default;

  explicit constexpr
  match_view(registry_type &reg)
    : m_reg{std::addressof(reg)}
  { }


  [[nodiscard]] constexpr
  iterator
  begin() const
  noexcept
  { return iterator{m_reg, std::ranges::begin(*m_reg)}; }

  [[nodiscard]] constexpr
  iterator
  end() const
  noexcept
  { return iterator{m_reg, std::ranges::end(*m_reg)}; }

  [[nodiscard]] constexpr
  auto
  reserve_hint() const
  noexcept
  { return std::ranges::size(*m_reg); }
};


namespace views
{
namespace detail
{
template<typename Expr>
struct match_fn
  : std::ranges::range_adaptor_closure<match_fn<Expr>>
{
  template<typename Reg>
  [[nodiscard]] constexpr
  auto
  operator()(Reg &reg) const
  noexcept
  { return match_view<Reg, Expr>{reg}; }
};

} // namespace detail

template<typename Expr>
inline constexpr
detail::match_fn<Expr>
match;

} // namespace views
} // namespace heim::ecs

#endif // HEIM_ECS_REGISTRY_VIEWS_HPP
