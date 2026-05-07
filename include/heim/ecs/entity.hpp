#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include "registry.hpp"

namespace heim
{
template<typename Registry>
requires registry<std::remove_cvref_t<Registry>>
class entity
{
public:
  using registry_type   = Registry;
  using identifier_type = typename registry_type::identifier_type;

private:
  registry_type  *m_registry;
  identifier_type m_identifier;

public:
  constexpr
  entity()
  noexcept
    : m_registry  {nullptr}
    , m_identifier{}
  { }

  constexpr
  entity(entity const &)
  = default;

  constexpr
  entity(entity &&)
  = default;

  constexpr
  entity(registry_type &registry, identifier_type const identifier)
  noexcept
    : m_registry  {&registry}
    , m_identifier{identifier}
  { }

  explicit constexpr
  entity(registry_type &registry)
    : entity{registry, registry.create()}
  { }

  constexpr
  ~entity()
  = default;

  constexpr
  entity &
  operator=(entity const &)
  = default;

  constexpr
  entity &
  operator=(entity &&)
  = default;

  constexpr
  void
  swap(entity &other)
  noexcept
  {
    using std::swap;

    swap(m_registry  , other.m_registry);
    swap(m_identifier, other.m_identifier);
  }

  friend constexpr
  void
  swap(entity &lhs, entity &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(entity const &, entity const &)
  = default;


  [[nodiscard]] constexpr
  registry_type &
  registry() const
  noexcept
  { return *m_registry; }

  [[nodiscard]] constexpr
  identifier_type
  identifier() const
  noexcept
  { return m_identifier; }

  [[nodiscard]] constexpr
  bool
  is_valid() const
  noexcept
  { return m_registry->is_valid(m_identifier); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  is_match(Expression const = Expression{}) const
  noexcept
  { return m_registry->template is_match<Expression>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get()
  noexcept
  { return m_registry->template get<Component>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get() const
  noexcept
  { return m_registry->template get<Component>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get()
  { return m_registry->template try_get<Component>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get() const
  { return m_registry->template try_get<Component>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if()
  noexcept
  { return m_registry->template get_if<Component>(m_identifier); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if() const
  noexcept
  { return m_registry->template get_if<Component>(m_identifier); }


  constexpr
  void
  create()
  { m_identifier = m_registry->create(); }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(Args &&...args)
  { m_registry->template emplace<Component>(m_identifier, std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(Args &&...args)
  { return m_registry->template try_emplace<Component>(m_identifier, std::forward<Args>(args)...); }

  template<typename Component>
  constexpr
  bool
  insert(Component const &c)
  { return m_registry->template insert<Component>(m_identifier, c); }

  template<typename Component>
  constexpr
  bool
  insert(Component &&c)
  { return m_registry->template insert<Component>(m_identifier, std::move(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(Component const &c)
  { return m_registry->template insert_or_assign<Component>(m_identifier, c); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(Component &&c)
  { return m_registry->template insert_or_assign<Component>(m_identifier, std::move(c)); }

  template<typename Component>
  constexpr
  void
  erase()
  { m_registry->template erase<Component>(m_identifier); }

  template<typename Component>
  constexpr
  bool
  try_erase()
  { return m_registry->template try_erase<Component>(m_identifier); }

  constexpr
  void
  clear()
  { m_registry->clear(m_identifier); }

  constexpr
  void
  destroy()
  { m_registry->destroy(m_identifier); }
};

} // namespace heim

#endif // HEIM_ENTITY_HPP
