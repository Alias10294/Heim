#ifndef HEIM_REGISTRY_HPP
#define HEIM_REGISTRY_HPP

#include <cstddef>
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

public:
  explicit constexpr
  registry(allocator_type const &)
  noexcept;

  constexpr
  registry()
  noexcept(std::is_nothrow_default_constructible_v<allocator_type>);

  constexpr
  registry(registry const &, allocator_type const &);

  constexpr
  registry(registry const &)
  = default;

  constexpr
  registry(registry &&, allocator_type const &)
  noexcept(
      std::is_nothrow_constructible_v<entity_manager_type, entity_manager_type &&, allocator_type const &>
   && std::is_nothrow_constructible_v<storage_type       , storage_type        &&, allocator_type const &>);

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


  constexpr
  void
  swap(registry &)
  noexcept(
      std::is_nothrow_swappable_v<entity_manager_type>
   && std::is_nothrow_swappable_v<storage_type       >);


  friend constexpr
  void
  swap(registry &lhs, registry &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
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
registry<Storage>
    ::registry(allocator_type const &alloc)
noexcept
  : m_entity_mgr(alloc),
    m_storage   (alloc)
{ }

template<typename Storage>
constexpr
registry<Storage>
    ::registry()
noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
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
noexcept(
    std::is_nothrow_constructible_v<entity_manager_type, entity_manager_type &&, allocator_type const &>
 && std::is_nothrow_constructible_v<storage_type       , storage_type        &&, allocator_type const &>)
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
void
registry<Storage>
    ::swap(registry &other)
noexcept(
    std::is_nothrow_swappable_v<entity_manager_type>
 && std::is_nothrow_swappable_v<storage_type       >)
{
  m_entity_mgr.swap(other.m_entity_mgr);
  m_storage   .swap(other.m_storage   );
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