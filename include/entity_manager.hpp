#ifndef HEIM_ENTITY_MANAGER_HPP
#define HEIM_ENTITY_MANAGER_HPP

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include "entity.hpp"

namespace heim
{
/*!
 * @brief The container used to manage and keep track of entities and their
 *   state.
 *
 * @tparam Entity The type of the entities.
 * @tparam Alloc  The type of allocator used in the container.
 *
 * @details Implements a stack of invalidated entities, that are reused
 *   whenever possible. To make entities reusable, each entity value contains
 *   its index and a generation, that is used to verify the current validity of
 *   the index.
 */
template<
    typename Entity,
    typename Alloc>
class entity_manager
{
  static_assert(
      is_entity_v<Entity>,
      "heim::entity_manager<Entity, Alloc>: "
          "is_entity_v<Entity>;");
  static_assert(
      std::is_same_v<
          typename std::allocator_traits<Alloc>::value_type,
          Entity>,
      "heim::entity_manager<Entity, Alloc>: "
          "std::is_same_v<"
              "typename std::allocator_traits<Alloc>::value_type, "
              "Entity>;");

public:
  using entity_type    = Entity;
  using allocator_type = Alloc;


  using size_type = std::size_t;

private:
  using entity_traits_t = entity_traits<entity_type>;

public:
  using index_type      = entity_traits_t::index_type;
  using generation_type = entity_traits_t::generation_type;

private:
  using alloc_traits_t
  = std::allocator_traits<Alloc>;

  using index_alloc_t
  = typename alloc_traits_t::template rebind_alloc<index_type>;

  using generation_alloc_t
  = typename alloc_traits_t::template rebind_alloc<generation_type>;


  using index_vector_t
  = std::vector<index_type, index_alloc_t>;

  using generation_vector_t
  = std::vector<generation_type, generation_alloc_t>;

private:
  [[no_unique_address]]
  allocator_type m_allocator;

  generation_vector_t m_generations;
  index_vector_t      m_stack;

private:
  //! @cond INTERNAL

  /*!
   * @brief Sets the capacity of the object to @code new_cap@endcode.
   *
   * @param new_cap The new capacity to reserve memory for.
   */
  constexpr void
  m_set_capacity(size_type const new_cap)
  {
    generation_vector_t generations{generation_alloc_t{m_allocator}};
    index_vector_t      stack      {index_alloc_t     {m_allocator}};

    generations.reserve(new_cap);
    stack      .reserve(new_cap);

    generations.assign(
        std::make_move_iterator(m_generations.begin()),
        std::make_move_iterator(m_generations.end  ()));
    stack      .assign(
        std::make_move_iterator(m_stack      .begin()),
        std::make_move_iterator(m_stack      .end  ()));

    using std::swap;

    swap(m_generations, generations);
    swap(m_stack      , stack      );
  }

  //! @endcond

public:
  constexpr
  entity_manager()
    : entity_manager{allocator_type{}}
  { }

  explicit constexpr
  entity_manager(allocator_type const &alloc)
    : m_allocator  {alloc},
      m_generations{generation_alloc_t{alloc}},
      m_stack      {index_alloc_t     {alloc}}
  { }

  constexpr
  entity_manager(entity_manager const &other)
    : m_allocator  {alloc_traits_t
          ::select_on_container_copy_construction(other.m_allocator)},
      m_generations{other.m_generations, generation_alloc_t{m_allocator}},
      m_stack      {other.m_stack      , index_alloc_t     {m_allocator}}
  { }

  constexpr
  entity_manager(entity_manager &&other)
  noexcept
  = default;

  constexpr
  entity_manager(
      entity_manager const &other,
      allocator_type const &alloc)
        : m_allocator  {alloc},
          m_generations{other.m_generations, generation_alloc_t{m_allocator}},
          m_stack      {other.m_stack      , index_alloc_t     {m_allocator}}
  { }

  constexpr
  entity_manager(
      entity_manager      &&other,
      allocator_type const &alloc)
  noexcept
    : m_allocator  {alloc},
      m_generations{
          std::move(other.m_generations),
          generation_alloc_t{m_allocator}},
      m_stack      {
          std::move(other.m_stack),
          index_alloc_t     {m_allocator}}
  { }

  constexpr
  ~entity_manager()
  noexcept
  = default;


  constexpr entity_manager &
  operator=(entity_manager const &other)
  {
    if (this == &other)
      return *this;

    if constexpr (alloc_traits_t::propagate_on_container_copy_assignment::value)
    {
      if constexpr (!alloc_traits_t::is_always_equal::value)
      {
        if (m_allocator != other.m_allocator)
        {
          entity_manager tmp{other, other.m_allocator};
          swap(tmp);
          return *this;
        }
      }
      m_allocator = other.m_allocator;
    }

    m_generations = other.m_generations;
    m_stack       = other.m_stack;

    return *this;
  }

  constexpr entity_manager &
  operator=(entity_manager &&other)
  noexcept
  {
    if (this == &other)
      return *this;

    m_generations = std::move(other.m_generations);
    m_stack       = std::move(other.m_stack      );

    if constexpr (
        alloc_traits_t::propagate_on_container_move_assignment::value
     || alloc_traits_t::is_always_equal::value)
      m_allocator = std::move(other.m_allocator);

    return *this;
  }


  /*!
   * @brief Swaps the contents of @c *this and @code other@endcode.
   *
   * @param other The other entity manager whose contents to swap.
   */
  constexpr void
  swap(entity_manager &other)
  noexcept(
      alloc_traits_t::propagate_on_container_swap::value
   || alloc_traits_t::is_always_equal::value)
  {
    using std::swap;

    if constexpr (alloc_traits_t::propagate_on_container_swap::value)
      swap(m_allocator, other.m_allocator);

    swap(m_generations, other.m_generations);
    swap(m_stack      , other.m_stack      );
  }

  /*!
   * @brief Swaps the contents of @code lhs @endcode and @code rhs@endcode.
   *
   * @param lhs The first  entity manager whose contents to swap.
   * @param rhs The second entity manager whose contents to swap.
   */
  friend constexpr void
  swap(entity_manager &lhs, entity_manager &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }


  //! @returns The entity manager's allocator.
  [[nodiscard]]
  constexpr allocator_type
  get_allocator() const
  noexcept
  {
    return m_allocator;
  }



  //! @returns The total number of managed entities, both valid and invalid.
  [[nodiscard]]
  constexpr size_type
  size() const
  noexcept
  {
    return m_generations.size();
  }


  //! @returns The maximum number of entities the entity manager can manage.
  [[nodiscard]]
  constexpr size_type
  max_size() const
  noexcept
  {
    return std::min(m_generations.max_size(), m_stack.max_size());
  }


  /*!
   * @returns The maximum number of entities the entity manager can manage
   *   without requiring reallocation.
   */
  [[nodiscard]]
  constexpr size_type
  capacity() const
  noexcept
  {
    return m_generations.capacity();
  }


  /*!
   * @brief Increases the capacity (the maximum number of entities the entity
   *   manager can manage without requiring reallocation) to a value that is at
   *   least equal to @code new_cap@endcode.
   *
   * @param new_cap The new capacity to attain of the entity manager.
   */
  constexpr void
  reserve(size_type const new_cap)
  {
    if (new_cap <= capacity())
      return;

    if (new_cap > max_size())
    {
      throw std::length_error{
        "heim::entity_manager<Entity, Alloc>::reserve(size_type const)"};
    }

    m_set_capacity(new_cap);
  }


  /*!
   * @brief Request the removal of unused capacity.
   *
   * @details It is a non-binding request to reduce the entity manager's
   *   capacity to its size. It depends on the implementation whether the
   *   request is fulfilled.
   */
  constexpr void
  shrink_to_fit()
  {
    m_set_capacity(size());
  }



  /*!
   * @brief Checks whether the entity @code e@endcode is valid in the context
   *   of managements of @c *this .
   *
   * @param e The entity to validate.
   * @returns @c true if the entity's generation exists and is the current,
   *   @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool
  valid(entity_type const e) const
  noexcept
  {
    index_type const idx = entity_traits_t::index(e);

    return idx < m_generations.size()
        && entity_traits_t::generation(e) == m_generations[idx];
  }


  /*!
   * @brief Returns the generation of the index @code idx@endcode.
   *
   * @param idx The index to get the generation of.
   * @returns The generation of @code idx@endcode.
   */
  [[nodiscard]]
  constexpr generation_type
  generation(index_type const idx) const
  noexcept
  {
    return m_generations[static_cast<size_type>(idx)];
  }



  /*!
   * @brief Generates a new valid entity.
   *
   * @returns A new valid entity.
   *
   * @details Either frees the stack of invalidated indexes with its generation
   *   updated, or generates a brand-new index value.
   */
  [[nodiscard]]
  constexpr entity_type
  generate()
  {
    index_type idx;

    if (m_stack.empty())
    {
      idx = static_cast<index_type>(m_generations.size());
      m_generations.push_back(0);
    }
    else
    {
      idx = m_stack.back();
      m_stack.pop_back();
    }

    return entity_traits_t::from(
        m_generations[static_cast<size_type>(idx)],
        idx);
  }


  /*!
   * @brief Invalidates the entity @code e@endcode 's generation and adds its
   *   index to the stack of invalidated entities.
   *
   * @param e The entity to invalidate.
   */
  constexpr void
  invalidate(entity_type const e)
  {
    if (!valid(e))
      return;

    index_type const idx = entity_traits_t::index(e);

    m_stack.push_back(idx);
    ++m_generations[idx];
  }

};


}


#endif // HEIM_ENTITY_MANAGER_HPP
