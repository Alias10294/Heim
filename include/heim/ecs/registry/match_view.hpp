#ifndef HEIM_ECS_REGISTRY_VIEW_HPP
#define HEIM_ECS_REGISTRY_VIEW_HPP

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
  registry_type *m_reg;

private:
  [[nodiscard]] constexpr
  auto
  m_base()
  noexcept
  {
    return m_reg
         | std::views::filter(
               [this](auto const id)
               { return m_reg->template matches<Expr>(id); });
  }

public:
  constexpr
  match_view()
  noexcept
    : m_reg{}
  { }

  explicit constexpr
  match_view(registry_type &reg)
  noexcept
    : m_reg{std::addressof(reg)}
  { }


  [[nodiscard]] constexpr
  auto
  begin()
  noexcept
  { return std::ranges::begin(m_base()); }

  [[nodiscard]] constexpr
  auto
  end()
  noexcept
  { return std::ranges::end(m_base()); }
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

#endif // HEIM_ECS_REGISTRY_VIEW_HPP
