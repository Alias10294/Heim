#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <type_traits>
#include <utility>
#include "registry.hpp"
#include "utility.hpp"

namespace heim
{
template<typename Registry>
class entity;

template<typename Registry>
class entity
{
public:
  using registry_type
  = Registry;

  static_assert(
      specializes_registry_v<Registry>,
      "heim::entity: registry_type must be specialization of registry.");

  using identifier_type
  = typename registry_type::identifier_type;

private:
  registry_type  *m_registry;
  identifier_type m_id;

private:
  static constexpr
  bool
  s_noexcept_destroy()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_has()
  noexcept;

  template<typename ...Components> static constexpr bool s_noexcept_has_all_of () noexcept;
  template<typename ...Components> static constexpr bool s_noexcept_has_any_of () noexcept;
  template<typename ...Components> static constexpr bool s_noexcept_has_none_of() noexcept;

  template<typename Component> static constexpr bool s_noexcept_get      () noexcept;
  template<typename Component> static constexpr bool s_noexcept_get_const() noexcept;

  template<typename Component> static constexpr bool s_noexcept_get_if      () noexcept;
  template<typename Component> static constexpr bool s_noexcept_get_if_const() noexcept;

  template<typename Component, typename ...Args> static constexpr bool s_noexcept_emplace    () noexcept;
  template<typename Component, typename ...Args> static constexpr bool s_noexcept_try_emplace() noexcept;

  template<typename Component> static constexpr bool s_noexcept_insert_copy() noexcept;
  template<typename Component> static constexpr bool s_noexcept_insert_move() noexcept;

  template<typename Component> static constexpr bool s_noexcept_insert_or_assign_copy() noexcept;
  template<typename Component> static constexpr bool s_noexcept_insert_or_assign_move() noexcept;

  template<typename Component> static constexpr bool s_noexcept_erase    () noexcept;
  template<typename Component> static constexpr bool s_noexcept_try_erase() noexcept;

  static constexpr
  bool
  s_noexcept_clear()
  noexcept;

public:
  constexpr
  entity()
  noexcept;

  constexpr entity(entity const &) = default;
  constexpr entity(entity &&)      = default;

  constexpr          entity(registry_type *, identifier_type) noexcept;
  constexpr          entity(registry_type &, identifier_type) noexcept;
  constexpr explicit entity(registry_type *);
  constexpr explicit entity(registry_type &);

  constexpr
  ~entity()
  = default;

  constexpr entity &operator=(entity const &) = default;
  constexpr entity &operator=(entity &&)      = default;

  constexpr
  void
  swap(entity &)
  noexcept;


  [[nodiscard]] constexpr identifier_type identifier() const noexcept;
  [[nodiscard]] constexpr registry_type  &registry  () const noexcept;


  [[nodiscard]] constexpr
  bool
  is_valid() const
  noexcept;

  constexpr
  void
  destroy()
  noexcept(s_noexcept_destroy());


  template<typename Component>
  [[nodiscard]] constexpr
  bool
  has() const
  noexcept(s_noexcept_has<Component>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_all_of() const
  noexcept(s_noexcept_has_all_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_any_of() const
  noexcept(s_noexcept_has_any_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_none_of() const
  noexcept(s_noexcept_has_none_of<Components ...>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get()
  noexcept(s_noexcept_get<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get() const
  noexcept(s_noexcept_get_const<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get();

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get() const;

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if()
  noexcept(s_noexcept_get_if<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if() const
  noexcept(s_noexcept_get_if_const<Component>());


  template<
      typename    Component,
      typename ...Args>
  constexpr
  void
  emplace(Args &&...)
  noexcept(s_noexcept_emplace<Component, Args ...>());

  template<
      typename    Component,
      typename ...Args>
  constexpr
  bool
  try_emplace(Args &&...)
  noexcept(s_noexcept_try_emplace<Component, Args ...>());

  template<typename Component>
  constexpr
  bool
  insert(Component const &)
  noexcept(s_noexcept_insert_copy<Component>());

  template<typename Component>
  constexpr
  bool
  insert(Component &&)
  noexcept(s_noexcept_insert_move<Component>());

  template<typename Component>
  constexpr
  bool
  insert_or_assign(Component const &)
  noexcept(s_noexcept_insert_or_assign_copy<Component>());

  template<typename Component>
  constexpr
  bool
  insert_or_assign(Component &&)
  noexcept(s_noexcept_insert_or_assign_move<Component>());

  template<typename Component>
  constexpr
  void
  erase()
  noexcept(s_noexcept_erase<Component>());

  template<typename Component>
  constexpr
  bool
  try_erase()
  noexcept(s_noexcept_try_erase<Component>());

  constexpr
  void
  clear()
  noexcept(s_noexcept_clear());


  friend constexpr
  void
  swap(entity &lhs, entity &rhs)
  noexcept
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(entity const &, entity const &)
  = default;
};


template<typename Registry>
constexpr
bool
entity<Registry>
    ::s_noexcept_destroy()
noexcept
{
  return noexcept(std::declval<registry_type *>()->destroy(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_has()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template has<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::s_noexcept_has_all_of()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template has_all_of<Components ...>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::s_noexcept_has_any_of()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template has_any_of<Components ...>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::s_noexcept_has_none_of()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template has_none_of<Components ...>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_get()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template get<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_get_const()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template get<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_get_if()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template get_if<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_get_if_const()
noexcept
{
  return noexcept(std::declval<registry_type const *>()
      ->template get_if<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
entity<Registry>
    ::s_noexcept_emplace()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...));
}

template<typename Registry>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
entity<Registry>
    ::s_noexcept_try_emplace()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template try_emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_insert_copy()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template insert<Component>(std::declval<identifier_type>(), std::declval<Component const &>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_insert_move()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template insert<Component>(std::declval<identifier_type>(), std::declval<Component &&>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_insert_or_assign_copy()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component const &>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_insert_or_assign_move()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component &&>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_erase()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template erase<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::s_noexcept_try_erase()
noexcept
{
  return noexcept(std::declval<registry_type *>()
      ->template try_erase<Component>(std::declval<identifier_type>()));
}

template<typename Registry>
constexpr
bool
entity<Registry>
    ::s_noexcept_clear()
noexcept
{
  return noexcept(std::declval<registry_type *>()->clear(std::declval<identifier_type>()));
}

template<typename Registry>
constexpr
entity<Registry>
    ::entity()
noexcept
  : m_registry{nullptr},
    m_id      {}
{ }

template<typename Registry>
constexpr
entity<Registry>
    ::entity(registry_type * const reg, identifier_type const id)
noexcept
  : m_registry{reg},
    m_id      (id)
{ }

template<typename Registry>
constexpr
entity<Registry>
    ::entity(registry_type &reg, identifier_type const id)
noexcept
  : entity(&reg, id)
{ }

template<typename Registry>
constexpr
entity<Registry>
    ::entity(registry_type *reg)
  : entity(reg, reg->create())
{ }

template<typename Registry>
constexpr
entity<Registry>
    ::entity(registry_type &reg)
  : entity(&reg)
{ }

template<typename Registry>
constexpr
void
entity<Registry>
    ::swap(entity &other)
noexcept
{
  using std::swap;

  swap(m_registry, other.m_registry);
  swap(m_id      , other.m_id      );
}

template<typename Registry>
constexpr
typename entity<Registry>
    ::identifier_type
entity<Registry>
    ::identifier() const
noexcept
{
  return m_id;
}

template<typename Registry>
constexpr
typename entity<Registry>
    ::registry_type &
entity<Registry>
    ::registry() const
noexcept
{
  return *m_registry;
}

template<typename Registry>
constexpr
bool
entity<Registry>::is_valid() const
noexcept
{
  return m_registry->is_valid(m_id);
}

template<typename Registry>
constexpr
void
entity<Registry>
    ::destroy()
noexcept(s_noexcept_destroy())
{
  m_registry->destroy(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>::has() const
noexcept(s_noexcept_has<Component>())
{
  return m_registry->template has<Component>(m_id);
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::has_all_of() const
noexcept(s_noexcept_has_all_of<Components...>())
{
  return m_registry->template has_all_of<Components ...>(m_id);
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::has_any_of() const
noexcept(s_noexcept_has_any_of<Components...>())
{
  return m_registry->template has_any_of<Components ...>(m_id);
}

template<typename Registry>
template<typename ...Components>
constexpr
bool
entity<Registry>
    ::has_none_of() const
noexcept(s_noexcept_has_none_of<Components...>())
{
  return m_registry->template has_none_of<Components ...>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component &
entity<Registry>
    ::get()
noexcept(s_noexcept_get<Component>())
{
  return m_registry->template get<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component const &
entity<Registry>
    ::get() const
noexcept(s_noexcept_get_const<Component>())
{
  return m_registry->template get<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component &
entity<Registry>
    ::try_get()
{
  return m_registry->template try_get<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component const &
entity<Registry>
    ::try_get() const
{
  return m_registry->template try_get<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component *
entity<Registry>
    ::get_if()
noexcept(s_noexcept_get_if<Component>())
{
  return m_registry->template get_if<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
Component const *
entity<Registry>
    ::get_if() const
noexcept(s_noexcept_get_if_const<Component>())
{
  return m_registry->template get_if<Component>(m_id);
}

template<typename Registry>
template<
    typename    Component,
    typename ...Args>
constexpr
void
entity<Registry>
    ::emplace(Args &&...args)
noexcept(s_noexcept_emplace<Component, Args ...>())
{
  return m_registry->template emplace<Component>(m_id, std::forward<Args>(args)...);
}

template<typename Registry>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
entity<Registry>
    ::try_emplace(Args &&...args)
noexcept(s_noexcept_try_emplace<Component, Args ...>())
{
  return m_registry->template try_emplace<Component>(m_id, std::forward<Args>(args)...);
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::insert(Component const &c)
noexcept(s_noexcept_insert_copy<Component>())
{
  return m_registry->template insert<Component>(m_id, c);
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::insert(Component &&c)
noexcept(s_noexcept_insert_move<Component>())
{
  return m_registry->template insert<Component>(m_id, std::forward<Component>(c));
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::insert_or_assign(Component const &c)
noexcept(s_noexcept_insert_or_assign_copy<Component>())
{
  return m_registry->template insert_or_assign<Component>(m_id, c);
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::insert_or_assign(Component &&c)
noexcept(s_noexcept_insert_or_assign_move<Component>())
{
  return m_registry->template insert_or_assign<Component>(m_id, std::forward<Component>(c));
}

template<typename Registry>
template<typename Component>
constexpr
void
entity<Registry>
    ::erase()
noexcept(s_noexcept_erase<Component>())
{
  m_registry->template erase<Component>(m_id);
}

template<typename Registry>
template<typename Component>
constexpr
bool
entity<Registry>
    ::try_erase()
noexcept(s_noexcept_try_erase<Component>())
{
  return m_registry->template try_erase<Component>(m_id);
}

template<typename Registry>
constexpr
void
entity<Registry>
    ::clear()
noexcept(s_noexcept_clear())
{
  m_registry->clear(m_id);
}


/*!
 * @brief Determines whether the given type is a specialization of entity.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_entity;

template<typename T>
struct specializes_entity
  : bool_constant<false>
{ };

template<typename Registry>
struct specializes_entity<
    entity<Registry>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
specializes_entity_v
= specializes_entity<T>::value;


}

#endif // HEIM_ENTITY_HPP