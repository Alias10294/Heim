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

  friend constexpr void
  swap(entity_manager &lhs, entity_manager &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }


  [[nodiscard]]
  constexpr allocator_type
  get_allocator() const
  noexcept
  {
    return m_allocator;
  }



  [[nodiscard]]
  constexpr size_type
  size() const
  noexcept
  {
    return m_generations.size();
  }


  [[nodiscard]]
  constexpr size_type
  max_size() const
  noexcept
  {
    return std::min(m_generations.max_size(), m_stack.max_size());
  }


  [[nodiscard]]
  constexpr size_type
  capacity() const
  noexcept
  {
    return m_generations.capacity();
  }


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


  constexpr void
  shrink_to_fit()
  {
    m_set_capacity(size());
  }



  [[nodiscard]]
  constexpr bool
  valid(entity_type const e) const
  noexcept
  {
    index_type const idx = entity_traits_t::index(e);

    return idx < m_generations.size()
        && entity_traits_t::generation(e) == m_generations[idx];
  }


  [[nodiscard]]
  constexpr generation_type
  generation(index_type const idx) const
  noexcept
  {
    return m_generations[static_cast<size_type>(idx)];
  }

  [[nodiscard]]
  constexpr generation_type
  generation(entity_type const e) const
  noexcept
  {
    size_type const i = static_cast<size_type>(entity_traits_t::index(e));

    return m_generations[i];
  }



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
