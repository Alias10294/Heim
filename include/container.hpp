#ifndef HEIM_CONTAINER_HPP
#define HEIM_CONTAINER_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "allocator.hpp"
#include "component.hpp"
#include "entity.hpp"

namespace heim
{
/*!
 * @brief An associative container specialized for its use in this
 *   entity-component library, providing constant-time insertion, removal,
 *   access and search of elements, while also providing memory contiguity.
 *
 * @tparam Component The component type held.
 * @tparam Entity    The entity type held.
 * @tparam Allocator The allocator type for the container.
 * @tparam PageSize  The size of each page in the position container.
 * @tparam IsTag     The value marking Component as a tag or not.
 */
template<
    typename    Component,
    typename    Entity    = default_entity,
    typename    Allocator = default_allocator<Entity>,
    std::size_t PageSize  = container_page_size_for_v<Component>,
    bool        IsTag     = is_tag_for_v<Component>>
class container
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;


  using component_type = Component;
  using entity_type    = Entity;
  using allocator_type = Allocator;
  
  static constexpr size_type page_size = PageSize;
  static constexpr bool      is_tag    = IsTag;

  static_assert(is_entity_v       <entity_type>);
  static_assert(is_allocator_for_v<allocator_type, entity_type>);


  using value_type
  = std::conditional_t<
      is_tag,
      std::tuple<
          entity_type>,
      std::tuple<
          entity_type,
          component_type>>;

  using reference
  = std::conditional_t<
      is_tag,
      std::tuple<
          entity_type const &>,
      std::tuple<
          entity_type const &,
          component_type    &>>;

  using const_reference
  = std::conditional_t<
      is_tag,
      std::tuple<
          entity_type const &>,
      std::tuple<
          entity_type    const &,
          component_type const &>>;

private:
  class value_container;
  class position_container;

  template<bool IsConst>
  class generic_iterator;

public:
  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true >;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  value_container    m_values;
  position_container m_positions;

private:
  using alloc_traits = allocator_traits<allocator_type>;

public:
  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type(m_values.entities().get_allocator()); }


  [[nodiscard]] constexpr
  size_type
  size() const
  noexcept
  { return m_values.size(); }

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  { return std::min(m_values.max_size(), m_positions.max_size()); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return m_values.empty(); }


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept
  { return iterator(this, 0); }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator(this, 0); }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return iterator(this, size()); }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator(this, size()); }

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return begin(); }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return end(); }


  [[nodiscard]] constexpr
  reverse_iterator
  rbegin()
  noexcept
  { return reverse_iterator(end()); }

  [[nodiscard]] constexpr
  const_reverse_iterator
  rbegin() const
  noexcept
  { return const_reverse_iterator(end()); }

  [[nodiscard]] constexpr
  reverse_iterator
  rend()
  noexcept
  { return reverse_iterator(begin()); }

  [[nodiscard]] constexpr
  const_reverse_iterator
  rend() const
  noexcept
  { return const_reverse_iterator(begin()); }

  [[nodiscard]] constexpr
  const_reverse_iterator
  crbegin() const
  noexcept
  { return rbegin(); }

  [[nodiscard]] constexpr
  const_reverse_iterator
  crend() const
  noexcept
  { return rend(); }



  [[nodiscard]] constexpr
  bool
  contains(entity_type const e) const
  noexcept
  { return m_positions.contains(e); }

  [[nodiscard]] constexpr
  iterator
  find(entity_type const e)
  noexcept
  { return contains(e) ? iterator(this, m_positions[e]) : end(); }

  [[nodiscard]] constexpr
  const_iterator
  find(entity_type const e) const
  noexcept
  { return contains(e) ? const_iterator(this, m_positions[e]) : end(); }


  [[nodiscard]] constexpr
  component_type &
  operator[](entity_type const e)
  noexcept
  requires(!is_tag)
  { return m_values.components()[m_positions[e]]; }

  [[nodiscard]] constexpr
  component_type const &
  operator[](entity_type const e) const
  noexcept
  requires(!is_tag)
  { return m_values.components()[m_positions[e]]; }


  [[nodiscard]] constexpr
  component_type &
  at(entity_type const e)
  requires(!is_tag)
  {
    if (!contains(e)) throw std::out_of_range("container::at");
    return operator[](e);
  }

  [[nodiscard]] constexpr
  component_type const &
  at(entity_type const e) const
  requires(!is_tag)
  {
    if (!contains(e)) throw std::out_of_range("container::at");
    return operator[](e);
  }



  constexpr
  void
  clear()
  noexcept
  {
    m_values   .clear();
    m_positions.clear();
  }


  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  emplace(entity_type const e, Args &&...args)
  requires(
      is_tag
   || std::is_constructible_v<component_type, Args &&...>)
  {
    if (contains(e))
      return {iterator(this, m_positions[e]), false};

    m_positions.reserve_for(e);
    m_values.emplace_back(e, std::forward<Args>(args)...);
    m_positions[e] = size() - 1;
    return {--end(), true};
  }


  constexpr
  bool
  erase(entity_type const e)
  noexcept(is_tag || std::is_nothrow_move_assignable_v<component_type>)
  requires(is_tag || std::is_move_assignable_v        <component_type>)
  {
    if (!contains(e))
      return false;

    if (size_type &pos = m_positions[e]; pos != size() - 1)
    {
      m_values.overwrite_with_back(pos);
      m_positions[m_values.entities()[pos]] = pos;
    }

    m_values   .pop_back();
    m_positions.erase(e);
    return true;
  }



  constexpr
  void
  swap(container &other)
  noexcept(
      std::is_nothrow_swappable_v<position_container>
   && std::is_nothrow_swappable_v<value_container   >)
  {
    using std::swap;
    swap(m_positions, other.m_positions);
    swap(m_values   , other.m_values   );
  }

  friend constexpr
  void
  swap(container &lhs, container &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  { lhs.swap(rhs); }



  explicit constexpr
  container(allocator_type const &alloc)
    : m_values   (alloc),
      m_positions(alloc)
  { }

  constexpr
  container()
  noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
  requires(std::is_default_constructible_v        <allocator_type>)
    : container(allocator_type())
  { }

  constexpr
  container(container const &other, allocator_type const &alloc)
    : m_values   (other.m_values   , alloc),
      m_positions(other.m_positions, alloc)
  { }

  constexpr
  container(container const &other)
    : container(
          other,
          alloc_traits::select_on_container_copy_construction(other.get_allocator()))
  { }

  constexpr
  container(container &&other, allocator_type const &alloc)
  noexcept(
      std::is_nothrow_constructible_v<
          position_container,
          position_container &&,
          allocator_type const &>
   && std::is_nothrow_constructible_v<
          value_container,
          value_container &&,
          allocator_type const &>)
    : m_values   (std::move(other.m_values   ), alloc),
      m_positions(std::move(other.m_positions), alloc)
  { }

  constexpr
  container(container &&other)
  noexcept
  = default;


  constexpr
  ~container()
  noexcept
  = default;


  constexpr
  container &
  operator=(container const &other)
  {
    m_values    = other.m_values;
    m_positions = other.m_positions;
    return *this;
  }

  constexpr
  container &
  operator=(container &&other)
  noexcept(
      std::is_nothrow_move_assignable_v<position_container>
   && std::is_nothrow_move_assignable_v<value_container   >)
  {
    m_positions = std::move(other.m_positions);
    m_values    = std::move(other.m_values   );
    return *this;
  }



  [[nodiscard]] friend constexpr
  bool
  operator==(container const &lhs, container const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto const e : lhs.m_values.entities())
    {
      if (!rhs.contains(e)) return false;

      if constexpr (!is_tag)
      {
        if (lhs[e] != rhs[e])
          return false;
      }
    }
    return true;
  }
};



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        IsTag>
class container<Component, Entity, Allocator, PageSize, IsTag>
    ::value_container
{
private:
  using entity_allocator    = typename alloc_traits::template rebind_alloc<entity_type   >;
  using component_allocator = typename alloc_traits::template rebind_alloc<component_type>;

  using entity_alloc_traits    = allocator_traits<entity_allocator   >;
  using component_alloc_traits = allocator_traits<component_allocator>;

  using entity_vector    = std::vector<entity_type   , entity_allocator   >;
  using component_vector = std::vector<component_type, component_allocator>;

  using vector_tuple
  = std::conditional_t<
      is_tag,
      std::tuple<entity_vector>,
      std::tuple<entity_vector, component_vector>>;

private:
  vector_tuple m_vectors;

public:
  [[nodiscard]] constexpr
  entity_vector &
  entities()
  noexcept
  { return std::get<0>(m_vectors); }

  [[nodiscard]] constexpr
  entity_vector const &
  entities() const
  noexcept
  { return std::get<0>(m_vectors); }


  [[nodiscard]] constexpr
  component_vector &
  components()
  noexcept
  requires(!is_tag)
  { return std::get<1>(m_vectors); }

  [[nodiscard]] constexpr
  component_vector const &
  components() const
  noexcept
  requires(!is_tag)
  { return std::get<1>(m_vectors); }


  [[nodiscard]] constexpr
  size_type
  size() const
  noexcept
  { return entities().size(); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return entities().empty(); }

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  {
    if constexpr (is_tag)
      return entities().max_size();
    else
      return std::min(entities().max_size(), components().max_size());
  }



  constexpr
  reference
  operator[](size_type const idx)
  noexcept
  requires(is_tag)
  { return reference(entities()[idx]); }

  constexpr
  reference
  operator[](size_type const idx)
  noexcept
  requires(!is_tag)
  { return reference(entities()[idx], components()[idx]); }

  constexpr
  const_reference
  operator[](size_type const idx) const
  noexcept
  requires(is_tag)
  { return const_reference(entities()[idx]); }

  constexpr
  const_reference
  operator[](size_type const idx) const
  noexcept
  requires(!is_tag)
  { return const_reference(entities()[idx], components()[idx]); }



  constexpr
  void
  clear()
  noexcept
  {
    entities().clear();
    if constexpr (!is_tag)
      components().clear();
  }


  template<typename ...Args>
  constexpr
  void
  emplace_back(entity_type const e, Args&&... args)
  requires(
      is_tag
   || std::is_constructible_v<component_type, Args &&...>)
  {
    if constexpr (is_tag)
      entities().emplace_back(e);
    else
    {
      components().emplace_back(std::forward<Args>(args)...);
      // strong exception safety guarantee
      try
      { entities().emplace_back(e); }
      catch (...)
      { components().pop_back(); throw; }
    }
  }


  constexpr void
  overwrite_with_back(size_type const i)
  noexcept(is_tag || std::is_nothrow_move_assignable_v<component_type>)
  requires(is_tag || std::is_move_assignable_v        <component_type>)
  {
    if constexpr (!is_tag)
      components()[i] = std::move(components().back());
    entities()[i] = std::move(entities().back());
  }


  constexpr
  void
  pop_back()
  noexcept
  {
    entities().pop_back();
    if constexpr (!is_tag)
      components().pop_back();
  }



  constexpr
  void
  swap(value_container &other)
  noexcept(std::is_nothrow_swappable_v<vector_tuple>)
  {
    using std::swap;
    swap(m_vectors, other.m_vectors);
  }

  friend constexpr
  void
  swap(value_container &lhs, value_container &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  { lhs.swap(rhs); }



  explicit constexpr
  value_container(allocator_type const &alloc)
  noexcept
  requires(is_tag)
    : m_vectors(entity_vector(entity_allocator(alloc)))
  { }

  explicit constexpr
  value_container(allocator_type const &alloc)
  noexcept
  requires(!is_tag)
    : m_vectors(
          entity_vector   (entity_allocator   (alloc)),
          component_vector(component_allocator(alloc)))
  { }

  constexpr
  value_container()
  noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
  requires(std::is_default_constructible_v        <allocator_type>)
    : value_container(allocator_type())
  { }

  constexpr
  value_container(value_container const &other, allocator_type const &alloc)
  requires(is_tag)
    : m_vectors(entity_vector(other.entities(), entity_allocator(alloc)))
  { }

  constexpr
  value_container(value_container const &other, allocator_type const &alloc)
  requires(!is_tag)
    : m_vectors(
          entity_vector   (other.entities  (), entity_allocator   (alloc)),
          component_vector(other.components(), component_allocator(alloc)))
  { }

  constexpr
  value_container(value_container &&other, allocator_type const &alloc)
  noexcept(
      std::is_nothrow_constructible_v<
          entity_vector,
          entity_vector &&,
          entity_allocator const &>)
  requires(is_tag)
    : m_vectors(entity_vector(std::move(other.entities()), entity_allocator(alloc)))
  { }

  constexpr
  value_container(value_container &&other, allocator_type const &alloc)
  noexcept(
      std::is_nothrow_constructible_v<
          entity_vector,
          entity_vector &&,
          entity_allocator const &>
   && std::is_nothrow_constructible_v<
          component_vector,
          component_vector &&,
          component_allocator const &>)
  requires(!is_tag)
    : m_vectors(
          entity_vector   (std::move(other.entities  ()), entity_allocator   (alloc)),
          component_vector(std::move(other.components()), component_allocator(alloc)))
  { }

  constexpr
  value_container(value_container &&other)
  noexcept
  = default;


  constexpr
  ~value_container()
  noexcept
  = default;


  constexpr value_container &
  operator=(value_container const &other)
  {
    m_vectors = other.m_vectors;
    return *this;
  }

  constexpr value_container &
  operator=(value_container &&other)
  noexcept(std::is_nothrow_move_assignable_v<vector_tuple>)
  {
    m_vectors = std::move(other.m_vectors);
    return *this;
  }
};



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        IsTag>
class container<Component, Entity, Allocator, PageSize, IsTag>
    ::position_container
{
private:
  static constexpr bool      s_is_paged      = page_size != 0;
  static constexpr size_type s_null_position = std::numeric_limits<size_type>::max();


  class page_deleter;

  using page         = std::array<size_type, page_size>;
  using page_pointer = std::unique_ptr<page, page_deleter>;

  using page_allocator         = typename alloc_traits::template rebind_alloc<page        >;
  using page_pointer_allocator = typename alloc_traits::template rebind_alloc<page_pointer>;
  using size_allocator         = typename alloc_traits::template rebind_alloc<size_type   >;

  using page_alloc_traits         = allocator_traits<page_allocator        >;
  using page_pointer_alloc_traits = allocator_traits<page_pointer_allocator>;
  using size_alloc_traits         = allocator_traits<size_allocator        >;

  using vector
  = std::conditional_t<
      s_is_paged,
      std::vector<page_pointer, page_pointer_allocator>,
      std::vector<size_type   , size_allocator        >>;

private:
  vector m_vector;

private:
  static constexpr
  size_type
  s_page_index(typename entity_type::index_type const idx)
  noexcept
  requires(s_is_paged)
  {
    if constexpr (std::has_single_bit(page_size))
      return static_cast<size_type>(idx) >> std::countr_zero(page_size);
    else
      return static_cast<size_type>(idx) / page_size;
  }

  static constexpr
  size_type
  s_line_index(typename entity_type::index_type const idx)
  noexcept
  requires(s_is_paged)
  {
    if constexpr (std::has_single_bit(page_size))
      return static_cast<size_type>(idx) & (page_size - 1);
    else
      return static_cast<size_type>(idx) % page_size;
  }



  template<typename ...Args>
  [[nodiscard]] constexpr
  page_pointer
  m_make_page_pointer(Args &&...args)
  requires(
      s_is_paged
   && std::is_constructible_v<page, Args &&...>)
  {
    page_allocator alloc(m_vector.get_allocator());
    page          *p    (page_alloc_traits::allocate(alloc, 1));

    try
    { page_alloc_traits::construct(alloc, p, std::forward<Args>(args)...); }
    catch (...)
    { page_alloc_traits::deallocate(alloc, p, 1); throw; }

    return page_pointer(p, page_deleter(std::move(alloc)));
  }

  [[nodiscard]] constexpr
  page_pointer
  m_make_page_pointer(std::nullptr_t p)
  requires(s_is_paged)
  { return page_pointer(p, page_deleter(page_allocator(m_vector.get_allocator()))); }



  constexpr
  void
  m_copy_vector(vector const &from, vector &to)
  requires(s_is_paged)
  {
    to.reserve(from.capacity());
    for (auto const &p : from)
    {
      if (static_cast<bool>(p))
        to.emplace_back(m_make_page_pointer(*p));
      else
        to.emplace_back(m_make_page_pointer(nullptr));
    }
  }

public:
  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  requires(s_is_paged)
  {
    size_type const max       = m_vector.max_size();
    size_type const max_pages = std::numeric_limits<size_type>::max() / page_size;

    return max > max_pages
         ? max_pages * page_size
         : max       * page_size;
  }

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  requires(!s_is_paged)
  { return m_vector.max_size(); }



  [[nodiscard]] constexpr
  bool
  contains(entity_type const e) const
  noexcept
  requires(s_is_paged)
  {
    auto      const idx      = e.index();
    size_type const page_idx = s_page_index(idx);

    return page_idx < m_vector.size()
        && static_cast<bool>(m_vector[page_idx])
        && (*m_vector[page_idx])[s_line_index(idx)] != s_null_position;
  }

  [[nodiscard]] constexpr
  bool
  contains(entity_type const e) const
  noexcept
  requires(!s_is_paged)
  {
    size_type const i = static_cast<size_type>(e.index());

    return i < m_vector.size()
        && m_vector[i] != s_null_position;
  }


  [[nodiscard]] constexpr
  size_type &
  operator[](entity_type const e)
  noexcept
  {
    auto const idx = e.index();

    if constexpr (s_is_paged)
      return (*m_vector[s_page_index(idx)])[s_line_index(idx)];
    else
      return m_vector[static_cast<size_type>(idx)];
  }

  [[nodiscard]] constexpr
  size_type
  operator[](entity_type const e) const
  noexcept
  {
    auto const idx = e.index();

    if constexpr (s_is_paged)
      return (*m_vector[s_page_index(idx)])[s_line_index(idx)];
    else
      return m_vector[static_cast<size_type>(idx)];
  }



  constexpr
  void
  clear()
  noexcept
  requires(s_is_paged)
  {
    for (auto const &p : m_vector)
    {
      if (static_cast<bool>(p))
        std::fill(p->begin(), p->end(), s_null_position);
    }
  }

  constexpr
  void
  clear()
  noexcept
  requires(!s_is_paged)
  { std::fill(m_vector.begin(), m_vector.end(), s_null_position); }


  constexpr
  void
  reserve_for(entity_type const e)
  requires(s_is_paged)
  {
    auto      const idx      = e.index();
    size_type const page_idx = s_page_index(idx);

    if (page_idx >= m_vector.size())
      m_vector.reserve(page_idx + 1);

    while (page_idx >= m_vector.size())
      m_vector.emplace_back(m_make_page_pointer(nullptr));

    if (!static_cast<bool>(m_vector[page_idx]))
    {
      m_vector[page_idx] = m_make_page_pointer();
      m_vector[page_idx] ->fill(s_null_position);
    }
  }

  constexpr
  void
  reserve_for(entity_type const e)
  requires(!s_is_paged)
  {
    size_type const i = static_cast<size_type>(e.index());

    if (i >= m_vector.size())
      m_vector.resize(i + 1, s_null_position);
  }


  constexpr
  void
  erase(entity_type const e)
  noexcept
  { (*this)[e] = s_null_position; }



  constexpr
  void
  swap(position_container &other)
  noexcept
  {
    using std::swap;
    swap(m_vector, other.m_vector);
  }

  friend constexpr
  void
  swap(position_container &lhs, position_container &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  { lhs.swap(rhs); }



  explicit constexpr
  position_container(allocator_type const &alloc)
  noexcept
    : m_vector(typename vector::allocator_type(alloc))
  { }

  constexpr
  position_container(
      position_container const &other,
      allocator_type     const &alloc)
  requires(s_is_paged)
    : position_container(alloc)
  { m_copy_vector(other.m_vector, m_vector); }

  constexpr
  position_container(
      position_container const &other,
      allocator_type     const &alloc)
  requires(!s_is_paged)
    : m_vector(other.m_vector, typename vector::allocator_type(alloc))
  { }

  constexpr
  position_container(
      position_container  &&other,
      allocator_type const &alloc)
    : m_vector(std::move(other.m_vector), typename vector::allocator_type(alloc))
  { }

  constexpr
  position_container(position_container &&other)
  noexcept
  = default;


  constexpr
  ~position_container()
  noexcept
  = default;


  constexpr position_container &
  operator=(position_container const &other)
  requires(s_is_paged)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
    {
      auto const other_alloc = other.m_vector.get_allocator();
      std::destroy_at  (&m_vector);
      std::construct_at(&m_vector, other_alloc);
    }
    else
    { m_vector.clear(); }

    m_copy_vector(other.m_vector, m_vector);
    return *this;
  }

  constexpr position_container &
  operator=(position_container const &other)
  requires(!s_is_paged)
  {
    m_vector = other.m_vector;
    return *this;
  }

  constexpr position_container &
  operator=(position_container &&other)
  noexcept
  {
    m_vector = std::move(other.m_vector);
    return *this;
  }
};



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        IsTag>
class container<Component, Entity, Allocator, PageSize, IsTag>
    ::position_container
    ::page_deleter
{
private:
  [[no_unique_address]] page_allocator m_allocator;

public:
  constexpr
  void
  operator()(page *p)
  noexcept
  {
    page_alloc_traits::destroy   (m_allocator, p);
    page_alloc_traits::deallocate(m_allocator, p, 1);
  }



  explicit constexpr
  page_deleter(page_allocator &&alloc)
    : m_allocator(std::move(alloc))
  { }


  constexpr
  ~page_deleter()
  noexcept
  = default;
};



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        IsTag>
template<
    bool IsConst>
class container<Component, Entity, Allocator, PageSize, IsTag>
    ::generic_iterator
{
public:
  static constexpr bool is_const() noexcept { return IsConst; }
  static constexpr bool is_const_v = is_const();


  using difference_type = typename container::difference_type;

  using iterator_category = std::input_iterator_tag;
  using iterator_concept  = std::random_access_iterator_tag;

  using value_type
  = typename container::value_type;

  using reference
  = std::conditional_t<
      is_const_v,
      typename container::const_reference,
      typename container::reference>;

  struct pointer
  {
    reference r;

    constexpr
    reference const *
    operator->() const
    noexcept
    { return std::addressof(r); }
  };


  friend container;
  friend generic_iterator<!is_const_v>;

private:
  maybe_const_t<container, is_const_v> *m_container;
  difference_type                       m_index;

private:
  constexpr
  generic_iterator(
      maybe_const_t<container, is_const_v> *container,
      difference_type const                 index)
  noexcept
    : m_container(container),
      m_index    (index    )
  { }

public:
  constexpr
  generic_iterator &
  operator++()
  noexcept
  {
    ++m_index;
    return *this;
  }

  constexpr
  generic_iterator
  operator++(int)
  noexcept
  {
    generic_iterator tmp(*this);
    ++*this;
    return tmp;
  }


  constexpr
  generic_iterator &
  operator--()
  noexcept
  {
    --m_index;
    return *this;
  }

  constexpr
  generic_iterator
  operator--(int)
  noexcept
  {
    generic_iterator tmp(*this);
    --*this;
    return tmp;
  }


  constexpr
  generic_iterator &
  operator+=(difference_type const n)
  noexcept
  {
    m_index += n;
    return *this;
  }

  constexpr
  generic_iterator &
  operator-=(difference_type const n)
  noexcept
  {
    m_index -= n;
    return *this;
  }


  friend constexpr
  generic_iterator
  operator+(
      generic_iterator      it,
      difference_type const n)
  noexcept
  {
    it += n;
    return it;
  }

  friend constexpr
  generic_iterator
  operator+(
      difference_type const n,
      generic_iterator      it)
  noexcept
  {
    it += n;
    return it;
  }


  friend constexpr
  generic_iterator
  operator-(
      generic_iterator      it,
      difference_type const n)
  noexcept
  {
    it -= n;
    return it;
  }

  friend constexpr
  difference_type
  operator-(
      generic_iterator const &lhs,
      generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index - rhs.m_index;
  }



  constexpr
  reference
  operator*() const
  noexcept
  { return reference(m_container->m_values[m_index]); }


  constexpr
  pointer
  operator->() const
  noexcept
  { return pointer(operator*()); }

  constexpr
  reference
  operator[](difference_type const n) const
  noexcept
  { return *(*this + n); }



  constexpr
  generic_iterator()
  noexcept
  = default;

  constexpr
  generic_iterator(generic_iterator const &other)
  noexcept
  = default;

  constexpr
  generic_iterator(generic_iterator &&other)
  noexcept
  = default;

  explicit constexpr
  generic_iterator(generic_iterator<!is_const_v> it)
  noexcept
  requires(is_const_v)
    : m_container(it.m_container),
      m_index    (it.m_index    )
  { }



  [[nodiscard]] friend constexpr
  bool
  operator==(
      generic_iterator const &lhs,
      generic_iterator const &rhs)
  noexcept
  { return lhs.m_index == rhs.m_index; }


  [[nodiscard]] friend constexpr
  auto
  operator<=>(
      generic_iterator const &lhs,
      generic_iterator const &rhs)
  noexcept
  { return lhs.m_index <=> rhs.m_index; }
};



template<typename T>
struct is_container
  : bool_constant<false>
{ };

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        IsTag>
struct is_container<container<Component, Entity, Allocator, PageSize, IsTag>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
is_container_v
= is_container<T>::value();


} // namespace heim

#endif // HEIM_CONTAINER_HPP
