#ifndef HEIM_REGISTRY_HPP
#define HEIM_REGISTRY_HPP

#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "entity_manager.hpp"
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


  using storage_type = Storage;

  using entity_type    = typename storage_type::entity_type;
  using allocator_type = typename storage_type::allocator_type;

private:
  using entity_manager_type = entity_manager<entity_type, allocator_type>;

private:
  entity_manager_type m_entity_mgr;
  storage_type        m_storage;

private:
  static constexpr
  bool
  s_noexcept_alloc_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_destroy()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_has()
  noexcept;

  template<typename ...Components>
  static constexpr
  bool
  s_noexcept_has_all_of()
  noexcept;

  template<typename ...Components>
  static constexpr
  bool
  s_noexcept_has_any_of()
  noexcept;

  template<typename ...Components>
  static constexpr
  bool
  s_noexcept_has_none_of()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_get()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_get_const()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_get_if()
  noexcept;

  template<typename Component>
  static constexpr
  bool
  s_noexcept_get_if_const()
  noexcept;

  template<
      typename    Component,
      typename ...Args>
  static constexpr
  bool
  s_noexcept_emplace()
  noexcept;

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

  static constexpr
  bool
  s_noexcept_clear()
  noexcept;

  static constexpr
  bool
  s_noexcept_swap()
  noexcept;

public:
  explicit constexpr
  registry(allocator_type const &)
  noexcept(s_noexcept_alloc_construct());

  constexpr
  registry()
  noexcept(s_noexcept_default_construct());

  constexpr
  registry(registry const &, allocator_type const &);

  constexpr
  registry(registry const &)
  = default;

  constexpr
  registry(registry &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  registry(registry &&)
  = default;

  constexpr
  ~registry()
  = default;

  constexpr
  registry &
  operator=(registry const &)
  = default;

  constexpr
  registry &
  operator=(registry &&)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;


  [[nodiscard]] constexpr
  bool
  is_valid(entity_type const) const
  noexcept;

  [[nodiscard]] constexpr
  entity_type
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
  destroy(entity_type const)
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
  has(entity_type const) const
  noexcept(s_noexcept_has<Component>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_all_of(entity_type const) const
  noexcept(s_noexcept_has_all_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_any_of(entity_type const) const
  noexcept(s_noexcept_has_any_of<Components ...>());

  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  has_none_of(entity_type const) const
  noexcept(s_noexcept_has_none_of<Components ...>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(entity_type const)
  noexcept(s_noexcept_get<Component>());

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(entity_type const) const
  noexcept(s_noexcept_get_const<Component>());

  template<typename Component>
  constexpr
  Component &
  try_get(entity_type const);

  template<typename Component>
  constexpr
  Component const &
  try_get(entity_type const) const;

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(entity_type const)
  noexcept(s_noexcept_get_if<Component>());

  template<typename Component>
  constexpr
  Component const *
  get_if(entity_type const) const
  noexcept(s_noexcept_get_if_const<Component>());


  template<
      typename    Component,
      typename ...Args>
  constexpr
  decltype(auto)
  emplace(entity_type const, Args &&...)
  noexcept(s_noexcept_emplace<Component, Args ...>());

  template<typename Component>
  constexpr
  decltype(auto)
  erase(entity_type const)
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
  noexcept(s_noexcept_erase<Component, std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>());

  constexpr
  void
  clear()
  noexcept(s_noexcept_clear());


  constexpr
  void
  swap(registry &)
  noexcept(s_noexcept_swap());


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
          entity_manager_type,
          entity_manager_type &&, allocator_type const &>
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
  return noexcept(std::declval<storage_type &>()
      .erase_entity(std::declval<entity_type const>()));
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
      .template has<Component>(std::declval<entity_type const>()));
}


template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::s_noexcept_has_all_of()
noexcept
{
  static constexpr
  bool
  storage_implements_has_all_of
  = requires
  {
    std::declval<storage_type const &>()
        .template has_all_of<Components ...>(std::declval<entity_type const>());
  };

  if constexpr (storage_implements_has_all_of)
  {
    return noexcept(std::declval<storage_type const &>()
        .template has_all_of<Components ...>(std::declval<entity_type const>()));
  }

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
  static constexpr
  bool
  storage_implements_has_any_of
  = requires
  {
    std::declval<storage_type const &>()
        .template has_any_of<Components ...>(std::declval<entity_type const>());
  };

  if constexpr (storage_implements_has_any_of)
  {
    return noexcept(std::declval<storage_type const &>()
        .template has_any_of<Components ...>(std::declval<entity_type const>()));
  }

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
  static constexpr
  bool
  storage_implements_has_none_of
  = requires
  {
    std::declval<storage_type const &>()
        .template has_none_of<Components ...>(std::declval<entity_type const>());
  };

  if constexpr (storage_implements_has_none_of)
  {
    return noexcept(std::declval<storage_type const &>()
        .template has_none_of<Components ...>(std::declval<entity_type const>()));
  }

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
      .template get<Component>(std::declval<entity_type const>()));
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
      .template get<Component>(std::declval<entity_type const>()));
}


template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get_if()
noexcept
{
  static constexpr
  bool
  storage_implements_get_if
  = requires
  {
    std::declval<storage_type &>()
        .template get_if<Component>(std::declval<entity_type const>());
  };

  if constexpr (storage_implements_get_if)
  {
    return noexcept(std::declval<storage_type &>()
        .template get_if<Component>(std::declval<entity_type const>()));
  }

  return s_noexcept_has<Component>()
      && s_noexcept_get<Component>();
}


template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::s_noexcept_get_if_const()
noexcept
{
  static constexpr
  bool
  storage_implements_get_if_const
  = requires
  {
    std::declval<storage_type const &>()
        .template get_if<Component>(std::declval<entity_type const>());
  };

  if constexpr (storage_implements_get_if_const)
  {
    return noexcept(std::declval<storage_type const &>()
        .template get_if<Component>(std::declval<entity_type const>()));
  }

  return s_noexcept_has      <Component>()
      && s_noexcept_get_const<Component>();
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
      .template emplace<Component>(std::declval<entity_type const>(), std::declval<Args &&>()...));
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
      .template erase<Component>(std::declval<entity_type const>()));
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

  return s_noexcept_erase<Component>();
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
  return std::is_nothrow_swappable_v<entity_manager_type>
      && std::is_nothrow_swappable_v<storage_type       >;
}



template<typename Storage>
constexpr
registry<Storage>
    ::registry(allocator_type const &alloc)
noexcept(s_noexcept_alloc_construct())
  : m_entity_mgr(alloc),
    m_storage   (alloc)
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry()
noexcept(s_noexcept_default_construct())
  : registry(allocator_type())
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry(registry const &other, allocator_type const &alloc)
  : m_entity_mgr(other.m_entity_mgr, alloc),
    m_storage   (other.m_storage   , alloc)
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry(registry &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_entity_mgr(std::move(other.m_entity_mgr), alloc),
    m_storage   (std::move(other.m_storage   ), alloc)
{ }



template<typename Storage>
constexpr
typename registry<Storage>
    ::allocator_type
registry<Storage>
    ::get_allocator() const
noexcept
{
  return m_entity_mgr.get_allocator();
}



template<typename Storage>
constexpr
bool
registry<Storage>
    ::is_valid(entity_type const e) const
noexcept
{
  return m_entity_mgr.is_valid(e);
}


template<typename Storage>
constexpr
typename registry<Storage>
    ::entity_type
registry<Storage>
    ::create()
{
  entity_type const e = m_entity_mgr.summon();

  static constexpr
  bool
  implements_emplace_entity
  = requires
  {
    std::declval<storage_type &>().emplace_entity(std::declval<entity_type const>());
  };

  static constexpr
  bool
  noexcept_emplace_entity
  = noexcept(std::declval<storage_type &>().emplace_entity(std::declval<entity_type const>()));

  // depending on the storage type it might need to be introduced to the entity
  if constexpr (implements_emplace_entity)
  {
    if constexpr (noexcept_emplace_entity)
      m_storage.emplace_entity(e);
    else
    {
      // ensure "strong" (basic in reality but strong in this context) exception safety guarantee
      try
      { m_storage.emplace_entity(e); }
      catch (...)
      { m_entity_mgr.banish(e); throw; }
    }
  }

  return e;
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
      std::output_iterator<Iterator, entity_type>,
      "Iterator must be an output iterator for entity_type");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "Sentinel must be a sentinel for Iterator.");

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
      std::ranges::output_range<Range, entity_type>,
      "Range must be an output range for entity_type.");

  create(std::ranges::begin(r), std::ranges::end(r));
}


template<typename Storage>
constexpr
void
registry<Storage>
    ::destroy(entity_type const e)
noexcept(s_noexcept_destroy())
{
  static_assert(
      requires { std::declval<storage_type &>().erase_entity(std::declval<entity_type const>()); },
      "storage_type must expose an erase_entity method.");

  m_storage   .erase_entity(e);
  m_entity_mgr.banish(e);
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
   && std::convertible_to<std::iter_reference_t<Iterator>, entity_type>,
      "Iterator must be an input iterator dereferenceable to a type convertible to entity_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "Sentinel must be a sentinel for Iterator.");

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
   && std::convertible_to<std::ranges::range_reference_t<Range>, entity_type>,
      "Range must be an input range with an iterator dereferenceable to a type convertible to entity_type.");

  destroy(std::ranges::begin(r), std::ranges::end(r));
}



template<typename Storage>
template<typename Component>
constexpr
bool
registry<Storage>
    ::has(entity_type const e) const
noexcept(s_noexcept_has<Component>())
{
  static constexpr
  bool
  implements_has
  = requires
  {
    { std::declval<storage_type const &>()
        .template has<Component>(std::declval<entity_type const>()) }
        -> std::same_as<bool>;
  };

  static_assert(
      implements_has,
      "storage_type must expose a has method.");

  return m_storage.template has<Component>(e);
}


template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_all_of(entity_type const e) const
noexcept(s_noexcept_has_all_of<Components ...>())
{
  static constexpr
  bool
  implements_has_all_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_all_of<Components ...>(std::declval<entity_type const>()) }
        -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_all_of)
    return m_storage.template has_all_of<Components ...>(e);

  return (has<Components>(e) && ...);
}


template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_any_of(entity_type const e) const
noexcept(s_noexcept_has_any_of<Components ...>())
{
  static constexpr
  bool
  implements_has_any_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_any_of<Components ...>(std::declval<entity_type const>()) } -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_any_of)
    return m_storage.template has_any_of<Components ...>(e);

  return (has<Components>(e) || ...);
}


template<typename Storage>
template<typename ...Components>
constexpr
bool
registry<Storage>
    ::has_none_of(entity_type const e) const
noexcept(s_noexcept_has_none_of<Components ...>())
{
  static constexpr
  bool
  implements_has_none_of
  = requires
  {
    { std::declval<storage_type const &>()
        .template has_none_of<Components ...>(std::declval<entity_type const>()) } -> std::same_as<bool>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_has_none_of)
    return m_storage.template has_none_of<Components ...>(e);

  return (!has<Components>(e) && ...);
}


template<typename Storage>
template<typename Component>
constexpr
Component &
registry<Storage>
    ::get(entity_type const e)
noexcept(s_noexcept_get<Component>())
{
  return m_storage.template get<Component>(e);
}

template<typename Storage>
template<typename Component>
constexpr
Component const &
registry<Storage>
    ::get(entity_type const e) const
noexcept(s_noexcept_get_const<Component>())
{
  return m_storage.template get<Component>(e);
}


template<typename Storage>
template<typename Component>
constexpr
Component &
registry<Storage>
    ::try_get(entity_type const e)
{
  static constexpr
  bool
  implements_try_get
  = requires
  {
    { std::declval<storage_type &>()
        .template try_get<Component>(std::declval<entity_type const>()) } -> std::convertible_to<Component &>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_try_get)
    return m_storage.template try_get<Component>(e);

  if (has<Component>(e))
    return get<Component>(e);

  throw std::out_of_range("registry::try_get");
}

template<typename Storage>
template<typename Component>
constexpr
Component const &
registry<Storage>
    ::try_get(entity_type const e) const
{
  static constexpr
  bool
  implements_try_get_const
  = requires
  {
    { std::declval<storage_type const &>()
        .template try_get<Component>(std::declval<entity_type const>()) } -> std::convertible_to<Component const &>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_try_get_const)
    return m_storage.template try_get<Component>(e);

  if (has<Component>(e))
    return get<Component>(e);

  throw std::out_of_range("registry::try_get");
}


template<typename Storage>
template<typename Component>
constexpr
Component *
registry<Storage>
    ::get_if(entity_type const e)
noexcept(s_noexcept_get_if<Component>())
{
  static constexpr
  bool
  implements_get_if
  = requires
  {
    { std::declval<storage_type &>()
        .template get_if<Component>(std::declval<entity_type const>()) } -> std::convertible_to<Component *>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_get_if)
    return m_storage.template get_if<Component>(e);

  if (has<Component>(e))
    return std::addressof(get<Component>(e));

  return nullptr;
}

template<typename Storage>
template<typename Component>
constexpr
Component const *
registry<Storage>
    ::get_if(entity_type const e) const
noexcept(s_noexcept_get_if_const<Component>())
{
  static constexpr
  bool
  implements_get_if_const
  = requires
  {
    { std::declval<storage_type const &>()
        .template get_if<Component>(std::declval<entity_type const>()) } -> std::convertible_to<Component const *>;
  };

  // the storage can implement this method for specific behavior
  if constexpr (implements_get_if_const)
    return m_storage.template get_if<Component>(e);

  if (has<Component>(e))
    return std::addressof(get<Component>(e));

  return nullptr;
}



template<typename Storage>
template<
    typename    Component,
    typename ...Args>
constexpr
decltype(auto)
registry<Storage>
    ::emplace(entity_type const e, Args &&...args)
noexcept(s_noexcept_emplace<Component, Args ...>())
{
  static constexpr
  bool
  implements_emplace
  = requires
  {
    std::declval<storage_type &>()
        .template emplace<Component>(std::declval<entity_type const>(), std::declval<Args &&>()...);
  };

  static_assert(
      implements_emplace,
      "storage_type must expose an emplace method.");

  return m_storage.template emplace<Component>(e, std::forward<Args>(args)...);
}


template<typename Storage>
template<typename Component>
constexpr
decltype(auto)
registry<Storage>
    ::erase(entity_type const e)
noexcept(s_noexcept_erase<Component>())
{
  static constexpr
  bool
  implements_erase
  = requires
  {
    std::declval<storage_type &>().template erase<Component>(std::declval<entity_type const>());
  };

  static_assert(
      implements_erase,
      "storage_type must expose an erase method.");

  return m_storage.template erase<Component>(e);
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
   && std::convertible_to<std::iter_reference_t<Iterator>, entity_type>,
      "Iterator must be an input iterator dereferenceable to a type convertible to entity_type.");
  static_assert(
      std::sentinel_for<Sentinel, Iterator>,
      "Sentinel must be a sentinel for Iterator.");

  static constexpr
  bool
  implements_iterator_erase
  = requires
  {
    std::declval<storage_type &>()
        .template erase<Component, Iterator, Sentinel>(std::declval<Iterator>(), std::declval<Sentinel>());
  };

  if constexpr (implements_iterator_erase)
    return m_storage.template erase<Component, Iterator, Sentinel>(first, last);

  for (; first != last; ++first)
    erase(*first);
}

template<typename Storage>
template<
    typename Component,
    typename Range>
constexpr
void
registry<Storage>
    ::erase(Range &&r)
noexcept(s_noexcept_erase<Component, std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>())
{
  static_assert(
      std::ranges::input_range<Range>
   && std::convertible_to<std::ranges::range_reference_t<Range>, entity_type>,
      "Range must be an input range with an iterator dereferenceable to a type convertible to entity_type.");

  erase(std::ranges::begin(r), std::ranges::end(r));
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
      "storage_type must expose a clear method.");

  m_storage   .clear();
  m_entity_mgr.banish_all();
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
      "storage_type must be swappable.");

  swap(m_storage   , other.m_storage   );
  swap(m_entity_mgr, other.m_entity_mgr);
}



/*!
 * @brief Determines whether the given type is a specialization of registry.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_registry;

template<typename T>
inline constexpr
bool
specializes_registry_v
= specializes_registry<T>::value;

template<typename T>
struct specializes_registry
  : bool_constant<false>
{ };

template<typename Storage>
struct specializes_registry<
    registry<Storage>>
  : bool_constant<true>
{ };


} // namespace heim

#endif // HEIM_REGISTRY_HPP