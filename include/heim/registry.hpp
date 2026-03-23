#ifndef HEIM_REGISTRY_HPP
#define HEIM_REGISTRY_HPP

#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "identifier_manager.hpp"
#include "query_expression.hpp"
#include "utility.hpp"

namespace heim
{
/*!
 * @brief The main container in the context of an entity-component-system pattern, allowing for
 *   management of both entities and their components.
 *
 * @details Provides full customization not only in the included component types themselves, but in
 *   the storage itself, being completely interchangeable.
 *   For a type to pass as a storage type for the registry, it only needs to expose a certain
 *   interface that the registry can use.
 *
 * @tparam Storage The storage type for all components.
 */
template<typename Storage>
class registry;

template<typename Storage>
class registry
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;


  using storage_type
  = Storage;

  using identifier_type = typename storage_type::identifier_type;
  using allocator_type  = typename storage_type::allocator_type;

  template<typename Expression> using query_type       = typename storage_type::template query_type      <Expression>;
  template<typename Expression> using const_query_type = typename storage_type::template const_query_type<Expression>;

private:
  using identifier_manager_type
  = identifier_manager<identifier_type, allocator_type>;

private:
  identifier_manager_type m_id_mgr;
  storage_type            m_storage;

private:
  template<typename ...Components> static constexpr bool s_implements_has_all_of () noexcept;
  template<typename ...Components> static constexpr bool s_implements_has_any_of () noexcept;
  template<typename ...Components> static constexpr bool s_implements_has_none_of() noexcept;

  template<typename Component> static constexpr bool s_implements_get_if      () noexcept;
  template<typename Component> static constexpr bool s_implements_get_if_const() noexcept;

  template<
      typename    Component,
      typename ...Args>
  static constexpr
  bool
  s_implements_try_emplace()
  noexcept;

  template<typename Component> static constexpr bool s_implements_insert_copy() noexcept;
  template<typename Component> static constexpr bool s_implements_insert_move() noexcept;

  template<typename Component> static constexpr bool s_implements_insert_or_assign_copy() noexcept;
  template<typename Component> static constexpr bool s_implements_insert_or_assign_move() noexcept;

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  static constexpr
  bool
  s_implements_erase()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_implements_try_erase()
  noexcept;

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  static constexpr
  bool
  s_implements_try_erase()
  noexcept;


  static constexpr bool s_noexcept_alloc_construct     () noexcept;
  static constexpr bool s_noexcept_default_construct   () noexcept;
  static constexpr bool s_noexcept_move_alloc_construct() noexcept;
  static constexpr bool s_noexcept_destroy() noexcept;

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

  template<typename Expression> static constexpr bool s_noexcept_query      () noexcept;
  template<typename Expression> static constexpr bool s_noexcept_query_const() noexcept;

  template<typename Component, typename ...Args> static constexpr bool s_noexcept_emplace    () noexcept;
  template<typename Component, typename ...Args> static constexpr bool s_noexcept_try_emplace() noexcept;

  template<typename Component> static constexpr bool s_noexcept_insert_copy() noexcept;
  template<typename Component> static constexpr bool s_noexcept_insert_move() noexcept;

  template<typename Component> static constexpr bool s_noexcept_insert_or_assign_copy() noexcept;
  template<typename Component> static constexpr bool s_noexcept_insert_or_assign_move() noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_erase()
  noexcept;

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  static constexpr
  bool
  s_noexcept_erase()
  noexcept;

  template<
      typename Component,
      typename Range>
  static constexpr
  bool
  s_noexcept_erase()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_try_erase()
  noexcept;

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  static constexpr
  bool
  s_noexcept_try_erase()
  noexcept;

  template<
      typename Component,
      typename Range>
  static constexpr
  bool
  s_noexcept_try_erase()
  noexcept;

  static constexpr bool s_noexcept_clear_identifier() noexcept;
  static constexpr bool s_noexcept_clear()            noexcept;
  static constexpr bool s_noexcept_swap ()            noexcept;

public:
  constexpr explicit
  registry(allocator_type const &)
  noexcept(s_noexcept_alloc_construct());

  constexpr
  registry()
  noexcept(s_noexcept_default_construct());

  constexpr registry(registry const &) = default;
  constexpr registry(registry &&)      = default;

  constexpr
  registry(registry const &, allocator_type const &);

  constexpr
  registry(registry &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  ~registry()
  = default;

  constexpr registry &operator=(registry const &) = default;
  constexpr registry &operator=(registry &&)      = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(registry &)
  noexcept(s_noexcept_swap());


  [[nodiscard]] constexpr storage_type       &get_storage()       noexcept;
  [[nodiscard]] constexpr storage_type const &get_storage() const noexcept;


  [[nodiscard]] constexpr
  bool
  is_valid(identifier_type) const
  noexcept;

  [[nodiscard]] constexpr
  identifier_type
  create();

  template<
      typename Iterator,
      typename Sentinel>
  constexpr
  void
  create(Iterator, Sentinel);

  template<typename Range>
  constexpr
  void
  create(Range &&);

  constexpr
  void
  destroy(identifier_type)
  noexcept(s_noexcept_destroy());

  template<
      typename Iterator,
      typename Sentinel>
  constexpr
  void
  destroy(Iterator, Sentinel)
  noexcept(s_noexcept_destroy());

  template<typename Range>
  constexpr
  void
  destroy(Range &&)
  noexcept(s_noexcept_destroy());


  template<typename Component>
  [[nodiscard]] constexpr
  bool
  has(identifier_type) const
  noexcept(s_noexcept_has<Component>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_all_of(identifier_type) const
  noexcept(s_noexcept_has_all_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_any_of(identifier_type) const
  noexcept(s_noexcept_has_any_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_none_of(identifier_type) const
  noexcept(s_noexcept_has_none_of<Components ...>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(identifier_type)
  noexcept(s_noexcept_get<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(identifier_type) const
  noexcept(s_noexcept_get_const<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get(identifier_type);

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get(identifier_type) const;

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(identifier_type)
  noexcept(s_noexcept_get_if<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(identifier_type) const
  noexcept(s_noexcept_get_if_const<Component>());


  template<typename Expression>
  [[nodiscard]] constexpr
  query_type<Expression>
  query()
  noexcept(s_noexcept_query<Expression>());

  template<typename Expression>
  [[nodiscard]] constexpr
  const_query_type<Expression>
  query() const
  noexcept(s_noexcept_query_const<Expression>());


  template<
      typename    Component,
      typename ...Args>
  constexpr
  void
  emplace(identifier_type, Args &&...)
  noexcept(s_noexcept_emplace<Component, Args ...>());

  template<
      typename    Component,
      typename ...Args>
  constexpr
  bool
  try_emplace(identifier_type, Args &&...)
  noexcept(s_noexcept_try_emplace<Component, Args ...>());

  template<typename Component>
  constexpr
  bool
  insert(identifier_type, Component const &)
  noexcept(s_noexcept_insert_copy<Component>());

  template<typename Component>
  constexpr
  bool
  insert(identifier_type, Component &&)
  noexcept(s_noexcept_insert_move<Component>());

  template<typename Component>
  constexpr
  bool
  insert_or_assign(identifier_type, Component const &)
  noexcept(s_noexcept_insert_or_assign_copy<Component>());

  template<typename Component>
  constexpr
  bool
  insert_or_assign(identifier_type, Component &&)
  noexcept(s_noexcept_insert_or_assign_move<Component>());

  template<typename Component>
  constexpr
  void
  erase(identifier_type)
  noexcept(s_noexcept_erase<Component>());

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  constexpr
  void
  erase(Iterator, Sentinel)
  noexcept(s_noexcept_erase<Component, Iterator, Sentinel>());

  template<
      typename Component,
      typename Range>
  constexpr
  void
  erase(Range &&)
  noexcept(s_noexcept_erase<Component, Range>());

  template<typename Component>
  constexpr
  bool
  try_erase(identifier_type)
  noexcept(s_noexcept_try_erase<Component>());

  template<
      typename Component,
      typename Iterator,
      typename Sentinel>
  constexpr
  void
  try_erase(Iterator, Sentinel)
  noexcept(s_noexcept_try_erase<Component, Iterator, Sentinel>());

  template<
      typename Component,
      typename Range>
  constexpr
  void
  try_erase(Range &&)
  noexcept(s_noexcept_try_erase<Component, Range>());

  constexpr
  void
  clear(identifier_type)
  noexcept(s_noexcept_clear_identifier());

  constexpr
  void
  clear()
  noexcept(s_noexcept_clear());


  friend constexpr
  void
  swap(registry &lhs, registry &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry const &, registry const &)
  = default;
};


template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_implements_has_all_of()
noexcept
{
  return requires
  {
    std::declval<storage_type const &>()
        .template has_all_of<Components ...>(std::declval<identifier_type>());
  };
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_implements_has_any_of()
noexcept
{
  return requires
  {
    std::declval<storage_type const &>()
        .template has_any_of<Components ...>(std::declval<identifier_type>());
  };
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_implements_has_none_of()
noexcept
{
  return requires
  {
    std::declval<storage_type const &>()
        .template has_none_of<Components ...>(std::declval<identifier_type>());
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_get_if()
noexcept
{
  return requires
  {
    std::declval<storage_type &>()
        .template get_if<Component>(std::declval<identifier_type>());
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_get_if_const()
noexcept
{
  return requires
  {
    std::declval<storage_type const &>()
        .template get_if<Component>(std::declval<identifier_type>());
  };
}

template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
registry<Storage>
    ::s_implements_try_emplace()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
          .template try_emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...) }
          -> std::same_as<bool>;
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_insert_copy()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
        .template insert<Component>(std::declval<identifier_type>(), std::declval<Component const &>()) }
        -> std::same_as<bool>;
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_insert_move()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
        .template insert<Component>(std::declval<identifier_type>(), std::declval<Component &&>()) }
        -> std::same_as<bool>;
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_insert_or_assign_copy()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
        .template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component const &>()) }
        -> std::same_as<bool>;
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_insert_or_assign_move()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
        .template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component &&>()) }
        -> std::same_as<bool>;
  };
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
bool
registry<Storage>
    ::s_implements_erase()
noexcept
{
  return requires
  {
    std::declval<storage_type &>()
        .template erase<Component>(std::declval<Iterator>(), std::declval<Sentinel>());
  };
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_implements_try_erase()
noexcept
{
  return requires
  {
    { std::declval<storage_type &>()
        .template try_erase<Component>(std::declval<identifier_type>()) }
        -> std::same_as<bool>;
  };
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
bool
registry<Storage>
    ::s_implements_try_erase()
noexcept
{
  return requires
  {
    std::declval<storage_type &>()
        .template try_erase<Component>(std::declval<Iterator>(), std::declval<Sentinel>());
  };
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<storage_type, allocator_type const &>;
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_default_construct()
noexcept
{
  return s_noexcept_alloc_construct()
      && std::is_nothrow_default_constructible_v<allocator_type>;
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return
      std::is_nothrow_constructible_v<
          identifier_manager_type,
          identifier_manager_type &&, allocator_type const &>
   && std::is_nothrow_constructible_v<
          storage_type,
          storage_type &&, allocator_type const &>;
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_destroy()
noexcept
{
  return noexcept(std::declval<storage_type &>().clear(std::declval<identifier_type>()));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_has()
noexcept
{
  return noexcept(std::declval<storage_type const &>()
        .template has<Component>(std::declval<identifier_type>()));
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_noexcept_has_all_of()
noexcept
{
  if constexpr (s_implements_has_all_of<Components ...>())
  {
    return noexcept(std::declval<storage_type const &>()
          .template has_all_of<Components ...>(std::declval<identifier_type const>()));
  }
  else
    return (s_noexcept_has<Components>() && ...);
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_noexcept_has_any_of()
noexcept
{
  if constexpr (s_implements_has_any_of<Components ...>())
  {
    return noexcept(std::declval<storage_type const &>()
          .template has_any_of<Components ...>(std::declval<identifier_type const>()));
  }
  else
    return (s_noexcept_has<Components>() && ...);
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_noexcept_has_none_of()
noexcept
{
  if constexpr (s_implements_has_none_of<Components ...>())
  {
    return noexcept(std::declval<storage_type const &>()
          .template has_none_of<Components ...>(std::declval<identifier_type const>()));
  }
  else
    return (s_noexcept_has<Components>() && ...);
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get()
noexcept
{
  return noexcept(std::declval<storage_type &>()
      .template get<Component>(std::declval<identifier_type>()));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get_const()
noexcept
{
  return noexcept(std::declval<storage_type const &>()
      .template get<Component>(std::declval<identifier_type>()));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get_if()
noexcept
{
  if constexpr (s_implements_get_if<Component>())
  {
    return noexcept(std::declval<storage_type &>()
          .template get_if<Component>(std::declval<identifier_type const>()));
  }
  else
  {
    return s_noexcept_has<Component>()
        && s_noexcept_get<Component>();
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get_if_const()
noexcept
{
  if constexpr (s_implements_get_if_const<Component>())
  {
    return noexcept(std::declval<storage_type const &>()
          .template get_if<Component>(std::declval<identifier_type const>()));
  }
  else
  {
    return s_noexcept_has      <Component>()
        && s_noexcept_get_const<Component>();
  }
}

template<typename Storage>
template<typename Expression>
constexpr
bool
registry<Storage>
    ::s_noexcept_query()
noexcept
{
  return noexcept(std::declval<storage_type &>()
        .template query<Expression>());
}

template<typename Storage>
template<typename Expression>
constexpr
bool
registry<Storage>
    ::s_noexcept_query_const()
noexcept
{
  return noexcept(std::declval<storage_type const &>()
        .template query<Expression>());
}

template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
registry<Storage>
    ::s_noexcept_emplace()
noexcept
{
  return noexcept(std::declval<storage_type &>()
        .template emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...));
}

template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
registry<Storage>
    ::s_noexcept_try_emplace()
noexcept
{
  if constexpr (s_implements_try_emplace<Component, Args ...>())
  {
    return noexcept(std::declval<storage_type &>()
        .template try_emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...));
  }
  else
  {
    return s_noexcept_has<Component>()
        && s_noexcept_emplace<Component, Args ...>();
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_insert_copy()
noexcept
{
  if constexpr (s_implements_insert_copy<Component>())
  {
    return noexcept(std::declval<storage_type &>()
          .template insert<Component>(std::declval<identifier_type>(), std::declval<Component const &>()));
  }
  else
    return s_noexcept_try_emplace<Component, Component const &>();
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_insert_move()
noexcept
{
  if constexpr (s_implements_insert_move<Component>())
  {
    return noexcept(std::declval<storage_type &>()
          .template insert<Component>(std::declval<identifier_type>(), std::declval<Component &&>()));
  }
  else
    return s_noexcept_try_emplace<Component, Component &&>();
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_insert_or_assign_copy()
noexcept
{
  if constexpr (s_implements_insert_or_assign_copy<Component>())
  {
    return noexcept(std::declval<storage_type &>()
          .template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component const &>()));
  }
  else
  {
    return s_noexcept_has<Component>()
        && std::is_nothrow_copy_assignable_v<Component>
        && s_noexcept_emplace<Component, Component const &>();
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_insert_or_assign_move()
noexcept
{
  if constexpr (s_implements_insert_or_assign_move<Component>())
  {
    return noexcept(std::declval<storage_type &>()
          .template insert_or_assign<Component>(std::declval<identifier_type>(), std::declval<Component &&>()));
  }
  else
  {
    return s_noexcept_has<Component>()
        && std::is_nothrow_move_assignable_v<Component>
        && s_noexcept_emplace<Component, Component &&>();
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_erase()
noexcept
{
  return noexcept(std::declval<storage_type &>()
        .template erase<Component>(std::declval<identifier_type>()));
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
bool
registry<Storage>
    ::s_noexcept_erase()
noexcept
{
  static constexpr
  bool
  implements_erase
  = requires
  {
    std::declval<storage_type &>()
        .template erase<Component, Iterator, Sentinel>(std::declval<Iterator>(), std::declval<Sentinel>());
  };

  if constexpr (implements_erase)
  {
    return noexcept(std::declval<storage_type &>()
          .template erase<Component, Iterator, Sentinel>(std::declval<Iterator>(), std::declval<Sentinel>()));
  }
  else
    return s_noexcept_erase<Component>();
}

template<typename Storage>
template<
    typename Component,
    typename Range>
constexpr
bool
registry<Storage>
    ::s_noexcept_erase()
noexcept
{
  return s_noexcept_erase<
      Component,
      std::ranges::iterator_t<Range>,
      std::ranges::sentinel_t<Range>>();
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_try_erase()
noexcept
{
  if constexpr (s_implements_try_erase<Component>())
  {
    return noexcept(std::declval<storage_type &>()
        .template try_erase<Component>(std::declval<identifier_type>()));
  }
  else
  {
    return s_noexcept_has  <Component>()
        && s_noexcept_erase<Component>();
  }
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
bool
registry<Storage>
    ::s_noexcept_try_erase()
noexcept
{
  if constexpr (s_implements_try_erase<Component, Iterator, Sentinel>())
  {
    return noexcept(std::declval<storage_type &>()
        .template try_erase<Component>(std::declval<Iterator>(), std::declval<Sentinel>()));
  }
  else
    return s_noexcept_try_erase<Component>();
}

template<typename Storage>
template<
    typename Component,
    typename Range>
constexpr
bool
registry<Storage>
    ::s_noexcept_try_erase()
noexcept
{
  return s_noexcept_try_erase<
      Component,
      std::ranges::iterator_t<Range>,
      std::ranges::sentinel_t<Range>>();
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_clear_identifier()
noexcept
{
  return noexcept(std::declval<storage_type &>().clear(std::declval<identifier_type>()));
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_clear()
noexcept
{
  return noexcept(std::declval<storage_type &>().clear());
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<identifier_manager_type>
      && std::is_nothrow_swappable_v<storage_type       >;
}

template<typename Storage>
constexpr
registry<Storage>
    ::registry(allocator_type const &alloc)
noexcept(s_noexcept_alloc_construct())
  : m_id_mgr (alloc),
    m_storage(alloc)
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry()
noexcept(s_noexcept_default_construct())
  : registry(allocator_type{})
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry(registry const &other, allocator_type const &alloc)
  : m_id_mgr (other.m_id_mgr , alloc),
    m_storage(other.m_storage, alloc)
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry(registry &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_id_mgr (std::move(other.m_id_mgr ), alloc),
    m_storage(std::move(other.m_storage), alloc)
{ }

template<typename Storage>
constexpr
typename registry<Storage>
    ::allocator_type
registry<Storage>
    ::get_allocator() const
noexcept
{
  return m_id_mgr.get_allocator();
}

template<typename Storage>
constexpr
void
registry<Storage>
    ::swap(registry &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  static_assert(
      std::is_swappable_v<storage_type>,
      "heim::registry::swap: storage_type must be swappable.");

  swap(m_storage, other.m_storage);
  swap(m_id_mgr , other.m_id_mgr );
}

template<typename Storage>
constexpr
typename registry<Storage>
    ::storage_type &
registry<Storage>
    ::get_storage()
noexcept
{
  return m_storage;
}

template<typename Storage>
constexpr
typename registry<Storage>
    ::storage_type const &
registry<Storage>
    ::get_storage() const
noexcept
{
  return m_storage;
}

template<typename Storage>
constexpr
bool
registry<Storage>
    ::is_valid(identifier_type const e) const
noexcept
{
  return m_id_mgr.is_valid(e);
}

template<typename Storage>
constexpr
typename registry<Storage>
    ::identifier_type
registry<Storage>
    ::create()
{
  identifier_type const
  id
  = m_id_mgr.summon();

  static constexpr
  bool
  implements_emplace
  = requires
  {
    std::declval<storage_type &>().emplace(std::declval<identifier_type>());
  };

  // depending on the storage type it might need to be introduced to the entity
  if constexpr (implements_emplace)
  {
    static constexpr
    bool
    noexcept_emplace
    = noexcept(std::declval<storage_type &>().emplace(std::declval<identifier_type>()));

    if constexpr (noexcept_emplace)
      m_storage.emplace(id);
    else
    {
      // ensure "strong" (basic in reality but strong in this context) exception safety guarantee
      try
      { m_storage.emplace(id); }
      catch (...)
      { m_id_mgr.banish(id); throw; }
    }
  }

  return id;
}

template<typename Storage>
template<
    typename Iterator,
    typename Sentinel>
constexpr
void
registry<Storage>
    ::create(Iterator first, Sentinel last)
{
  static_assert(
      std::output_iterator<Iterator, identifier_type>,
      "heim::registry::create: Iterator must be an output iterator for identifier_type");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "heim::registry::create: Sentinel must be a sentinel for Iterator.");

  for (; first != last; ++first)
    *first = create();
}

template<typename Storage>
template<typename Range>
constexpr
void
registry<Storage>
    ::create(Range &&r)
{
  static_assert(
      std::ranges::output_range<Range, identifier_type>,
      "heim::registry::create: Range must be an output range for identifier_type.");

  create(std::ranges::begin(r), std::ranges::end(r));
}

template<typename Storage>
constexpr
void
registry<Storage>
    ::destroy(identifier_type const id)
noexcept(s_noexcept_destroy())
{
  static_assert(
      requires { std::declval<storage_type &>().clear(std::declval<identifier_type>()); },
      "heim::registry::destroy: storage_type must expose a clear(identifier_type) method.");

  m_storage.clear (id);
  m_id_mgr .banish(id);
}

template<typename Storage>
template<
    typename Iterator,
    typename Sentinel>
constexpr
void
registry<Storage>
    ::destroy(Iterator first, Sentinel last)
noexcept(s_noexcept_destroy())
{
  static_assert(
      std::input_iterator<Iterator>
   && std::convertible_to<std::iter_reference_t<Iterator>, identifier_type>,
      "heim::registry::destroy: Iterator must be an input iterator dereferenceable to a type convertible "
      "to identifier_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "heim::registry::destroy: Sentinel must be a sentinel for Iterator.");

  for (; first != last; ++first)
    destroy(*first);
}

template<typename Storage>
template<typename Range>
constexpr
void
registry<Storage>
    ::destroy(Range &&r)
noexcept(s_noexcept_destroy())
{
  static_assert(
      std::ranges::input_range<Range>
   && std::convertible_to<std::ranges::range_reference_t<Range>, identifier_type>,
      "heim::registry::destroy: Range must be an input range with an iterator dereferenceable to a type "
      "convertible to identifier_type.");

  destroy(std::ranges::begin(r), std::ranges::end(r));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::has(identifier_type const id) const
noexcept(s_noexcept_has<Component>())
{
  static constexpr
  bool
  implements_has
  = requires
  {
    { std::declval<storage_type const &>()
        .template has<Component>(std::declval<identifier_type>()) }
        -> std::same_as<bool>;
  };

  static_assert(
      implements_has,
      "heim::registry::has: storage_type must expose a has method.");

  return m_storage.template has<Component>(id);
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_all_of(identifier_type const id) const
noexcept(s_noexcept_has_all_of<Components ...>())
{
  static constexpr
  bool
  implements_has_all_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_all_of<Components ...>(std::declval<identifier_type>()) }
        -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_all_of)
    return m_storage.template has_all_of<Components ...>(id);

  return (has<Components>(id) && ...);
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_any_of(identifier_type const id) const
noexcept(s_noexcept_has_any_of<Components ...>())
{
  static constexpr
  bool
  implements_has_any_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_any_of<Components ...>(std::declval<identifier_type>()) }
        -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_any_of)
    return m_storage.template has_any_of<Components ...>(id);

  return (has<Components>(id) || ...);
}

template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_none_of(identifier_type const id) const
noexcept(s_noexcept_has_none_of<Components ...>())
{
  static constexpr
  bool
  implements_has_none_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_none_of<Components ...>(std::declval<identifier_type>()) }
        -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_none_of)
    return m_storage.template has_none_of<Components ...>(id);

  return (!has<Components>(id) && ...);
}

template<typename Storage>
template<typename Component>
constexpr
Component &
registry<Storage>
    ::get(identifier_type const id)
noexcept(s_noexcept_get<Component>())
{
  static constexpr
  bool
  implements_get
  = requires
  {
    std::declval<storage_type &>().template get<Component>(std::declval<identifier_type>());
  };

  static_assert(
      implements_get,
      "heim::registry::get: storage_type must expose a get method.");

  return m_storage.template get<Component>(id);
}

template<typename Storage>
template<typename Component>
constexpr
Component const &
registry<Storage>
    ::get(identifier_type const id) const
noexcept(s_noexcept_get_const<Component>())
{
  static constexpr
  bool
  implements_get_const
  = requires
  {
    std::declval<storage_type const &>().template get<Component>(std::declval<identifier_type>());
  };

  static_assert(
      implements_get_const,
      "heim::registry::get: storage_type must expose a get method.");
  return m_storage.template get<Component>(id);
}

template<typename Storage>
template<typename Component>
constexpr
Component &
registry<Storage>
    ::try_get(identifier_type const id)
{
  static constexpr
  bool
  implements_try_get
  = requires
  {
    { std::declval<storage_type &>()
        .template try_get<Component>(std::declval<identifier_type>()) }
        -> std::convertible_to<Component &>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_try_get)
    return m_storage.template try_get<Component>(id);

  if (has<Component>(id))
    return get<Component>(id);

  throw std::out_of_range("heim::registry::try_get");
}

template<typename Storage>
template<typename Component>
constexpr
Component const &
registry<Storage>
    ::try_get(identifier_type const id) const
{
  static constexpr
  bool
  implements_try_get_const
  = requires
  {
    { std::declval<storage_type const &>()
        .template try_get<Component>(std::declval<identifier_type>()) }
        -> std::convertible_to<Component const &>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_try_get_const)
    return m_storage.template try_get<Component>(id);

  if (has<Component>(id))
    return get<Component>(id);

  throw std::out_of_range("heim::registry::try_get");
}

template<typename Storage>
template<typename Component>
constexpr
Component *
registry<Storage>
    ::get_if(identifier_type const id)
noexcept(s_noexcept_get_if<Component>())
{
  // the storage can implement this method for specific behavior
  if constexpr (s_implements_get_if<Component>())
    return m_storage.template get_if<Component>(id);

  if (has<Component>(id))
    return std::addressof(get<Component>(id));

  return nullptr;
}

template<typename Storage>
template<typename Component>
constexpr
Component const *
registry<Storage>
    ::get_if(identifier_type const id) const
noexcept(s_noexcept_get_if_const<Component>())
{
  // the storage can implement this method for specific behavior
  if constexpr (s_implements_get_if_const<Component>())
    return m_storage.template get_if<Component>(id);

  if (has<Component>(id))
    return std::addressof(get<Component>(id));

  return nullptr;
}

template<typename Storage>
template<typename Expression>
constexpr
typename registry<Storage>
    ::template query_type<Expression>
registry<Storage>
    ::query()
noexcept(s_noexcept_query<Expression>())
{
  static_assert(
      specializes_query_expression_v<Expression>,
      "heim::registry::query: Expression must be a specialization of query_expression.");

  static constexpr
  bool
  implements_query
  = requires
  {
    { std::declval<storage_type &>().template query<Expression>() } -> std::same_as<query_type<Expression>>;
  };

  static_assert(
      implements_query,
      "heim::registry::query: storage_type must expose a query method.");

  static_assert(
      std::ranges::forward_range<query_type<Expression>>,
      "heim::registry::query: the returned query type must pass as a forward range.");
  static_assert(
      std::is_same_v<
          std::ranges::range_value_t<query_type<Expression>>,
          typename Expression::template value_type<identifier_type>>,
      "heim::registry::query: the returned query type must expose a coherent value_type alias.");
  static_assert(
      std::is_same_v<
          std::ranges::range_reference_t<query_type<Expression>>,
          typename Expression::template reference<identifier_type>>,
      "heim::registry::query: the returned query type must expose a coherent reference alias.");
  static_assert(
      std::is_same_v<
          std::ranges::range_const_reference_t<query_type<Expression>>,
          typename Expression::template const_reference<identifier_type>>,
      "heim::registry::query: the returned query type must expose a coherent const_reference alias.");

  return m_storage.template query<Expression>();
}

template<typename Storage>
template<typename Expression>
constexpr
typename registry<Storage>
    ::template const_query_type<Expression>
registry<Storage>
    ::query() const
noexcept(s_noexcept_query_const<Expression>())
{
  static_assert(
      specializes_query_expression_v<Expression>,
      "heim::registry::query: Expression must be a specialization of query_expression.");

  static constexpr
  bool
  implements_query_const
  = requires
  {
    { std::declval<storage_type const &>().template query<Expression>() } -> std::same_as<const_query_type<Expression>>;
  };

  static_assert(
      implements_query_const,
      "heim::registry::query: storage_type must expose a query method.");

  static_assert(
      std::ranges::forward_range<const_query_type<Expression>>,
      "heim::registry::query: the returned query type must pass as a forward range.");
  static_assert(
      std::is_same_v<
          std::ranges::range_value_t<const_query_type<Expression>>,
          typename Expression::value_type>,
      "heim::registry::query: the returned query type must expose a coherent value_type alias.");
  static_assert(
      std::is_same_v<
          std::ranges::range_reference_t<const_query_type<Expression>>,
          typename Expression::reference>,
      "heim::registry::query: the returned query type must expose a coherent reference alias.");
  static_assert(
      std::is_same_v<
          std::ranges::range_const_reference_t<const_query_type<Expression>>,
          typename Expression::const_reference>,
      "heim::registry::query: the returned query type must expose a coherent const_reference alias.");

  return m_storage.template query<Expression>();
}

template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
void
registry<Storage>
    ::emplace(identifier_type const id, Args &&...args)
noexcept(s_noexcept_emplace<Component, Args ...>())
{
  static constexpr
  bool
  implements_emplace
  = requires
  {
    std::declval<storage_type &>()
        .template emplace<Component>(std::declval<identifier_type>(), std::declval<Args &&>()...);
  };

  static_assert(
      implements_emplace,
      "heim::registry::emplace: storage_type must expose a emplace method.");

  m_storage.template emplace<Component>(id, std::forward<Args>(args)...);
}

template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
registry<Storage>
    ::try_emplace(identifier_type const id, Args &&...args)
noexcept(s_noexcept_try_emplace<Component, Args ...>())
{
  if constexpr (s_implements_try_emplace<Component, Args ...>())
    return m_storage.template try_emplace<Component>(id, std::forward<Args>(args)...);
  else
  {
    if (has<Component>(id))
      return false;

    emplace<Component>(id, std::forward<Args>(args)...);
    return true;
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::insert(identifier_type const id, Component const &c)
noexcept(s_noexcept_insert_copy<Component>())
{
  if constexpr (s_implements_insert_copy<Component>())
    return m_storage.template insert<Component>(id, c);
  else
    return try_emplace<Component>(id, c);
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::insert(identifier_type const id, Component &&c)
noexcept(s_noexcept_insert_move<Component>())
{
  if constexpr (s_implements_insert_move<Component>())
    return m_storage.template insert<Component>(id, std::forward<Component>(c));

  return try_emplace<Component>(id, std::forward<Component>(c));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::insert_or_assign(identifier_type const id, Component const &c)
noexcept(s_noexcept_insert_or_assign_copy<Component>())
{
  if constexpr (s_implements_insert_or_assign_copy<Component>())
    return m_storage.template insert_or_assign<Component>(id, c);
  else
  {
    if (has<Component>(id))
    {
      get<Component>(id) = c;
      return false;
    }

    emplace<Component>(id, c);
    return true;
  }
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::insert_or_assign(identifier_type const id, Component &&c)
noexcept(s_noexcept_insert_or_assign_move<Component>())
{
  if constexpr (s_implements_insert_or_assign_move<Component>())
    return m_storage.template insert_or_assign<Component>(id, std::forward<Component>(c));
  else
  {
    if (has<Component>(id))
    {
      get<Component>(id) = std::forward<Component>(c);
      return false;
    }

    emplace<Component>(id, std::forward<Component>(c));
    return true;
  }
}

template<typename Storage>
template<typename Component>
constexpr
void
registry<Storage>
    ::erase(identifier_type const id)
noexcept(s_noexcept_erase<Component>())
{
  static constexpr
  bool
  implements_erase
  = requires
  {
    std::declval<storage_type &>().template erase<Component>(std::declval<identifier_type>());
  };

  static_assert(
      implements_erase,
      "heim::registry::erase: storage_type must expose an erase method.");

  m_storage.template erase<Component>(id);
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
void
registry<Storage>
    ::erase(Iterator first, Sentinel last)
noexcept(s_noexcept_erase<Component, Iterator,  Sentinel>())
{
  static_assert(
      std::input_iterator<Iterator>
   && std::convertible_to<std::iter_reference_t<Iterator>, identifier_type>,
      "heim::registry::erase: Iterator must be an input iterator dereferenceable to a type convertible "
      "to identifier_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "heim::registry::erase: Sentinel must be a sentinel for Iterator.");

  if constexpr (s_implements_erase<Component, Iterator, Sentinel>())
    m_storage.template erase<Component>(first, last);
  else
  {
    for (; first != last; ++first)
      erase<Component>(*first);
  }
}

template<typename Storage>
template<
    typename Component,
    typename Range>
constexpr
void
registry<Storage>
    ::erase(Range &&r)
noexcept(s_noexcept_erase<Component, Range>())
{
  static_assert(
      std::ranges::input_range<Range>
   && std::convertible_to<std::ranges::range_reference_t<Range>, identifier_type>,
      "heim::registry::erase: Range must be an input range with an iterator dereferenceable to a type "
      "convertible to identifier_type.");

  erase<Component>(std::ranges::begin(r), std::ranges::end(r));
}

template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::try_erase(identifier_type const id)
noexcept(s_noexcept_try_erase<Component>())
{
  if constexpr (s_implements_try_erase<Component>())
    return m_storage.template try_erase<Component>(id);
  else
  {
    if (has<Component>(id))
    {
      erase<Component>(id);
      return true;
    }

    return false;
  }
}

template<typename Storage>
template<
    typename Component,
    typename Iterator,
    typename Sentinel>
constexpr
void
registry<Storage>
    ::try_erase(Iterator first, Sentinel last)
noexcept(s_noexcept_try_erase<Component, Iterator, Sentinel>())
{
  static_assert(
      std::input_iterator<Iterator>
   && std::convertible_to<std::iter_reference_t<Iterator>, identifier_type>,
      "heim::registry::try_erase: Iterator must be an input iterator dereferenceable to a type convertible "
      "to identifier_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "heim::registry::try_erase: Sentinel must be a sentinel for Iterator.");

  if constexpr (s_implements_try_erase<Component, Iterator, Sentinel>())
    m_storage.template try_erase<Component>(first, last);
  else
  {
    for (; first != last; ++first)
      try_erase<Component>(*first);
  }
}

template<typename Storage>
template<
    typename Component,
    typename Range>
constexpr
void
registry<Storage>
    ::try_erase(Range &&r)
noexcept(s_noexcept_try_erase<Component, Range>())
{
  try_erase<Component>(std::ranges::begin(r), std::ranges::end(r));
}

template<typename Storage>
constexpr
void
registry<Storage>
    ::clear(identifier_type const id)
noexcept(s_noexcept_clear_identifier())
{
  static constexpr
  bool
  implements_clear_identifier
  = requires
  {
    std::declval<storage_type &>().clear(std::declval<identifier_type>());
  };

  static_assert(
      implements_clear_identifier,
      "heim::registry::clear: storage_type must expose a clear(identifier_type) method.");

  m_storage.clear(id);
}

template<typename Storage>
constexpr
void
registry<Storage>
    ::clear()
noexcept(s_noexcept_clear())
{
  static constexpr
  bool
  implements_clear
  = requires { std::declval<storage_type &>().clear(); };

  static_assert(
      implements_clear,
      "heim::registry::clear: storage_type must expose a clear() method.");

  m_storage.clear();
  m_id_mgr .banish_all();
}


/*!
 * @brief Determines whether the given type is a specialization of registry.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_registry;

template<typename T>
struct specializes_registry
  : bool_constant<false>
{ };

template<typename Storage>
struct specializes_registry<
    registry<Storage>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
specializes_registry_v
= specializes_registry<T>::value;


} // namespace heim

#endif // HEIM_REGISTRY_HPP