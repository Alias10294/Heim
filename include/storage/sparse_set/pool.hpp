#ifndef HEIM_SPARSE_SET_BASED_POOL_HPP
#define HEIM_SPARSE_SET_BASED_POOL_HPP

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
#include "entity.hpp"
#include "utility.hpp"

namespace heim::sparse_set_based
{
/*!
 * @brief Determines the default page size to use for pools of components when used by a storage.
 *
 * @tparam Redefine The type present for user specialization.
 *
 * @note The actual default value can be customized by specializing the trait using redefine_tag.
 */
template<typename Redefine = redefine_tag>
struct default_pool_page_size;

template<typename Redefine>
struct default_pool_page_size
  : size_constant<1024>
{ };



/*!
 * @brief An associative container optimized for usage in the context of the
 *   entity-component-system pattern.
 *
 * @details Implements a customized sparse set, that is each entity's position in the container is
 *   kept tracked by a complementary array, which is by default paginated to avoid significant
 *   memory overhead in most use cases. This structure allows for constant-time insertion, removal
 *   and access to elements, whilst containing values in contiguous memory for optimal iteration.
 *   Also, because of the structure-of-array (SoA) nature of the container, iterators expose a pair
 *   of references rather than a reference to a pair.
 *   Empty types, or types that are only considered valuable for their presence alongside an entity
 *   can be marked as tags. This option makes it so that they are not stored at all in the
 *   container nor exposed by iterators, which can constitute a significant memory saving.
 *
 * @tparam Component The component type.
 * @tparam Entity    The entity type.
 * @tparam Allocator The allocator type.
 * @tparam PageSize  The size of the internal pages of tracked positions.
 * @tparam TagValue  The value by which the component type is considered a tag type or not.
 *
 * @note Specializing the page size to be 0 causes the position container to not be paginated at
 *   all. This can be considered a good option if inserted entities are expected to have low enough
 *   index values.
 */
template<
    typename    Component,
    typename    Entity    = entity<>,
    typename    Allocator = std::allocator<Entity>,
    std::size_t PageSize  = default_pool_page_size<>::value,
    bool        TagValue  = std::is_empty_v<Component>>
class pool;

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
class pool
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;


  using component_type = Component;
  using entity_type    = Entity;
  using allocator_type = Allocator;

  static constexpr size_type page_size = PageSize;
  static constexpr bool      tag_value = TagValue;

  static_assert(
      specializes_entity_v<entity_type>,
      "entity_type must be a specialization of entity.");
  static_assert(
      is_an_allocator_for_v<allocator_type, entity_type>,
      "allocator_type must pass as an allocator of entity_type.");

private:
  using alloc_traits = std::allocator_traits<allocator_type>;

private:
  class value_container
  {
  public:
    using value_type
    = std::conditional_t<
        tag_value,
        std::tuple<entity_type>,
        std::tuple<entity_type, component_type>>;

    using reference
    = std::conditional_t<
        tag_value,
        std::tuple<entity_type const &>,
        std::tuple<entity_type const &, component_type &>>;

    using const_reference
    = std::conditional_t<
        tag_value,
        std::tuple<entity_type const &>,
        std::tuple<entity_type const &, component_type const &>>;

  private:
    using entity_allocator    = typename alloc_traits::template rebind_alloc<entity_type   >;
    using component_allocator = typename alloc_traits::template rebind_alloc<component_type>;

    using entity_alloc_traits    = std::allocator_traits<entity_allocator   >;
    using component_alloc_traits = std::allocator_traits<component_allocator>;

    using entity_vector    = std::vector<entity_type   , entity_allocator   >;
    using component_vector = std::vector<component_type, component_allocator>;

    using vector_tuple
    = std::conditional_t<
        tag_value,
        std::tuple<entity_vector>,
        std::tuple<entity_vector, component_vector>>;

  private:
    vector_tuple
    m_vectors;

  private:
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
    s_noexcept_clear()
    noexcept;

    static constexpr
    bool
    s_noexcept_assign_copy()
    noexcept;

    static constexpr
    bool
    s_noexcept_assign_move()
    noexcept;

    static constexpr
    bool
    s_noexcept_overwrite_with_back()
    noexcept;

    static constexpr
    bool
    s_noexcept_pop_back()
    noexcept;

    static constexpr
    bool
    s_noexcept_swap()
    noexcept;

    static constexpr
    bool
    s_noexcept_swap_positions()
    noexcept;

  public:
    explicit constexpr
    value_container(allocator_type const &)
    noexcept;

    constexpr
    value_container()
    noexcept(s_noexcept_default_construct());

    constexpr
    value_container(value_container const &, allocator_type const &);

    constexpr
    value_container(value_container const &)
    = default;

    constexpr
    value_container(value_container &&, allocator_type const &)
    noexcept(s_noexcept_move_alloc_construct());

    constexpr
    value_container(value_container &&)
    = default;

    constexpr
    ~value_container()
    = default;

    constexpr value_container &
    operator=(value_container const &)
    = default;

    constexpr value_container &
    operator=(value_container &&)
    = default;

    [[nodiscard]] constexpr
    allocator_type
    get_allocator() const
    noexcept;


    [[nodiscard]] constexpr
    entity_vector &
    entities()
    noexcept;

    [[nodiscard]] constexpr
    entity_vector const &
    entities() const
    noexcept;


    [[nodiscard]] constexpr
    component_vector &
    components()
    noexcept;

    [[nodiscard]] constexpr
    component_vector const &
    components() const
    noexcept;


    [[nodiscard]] constexpr
    size_type
    size() const
    noexcept;

    [[nodiscard]] constexpr
    bool
    empty() const
    noexcept;

    [[nodiscard]] constexpr
    size_type
    max_size() const
    noexcept;


    [[nodiscard]] constexpr
    reference
    operator[](size_type const)
    noexcept;

    [[nodiscard]] constexpr
    const_reference
    operator[](size_type const) const
    noexcept;



    constexpr
    void
    clear()
    noexcept(s_noexcept_clear());

    template<typename ...Args>
    constexpr
    void
    emplace_back(entity_type const, Args &&...);

    constexpr
    void
    assign(size_type const, component_type const &)
    noexcept(s_noexcept_assign_copy());

    constexpr
    void
    assign(size_type const, component_type &&)
    noexcept(s_noexcept_assign_move());

    constexpr
    void
    overwrite_with_back(size_type const)
    noexcept(s_noexcept_overwrite_with_back());

    constexpr
    void
    pop_back()
    noexcept(s_noexcept_pop_back());


    constexpr
    void
    swap(value_container &)
    noexcept(s_noexcept_swap());

    constexpr
    void
    swap(size_type const, size_type const)
    noexcept(s_noexcept_swap_positions());


    friend constexpr
    void
    swap(value_container &lhs, value_container &rhs)
    noexcept(s_noexcept_swap())
    {
      lhs.swap(rhs);
    }
  };


  class position_container
  {
  private:
    static constexpr bool      s_is_paged      = page_size != 0;
    static constexpr size_type s_null_position = std::numeric_limits<size_type>::max();

    using size_allocator    = typename alloc_traits::template rebind_alloc<size_type>;
    using size_alloc_traits = std::allocator_traits<size_allocator>;

    using page              = std::array<size_type, page_size>;
    using page_allocator    = typename alloc_traits::template rebind_alloc<page>;
    using page_alloc_traits = std::allocator_traits<page_allocator>;

    class page_deleter
    {
    private:
      [[no_unique_address]]
      page_allocator
      m_allocator;

    public:
      explicit constexpr
      page_deleter(allocator_type &&)
      noexcept;

      constexpr
      ~page_deleter()
      = default;


      constexpr
      void
      operator()(page *)
      noexcept;
    };

    using page_pointer              = std::unique_ptr<page, page_deleter>;
    using page_pointer_allocator    = typename alloc_traits::template rebind_alloc<page_pointer>;
    using page_pointer_alloc_traits = std::allocator_traits<page_pointer_allocator>;

    using vector_type
    = std::conditional_t<
        s_is_paged,
        std::vector<page_pointer, page_pointer_allocator>,
        std::vector<size_type   , size_allocator        >>;

  private:
    vector_type
    m_vector;

  private:
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
    s_noexcept_swap()
    noexcept;


    static constexpr
    size_type
    s_page_index(typename entity_type::index_type const)
    noexcept;

    static constexpr
    size_type
    s_line_index(typename entity_type::index_type const)
    noexcept;


    constexpr
    position_container(position_container const &, allocator_type const &, bool_constant<true>);

    constexpr
    position_container(position_container const &, allocator_type const &, bool_constant<false>);

    constexpr
    position_container(position_container const &, bool_constant<true>);

    constexpr
    position_container(position_container const &, bool_constant<false>);


    template<typename ...Args>
    [[nodiscard]] constexpr
    page_pointer
    m_make_page_pointer(Args &&...);

    [[nodiscard]] constexpr
    page_pointer
    m_make_page_pointer(std::nullptr_t);


    constexpr
    void
    m_copy_vector(vector_type const &, vector_type &);


    [[nodiscard]] constexpr
    allocator_type
    m_get_allocator() const
    noexcept;

  public:
    explicit constexpr
    position_container(allocator_type const &)
    noexcept;

    constexpr
    position_container()
    noexcept(s_noexcept_default_construct());

    constexpr
    position_container(position_container const &, allocator_type const &);

    constexpr
    position_container(position_container const &);

    constexpr
    position_container(position_container &&, allocator_type const &)
    noexcept(s_noexcept_move_alloc_construct());

    constexpr
    position_container(position_container &&)
    = default;

    constexpr
    ~position_container()
    = default;

    constexpr position_container &
    operator=(position_container const &);

    constexpr position_container &
    operator=(position_container &&)
    = default;


    [[nodiscard]] constexpr
    size_type
    max_size() const
    noexcept;

    [[nodiscard]] constexpr
    bool
    contains(entity_type const) const
    noexcept;

    [[nodiscard]] constexpr
    size_type &
    operator[](entity_type const)
    noexcept;

    [[nodiscard]] constexpr
    size_type
    operator[](entity_type const) const
    noexcept;


    constexpr
    void
    clear()
    noexcept;

    constexpr
    void
    reserve_for(entity_type const);

    constexpr
    void
    erase(entity_type const)
    noexcept;


    constexpr
    void
    swap(position_container &)
    noexcept(s_noexcept_swap());

    constexpr
    void
    swap(entity_type const, entity_type const)
    noexcept;


    friend constexpr
    void
    swap(position_container &lhs, position_container &rhs)
    noexcept(s_noexcept_swap())
    {
      lhs.swap(rhs);
    }
  };


  template<bool IsConst>
  class generic_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;

    static constexpr bool is_const = IsConst;

    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::random_access_iterator_tag;

    using value_type = typename value_container::value_type;

    using reference
    = std::conditional_t<
        is_const,
        typename value_container::const_reference,
        typename value_container::reference>;

    struct pointer
    {
    private:
      reference m_ref;

    public:
      explicit constexpr
      pointer(reference &&)
      noexcept;


      constexpr
      reference *
      operator->() const
      noexcept;
    };


    friend pool;
    friend generic_iterator<!is_const>;

  private:
    maybe_const_t<pool, is_const> *m_pool;
    difference_type                m_index;

  private:
    constexpr
    generic_iterator(maybe_const_t<pool, is_const> * const, difference_type const)
    noexcept;

  public:
    constexpr
    generic_iterator()
    = default;

    constexpr
    generic_iterator(generic_iterator const &)
    = default;

    constexpr
    generic_iterator(generic_iterator &&)
    = default;

    explicit constexpr
    generic_iterator(generic_iterator<!is_const>)
    noexcept;


    constexpr
    generic_iterator &
    operator++()
    noexcept;

    constexpr
    generic_iterator
    operator++(int)
    noexcept;

    constexpr
    generic_iterator &
    operator--()
    noexcept;

    constexpr
    generic_iterator
    operator--(int)
    noexcept;


    constexpr
    generic_iterator &
    operator+=(difference_type const)
    noexcept;

    constexpr
    generic_iterator &
    operator-=(difference_type const)
    noexcept;


    constexpr
    reference
    operator*() const
    noexcept;

    constexpr
    pointer
    operator->() const
    noexcept;

    constexpr
    reference
    operator[](difference_type const n) const
    noexcept;


    friend constexpr
    generic_iterator
    operator+(generic_iterator it, difference_type const n)
    noexcept
    {
      it += n;
      return it;
    }

    friend constexpr
    generic_iterator
    operator+(difference_type const n, generic_iterator it)
    noexcept
    {
      it += n;
      return it;
    }

    friend constexpr
    generic_iterator
    operator-(generic_iterator it, difference_type const n)
    noexcept
    {
      it -= n;
      return it;
    }

    friend constexpr
    difference_type
    operator-(generic_iterator const &lhs, generic_iterator const &rhs)
    noexcept
    {
      return lhs.m_index - rhs.m_index;
    }

    [[nodiscard]] friend constexpr
    bool
    operator==(generic_iterator const &lhs, generic_iterator const &rhs)
    noexcept
    {
      return lhs.m_index == rhs.m_index;
    }

    [[nodiscard]] friend constexpr
    decltype(auto)
    operator<=>(generic_iterator const &lhs, generic_iterator const &rhs)
    noexcept
    {
      return lhs.m_index <=> rhs.m_index;
    }
  };

public:
  using value_type      = typename value_container::value_type;
  using reference       = typename value_container::reference;
  using const_reference = typename value_container::const_reference;

  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true >;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  value_container    m_values;
  position_container m_positions;

private:
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
  s_noexcept_clear()
  noexcept;

  static constexpr
  bool
  s_noexcept_erase()
  noexcept;

  static constexpr
  bool
  s_noexcept_swap()
  noexcept;

  static constexpr
  bool
  s_noexcept_swap_entities()
  noexcept;


  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  m_emplace(entity_type const, Args &&...);

public:
  explicit constexpr
  pool(allocator_type const &)
  noexcept;

  constexpr
  pool()
  noexcept(s_noexcept_default_construct());

  constexpr
  pool(pool const &, allocator_type const &);

  constexpr
  pool(pool const &)
  = default;

  constexpr
  pool(pool &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  pool(pool &&)
  = default;

  constexpr
  ~pool()
  = default;

  constexpr pool &
  operator=(pool const &)
  = default;

  constexpr pool &
  operator=(pool &&)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;


  [[nodiscard]] constexpr
  size_type
  size() const
  noexcept;

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept;

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept;


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept;

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept;


  [[nodiscard]] constexpr
  reverse_iterator
  rbegin()
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  rbegin() const
  noexcept;

  [[nodiscard]] constexpr
  reverse_iterator
  rend()
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  rend() const
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  crbegin() const
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  crend() const
  noexcept;

  [[nodiscard]] constexpr
  iterator
  iterate(entity_type const)
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  iterate(entity_type const) const
  noexcept;


  [[nodiscard]] constexpr
  bool
  contains(entity_type const) const
  noexcept;

  [[nodiscard]] constexpr
  iterator
  find(entity_type const)
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  find(entity_type const) const
  noexcept;


  [[nodiscard]] constexpr
  component_type &
  operator[](entity_type const)
  noexcept;

  [[nodiscard]] constexpr
  component_type const &
  operator[](entity_type const) const
  noexcept;

  [[nodiscard]] constexpr
  component_type &
  at(entity_type const);

  [[nodiscard]] constexpr
  component_type const &
  at(entity_type const) const;


  constexpr
  void
  clear()
  noexcept(s_noexcept_clear());

  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  emplace(entity_type const, Args &&...);

  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  try_emplace(entity_type const, Args &&...);

  constexpr
  std::pair<iterator, bool>
  insert(entity_type const, component_type const &);

  constexpr
  std::pair<iterator, bool>
  insert(entity_type const, component_type &&);

  template<typename C>
  constexpr
  std::pair<iterator, bool>
  insert(entity_type const, C &&);

  constexpr
  std::pair<iterator, bool>
  insert_or_assign(entity_type const, component_type const &);

  constexpr
  std::pair<iterator, bool>
  insert_or_assign(entity_type const, component_type &&);


  constexpr
  bool
  erase(entity_type const)
  noexcept(s_noexcept_erase());


  constexpr
  void
  swap(pool &)
  noexcept(s_noexcept_swap());

  constexpr
  void
  swap(entity_type const, entity_type const)
  noexcept(s_noexcept_swap_entities());


  friend constexpr
  void
  swap(pool &lhs, pool &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(
      pool const &lhs,
      pool const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto const e : lhs.m_values.entities())
    {
      if (!rhs.contains(e))
        return false;

      if constexpr (!tag_value)
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
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<
      vector_tuple,
      std::allocator_arg_t, allocator_type const &, vector_tuple &&>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_clear()
noexcept
{
  return (tag_value || noexcept(std::declval<component_vector &>().clear()))
      && noexcept(std::declval<entity_vector &>().clear());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_assign_copy()
noexcept
{
  return tag_value || std::is_nothrow_copy_assignable_v<component_type>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_assign_move()
noexcept
{
  return tag_value || std::is_nothrow_move_assignable_v<component_type>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_overwrite_with_back()
noexcept
{
  return tag_value || std::is_nothrow_move_assignable_v<component_type>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_pop_back()
noexcept
{
  return (tag_value || noexcept(std::declval<component_vector &>().pop_back()))
      && noexcept(std::declval<entity_vector &>().pop_back());
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<vector_tuple>;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::s_noexcept_swap_positions()
noexcept
{
  return std::is_nothrow_swappable_v<component_type>;
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::value_container(allocator_type const &alloc)
noexcept
  : m_vectors(std::allocator_arg, alloc)
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::value_container()
noexcept(s_noexcept_default_construct())
  : m_vectors(std::allocator_arg, allocator_type())
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::value_container(
        value_container const &other,
        allocator_type  const &alloc)
  : m_vectors(std::allocator_arg, alloc, other.m_vectors)
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::value_container(value_container &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_vectors(std::allocator_arg, alloc, std::move(other.m_vectors))
{ }


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::allocator_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::get_allocator() const
noexcept
{
  return entities().get_allocator();
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::entity_vector &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::entities()
noexcept
{
  return std::get<0>(m_vectors);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::entity_vector const &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::entities() const
noexcept
{
  return std::get<0>(m_vectors);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::component_vector &
pool<Component, Entity, Allocator,PageSize, TagValue>
    ::value_container
    ::components()
noexcept
{
  static_assert(!tag_value, "tag_value must be false.");

  return std::get<1>(m_vectors);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::component_vector const &
pool<Component, Entity, Allocator,PageSize, TagValue>
    ::value_container
    ::components() const
noexcept
{
  static_assert(!tag_value, "tag_value must be false.");

  return std::get<1>(m_vectors);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::size() const
noexcept
{
  return entities().size();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::empty() const
noexcept
{
  return entities().empty();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::max_size() const
noexcept
{
  if constexpr (tag_value)
    return entities().max_size();
  else
    return std::min(entities().max_size(), components().max_size());
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::reference
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::operator[](size_type const pos)
noexcept
{
  if constexpr (tag_value)
    return reference(entities()[pos]);
  else
    return reference(entities()[pos], components()[pos]);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::const_reference
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::operator[](size_type const pos) const
noexcept
{
  if constexpr (tag_value)
    return const_reference(entities()[pos]);
  else
    return const_reference(entities()[pos], components()[pos]);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::clear()
noexcept(s_noexcept_clear())
{
  if constexpr (!tag_value)
    components().clear();

  entities().clear();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<
    typename ...Args>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::emplace_back(
        entity_type const e,
        Args &&...        args)
{
  if constexpr (tag_value)
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


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::assign(size_type const pos, component_type const &c)
noexcept(s_noexcept_assign_copy())
{
  if constexpr (!tag_value)
    components()[pos] = c;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::assign(size_type const pos, component_type &&c)
noexcept(s_noexcept_assign_move())
{
  if constexpr (!tag_value)
    components()[pos] = std::move(c);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::overwrite_with_back(size_type const pos)
noexcept(s_noexcept_overwrite_with_back())
{
  if constexpr (!tag_value)
    components()[pos] = std::move(components().back());

  entities()[pos] = std::move(entities().back());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::pop_back()
noexcept(s_noexcept_pop_back())
{
  if constexpr (!tag_value)
    components().pop_back();
  entities().pop_back();
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::swap(value_container &other)
noexcept(s_noexcept_swap())
{
  std::swap(m_vectors, other.m_vectors);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::value_container
    ::swap(size_type const pos_i, size_type const pos_j)
noexcept(s_noexcept_swap_positions())
{
  using std::swap;

  if constexpr (!tag_value)
    swap(components()[pos_i], components()[pos_j]);

  swap(entities()[pos_i], entities()[pos_j]);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::page_deleter
    ::page_deleter(allocator_type &&alloc)
noexcept
  : m_allocator(std::move(alloc))
{ }



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::page_deleter
    ::operator()(page *p)
noexcept
{
  page_alloc_traits::destroy   (m_allocator, p);
  page_alloc_traits::deallocate(m_allocator, p, 1);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<
        vector_type,
        vector_type &&, typename vector_type::allocator_type const &>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<vector_type>;
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::s_page_index(typename entity_type::index_type const idx)
noexcept
{
  // optimized for page_size as a power of 2
  if constexpr (std::has_single_bit(page_size))
    return static_cast<size_type>(idx) >> std::countr_zero(page_size);
  else
    return static_cast<size_type>(idx) / page_size;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::s_line_index(typename entity_type::index_type const idx)
noexcept
{
  // optimized for page_size as a power of 2
  if constexpr (std::has_single_bit(page_size))
    return static_cast<size_type>(idx) & (page_size - 1);
  else
    return static_cast<size_type>(idx) % page_size;
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(
        position_container const &other,
        allocator_type     const &alloc,
        bool_constant<true>)
  : position_container(alloc)
{
  m_copy_vector(other.m_vector, m_vector);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(
        position_container const &other,
        allocator_type     const &alloc,
        bool_constant<false>)
  : m_vector(other.m_vector, typename vector_type::allocator_type(alloc))
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(
        position_container const &other,
        bool_constant<true>)
  : position_container(alloc_traits::select_on_container_copy_construction(other.m_get_allocator()))
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(
        position_container const &other,
        bool_constant<false>)
  : m_vector(other.m_vector)
{ }



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<
    typename ...Args>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::page_pointer
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::m_make_page_pointer(Args &&...args)
{
  page_allocator alloc(m_vector.get_allocator());
  page          *p    (page_alloc_traits::allocate(alloc, 1));

  // exception safety guarantee
  try
  { page_alloc_traits::construct(alloc, p, std::forward<Args>(args)...); }
  catch (...)
  { page_alloc_traits::deallocate(alloc, p, 1); throw; }

  return page_pointer(p, page_deleter(std::move(alloc)));
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::page_pointer
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::m_make_page_pointer(std::nullptr_t p)
{
  return page_pointer(p, page_deleter(page_allocator(m_vector.get_allocator())));
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::m_copy_vector(vector_type const &from, vector_type &into)
{
  into.reserve(from.capacity());
  for (page_pointer const &p : from)
  {
    if (static_cast<bool>(p))
      into.emplace_back(m_make_page_pointer(*p));
    else
      into.emplace_back(m_make_page_pointer(nullptr));
  }
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::allocator_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::m_get_allocator() const
noexcept
{
  return m_vector.get_allocator();
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(allocator_type const &alloc)
noexcept
  : m_vector(typename vector_type::allocator_type(alloc))
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container()
noexcept(s_noexcept_default_construct())
  : position_container(allocator_type())
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(position_container const &other, allocator_type const &alloc)
  : position_container(other, alloc, bool_constant<s_is_paged>())
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(position_container const &other)
  : position_container(other, bool_constant<s_is_paged>())
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::position_container(position_container && other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_vector(std::move(other.m_vector), alloc)
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::operator=(position_container const &other)
{
  if constexpr (s_is_paged)
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
  }
  else
    m_vector = other.m_vector;

  return *this;
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::max_size() const
noexcept
{
  if constexpr (s_is_paged)
  {
    size_type const max      (m_vector.max_size());
    size_type const max_pages(std::numeric_limits<size_type>::max() / page_size);

    return max > max_pages
         ? max_pages * page_size
         : max       * page_size;
  }
  else
  {
    return m_vector.max_size();
  }
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::contains(entity_type const e) const
noexcept
{
  if constexpr (s_is_paged)
  {
    auto      const idx      = e.index();
    size_type const page_idx = s_page_index(idx);

    return page_idx < m_vector.size()
        && static_cast<bool>(m_vector[page_idx])
        && (*m_vector[page_idx])[s_line_index(idx)] != s_null_position;
  }
  else
  {
    size_type const i = static_cast<size_type>(e.index());

    return i < m_vector.size()
        && m_vector[i] != s_null_position;
  }
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::operator[](entity_type const e)
noexcept
{
  auto const idx = e.index();

  if constexpr (s_is_paged)
    return (*m_vector[s_page_index(idx)])[s_line_index(idx)];
  else
    return m_vector[static_cast<size_type>(idx)];
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::operator[](entity_type const e) const
noexcept
{
  auto const idx = e.index();

  if constexpr (s_is_paged)
    return (*m_vector[s_page_index(idx)])[s_line_index(idx)];
  else
    return m_vector[static_cast<size_type>(idx)];
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::clear()
noexcept
{
  if constexpr (s_is_paged)
  {
    for (auto const &p : m_vector)
    {
      if (static_cast<bool>(p))
        std::fill(p->begin(), p->end(), s_null_position);
    }
  }
  else
    std::fill(m_vector.begin(), m_vector.end(), s_null_position);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::reserve_for(entity_type const e)
{
  if constexpr (s_is_paged)
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
  else
  {
    size_type const i = static_cast<size_type>(e.index());

    if (i >= m_vector.size())
      m_vector.resize(i + 1, s_null_position);
  }
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::erase(entity_type const e)
noexcept
{
  (*this)[e] = s_null_position;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::swap(position_container &other)
noexcept(s_noexcept_swap())
{
  std::swap(m_vector, other.m_vector);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::position_container
    ::swap(
        entity_type const e,
        entity_type const f)
noexcept
{
  using std::swap;
  swap((*this)[e], (*this)[f]);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::pointer
    ::pointer(reference &&ref)
noexcept
  : m_ref(std::move(ref))
{ }



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
    ::reference *
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::pointer
    ::operator->() const
noexcept
{
  return std::addressof(m_ref);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::generic_iterator(maybe_const_t<pool, is_const> * const pool, difference_type const index)
noexcept
  : m_pool (pool),
    m_index(index)
{ }



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::generic_iterator(generic_iterator<!is_const> it)
noexcept
  : m_pool (it.m_container),
    m_index(it.m_index    )
{
  static_assert(is_const, "is_const must be true.");
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst> &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator++()
noexcept
{
  ++m_index;
  return *this;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator++(int)
noexcept
{
  generic_iterator tmp(*this);
  ++*this;
  return tmp;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst> &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator--()
noexcept
{
  --m_index;
  return *this;
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator--(int)
noexcept
{
  generic_iterator tmp(*this);
  --*this;
  return tmp;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst> &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator+=(difference_type const n)
noexcept
{
  m_index += n;
  return *this;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst> &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator-=(difference_type const n)
noexcept
{
  m_index -= n;
  return *this;
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
    ::reference
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator*() const
noexcept
{
  return m_pool->m_values[static_cast<size_type>(m_index)];
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
    ::pointer
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator->() const
noexcept
{
  return pointer(**this);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<bool IsConst>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::template generic_iterator<IsConst>
    ::reference
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::generic_iterator<IsConst>
    ::operator[](difference_type const n) const
noexcept
{
  return *(*this + n);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return
      std::is_nothrow_constructible_v<
          value_container,
          value_container &&, allocator_type const &>
   && std::is_nothrow_constructible_v<
          position_container,
          position_container &&, allocator_type const &>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_clear()
noexcept
{
  return noexcept(std::declval<value_container &>().clear());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_erase()
noexcept
{
  return noexcept(std::declval<value_container &>().overwrite_with_back(std::declval<size_type const>()))
      && noexcept(std::declval<value_container &>().pop_back());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<value_container   >
      && std::is_nothrow_swappable_v<position_container>;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::s_noexcept_swap_entities()
noexcept
{
  return noexcept(std::declval<value_container &>().swap(
      std::declval<size_type const>(),
      std::declval<size_type const>()));
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<typename ... Args>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::m_emplace(entity_type const e, Args &&...args)
{
  m_positions.reserve_for(e);
  m_values.emplace_back(e, std::forward<Args>(args)...);
  m_positions[e] = size() - 1;
  return {--end(), true};
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::pool(allocator_type const &alloc)
noexcept
  : m_values   (alloc),
    m_positions(alloc)
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::pool()
noexcept(s_noexcept_default_construct())
  : pool(allocator_type())
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::pool(pool const &other, allocator_type const &alloc)
  : m_values   (other.m_values   , alloc),
    m_positions(other.m_positions, alloc)
{ }

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::pool(pool &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_values   (std::move(other.m_values   ), alloc),
    m_positions(std::move(other.m_positions), alloc)
{ }


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::allocator_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::get_allocator() const
noexcept
{
  return m_values.get_allocator();
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size() const
noexcept
{
  return m_values.size();
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::empty() const
noexcept
{
  return m_values.empty();
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::size_type
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::max_size() const
noexcept
{
  return std::min(m_values.max_size(), m_positions.max_size());
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::begin()
noexcept
{
  return iterator(this, 0);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::begin() const
noexcept
{
  return const_iterator(this, 0);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::end()
noexcept
{
  return iterator(this, size());
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::end() const
noexcept
{
  return const_iterator(this, size());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::cbegin() const
noexcept
{
  return begin();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::cend() const
noexcept
{
  return end();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::rbegin()
noexcept
{
  return reverse_iterator(end());
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::rbegin() const
noexcept
{
  return const_reverse_iterator(end());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::rend()
noexcept
{
  return reverse_iterator(begin());
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::rend() const
noexcept
{
  return const_reverse_iterator(begin());
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::crbegin() const
noexcept
{
  return rbegin();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_reverse_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::crend() const
noexcept
{
  return rend();
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterate(entity_type const e)
noexcept
{
  return iterator(this, m_positions[e]);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterate(entity_type const e) const
noexcept
{
  return const_iterator(this, m_positions[e]);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::contains(entity_type const e) const
noexcept
{
  return m_positions.contains(e)
      && m_values.entities()[m_positions[e]] == e;
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::find(entity_type const e)
noexcept
{
  return contains(e) ? iterate(e) : end();
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::const_iterator
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::find(entity_type const e) const
noexcept
{
  return contains(e) ? iterate(e) : end();
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::component_type &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::operator[](entity_type const e)
noexcept
{
  return m_values.components()[m_positions[e]];
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::component_type const &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::operator[](entity_type const e) const
noexcept
{
  return m_values.components()[m_positions[e]];
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::component_type &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::at(entity_type const e)
{
  if (!contains(e))
    throw std::out_of_range("generic_pool::at");

  return operator[](e);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
typename pool<Component, Entity, Allocator, PageSize, TagValue>
    ::component_type const &
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::at(entity_type const e) const
{
  if (!contains(e))
    throw std::out_of_range("generic_pool::at");

  return operator[](e);
}



template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::clear()
noexcept(s_noexcept_clear())
{
  m_values   .clear();
  m_positions.clear();
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<typename ... Args>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::emplace(entity_type const e, Args &&...args)
{
  if (contains(e))
    return {iterator(this, m_positions[e]), false};

  return m_emplace(e, std::forward<Args>(args)...);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<typename ... Args>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::try_emplace(entity_type const e, Args &&...args)
{
  return emplace(e, std::forward<Args>(args)...);
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::insert(entity_type const e, component_type const &c)
{
  return emplace(e, c);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::insert(entity_type const e, component_type &&c)
{
  return emplace(e, std::move(c));
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
template<typename C>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::insert(entity_type const e, C &&c)
{
  return emplace(e, std::move(c));
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::insert_or_assign(entity_type const e, component_type const &c)
{
  if (contains(e))
  {
    size_type const pos = m_positions[e];

    m_values.assign(pos, c);
    return {iterator(this, pos), false};
  }

  return m_emplace(e, c);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
std::pair<
    typename pool<Component, Entity, Allocator, PageSize, TagValue>
        ::iterator,
    bool>
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::insert_or_assign(entity_type const e, component_type &&c)
{
  if (contains(e))
  {
    size_type const pos = m_positions[e];

    m_values.assign(pos, std::move(c));
    return {iterator(this, pos), false};
  }

  return m_emplace(e, std::move(c));
}


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
bool
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::erase(entity_type const e)
noexcept(s_noexcept_erase())
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


template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::swap(pool &other)
noexcept(s_noexcept_swap())
{
  using std::swap;
  swap(m_values   , other.m_values   );
  swap(m_positions, other.m_positions);
}

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
constexpr
void
pool<Component, Entity, Allocator, PageSize, TagValue>
    ::swap(entity_type const e, entity_type const f)
noexcept(s_noexcept_swap_entities())
{
  m_values   .swap(m_positions[e], m_positions[f]);
  m_positions.swap(e, f);
}



/*!
 * @brief Determines whether the given type is a specialization of pool.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_pool;

template<typename T>
inline constexpr
bool
specializes_pool_v
= specializes_pool<T>::value;

template<typename T>
struct specializes_pool
  : bool_constant<false>
{ };

template<
    typename    Component,
    typename    Entity,
    typename    Allocator,
    std::size_t PageSize,
    bool        TagValue>
struct specializes_pool<
    pool<Component, Entity, Allocator, PageSize, TagValue>>
  : bool_constant<true>
{ };


} // namespace heim::sparse_set_based

#endif // HEIM_SPARSE_SET_BASED_POOL_HPP
