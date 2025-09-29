#ifndef HEIM_MAP_HPP
#define HEIM_MAP_HPP

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "component.hpp"
#include "entity.hpp"

namespace heim
{
template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
class map
{
  static_assert(
      is_entity_v<Entity>,
      "heim::map: Entity must satisfy: "
          "is_entity_v<Entity> ");
  static_assert(
      is_component_v<Component>,
      "heim::map: Component must satisfy: "
          "is_component_v<Component> ");
  static_assert(
      is_component_allocator_v<Component, Allocator>,
      "heim::map: Allocator must satisfy: "
          "is_component_allocator_v<Component, Allocator> ");

public:
  using size_type
  = std::size_t;



  using entity_type
  = Entity;

  using component_type
  = Component;

  constexpr
  static size_type page_size
  = PageSize;

  using component_allocator_type
  = Allocator;

private:
  template<bool IsConst>
  class generic_iterator
  {
  public:
    constexpr
    static bool is_const
    = IsConst;



    using entity_type
    = Entity;

    using component_type
    = std::conditional_t<
        is_const,
        Component const,
        Component>;



    using proxy
    = std::tuple<entity_type const, component_type &>;

  private:
    entity_type const *entities_;
    component_type    *components_;

  public:
    constexpr
    generic_iterator()
    = default;

    constexpr
    generic_iterator(generic_iterator const &)
    = default;

    constexpr
    generic_iterator(generic_iterator &&)
    noexcept
    = default;

    constexpr
    generic_iterator(
        entity_type const *entities,
        component_type    *components)
      : entities_  {entities},
        components_{components}
    { }


    constexpr
    ~generic_iterator()
    noexcept
    = default;


    constexpr
    generic_iterator &operator=(generic_iterator const &)
    = default;

    constexpr
    generic_iterator &operator=(generic_iterator &&)
    noexcept
    = default;



    constexpr
    proxy operator*() const
    noexcept
    {
      return proxy{
          *entities_,
          *components_};
    }



    constexpr
    generic_iterator &operator++()
    noexcept
    {
      ++entities_;
      ++components_;

      return *this;
    }



    constexpr
    bool operator==(generic_iterator const &other) const
    noexcept
    {
      return entities_ == other.entities_;
    }

  };

public:
  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

  using proxy       = typename iterator::proxy;
  using const_proxy = typename const_iterator::proxy;

private:
  using page_type
  = std::array<size_type, page_size>;

  constexpr
  static size_type null_index_
  = std::numeric_limits<size_type>::max();

private:
  std::vector<std::unique_ptr<page_type>> pages_;

  std::vector<entity_type>    entities_;
  std::vector<component_type> components_;



  constexpr
  bool is_blank(page_type const &page)
  noexcept
  {
    for (auto const idx : page)
    {
      if (idx != null_index_)
        return false;
    }
    return true;
  }


  constexpr
  static size_type page_index(entity_type const e)
  noexcept
  {
    return e / page_size;
  }


  constexpr
  static size_type line_index(entity_type const e)
  noexcept
  {
    return e % page_size;
  }


  constexpr
  size_type  index(entity_type const e) const
  noexcept
  {
    return (*pages_[page_index(e)])[line_index(e)];
  }

  constexpr
  size_type &index(entity_type const e)
  noexcept
  {
    return (*pages_[page_index(e)])[line_index(e)];
  }

public:
  constexpr
  map()
  = default;

  constexpr
  map(map const &other)
    : pages_     {},
      entities_  {other.entities_},
      components_{other.components_}
  {
    pages_.reserve(other.pages_.capacity());
    for (auto const &page_uptr : other.pages_)
    {
      if (page_uptr)
        pages_.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        pages_.emplace_back(nullptr);
    }
  }

  constexpr
  map(map &&other)
  noexcept
  = default;


  constexpr
  ~map()
  noexcept
  = default;


  constexpr
  map &operator=(map const &other)
  {
    entities_   = other.entities_;
    components_ = other.components_;

    std::vector<std::unique_ptr<page_type>> pages;

    pages.reserve(other.pages_.capacity());
    for (auto const &page_uptr : other.pages_)
    {
      if (page_uptr)
        pages.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        pages.emplace_back(nullptr);
    }
    pages_ = std::move(pages);

    return *this;
  }

  constexpr
  map &operator=(map &&other)
  noexcept
  = default;



  [[nodiscard]]
  constexpr
  size_type size() const
  noexcept
  {
    return entities_.size();
  }


  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return entities_.empty();
  }


  [[nodiscard]]
  constexpr
  size_type max_size() const
  noexcept
  {
    return entities_.max_size();
  }


  [[nodiscard]]
  constexpr
  size_type capacity() const
  noexcept
  {
    return entities_.capacity();
  }


  constexpr
  void reserve(size_type const n)
  {
    entities_  .reserve(n);
    components_.reserve(n);
  }


  constexpr
  void shrink_to_fit()
  {
    entities_.shrink_to_fit();

    try
    {
      components_.shrink_to_fit();
    }
    catch(...)
    {
      entities_.reserve(components_.capacity());
      throw;
    }

    for (auto &page_uptr : pages_)
    {
      if (page_uptr && is_blank(*page_uptr))
        page_uptr.reset();
    }
    while (!pages_.empty() && !pages_.back())
      pages_.pop_back();
  }



  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return iterator{
        entities_  .data(),
        components_.data()};
  }

  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return const_iterator{
        entities_  .data(),
        components_.data()};
  }


  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return iterator{
      entities_  .data() + size(),
      components_.data() + size()};
  }

  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return const_iterator{
      entities_  .data() + size(),
      components_.data() + size()};
  }


  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return const_iterator{
      entities_  .data(),
      components_.data()};
  }


  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return const_iterator{
      entities_  .data() + size(),
      components_.data() + size()};
  }



  [[nodiscard]]
  constexpr
  bool contains(entity_type const e) const
  noexcept
  {
    return page_index(e) < pages_.size()
        && pages_[page_index(e)]
        && index(e) != null_index_
        && entities_[index(e)] == e;
  }


  [[nodiscard]]
  constexpr
  iterator       find(entity_type const e)
  noexcept
  {
    if (!contains(e))
      return end();

    return iterator{
        entities_  .data() + index(e),
        components_.data() + index(e)};
  }

  [[nodiscard]]
  constexpr
  const_iterator find(entity_type const e) const
  noexcept
  {
    if (!contains(e))
      return end();

    return iterator{
      entities_  .data() + index(e),
      components_.data() + index(e)};
  }


  [[nodiscard]]
  constexpr
  component_type       &operator[](entity_type const e)
  noexcept
  {
    return components_[index(e)];
  }

  [[nodiscard]]
  constexpr
  component_type const &operator[](entity_type const e) const
  noexcept
  {
    return components_[index(e)];
  }


  [[nodiscard]]
  constexpr
  component_type       &at(entity_type const e)
  {
    if (!contains(e))
    {
      throw std::out_of_range{
          "heim::map::at(entity_type): out_of_range"};
    }
    return components_[index(e)];
  }

  [[nodiscard]]
  constexpr
  component_type const &at(entity_type const e) const
  {
    if (!contains(e))
    {
      throw std::out_of_range{
        "heim::map::at(entity_type) const: out_of_range"};
    }
    return components_[index(e)];
  }


  constexpr
  void clear()
  noexcept
  {
    for (auto &page_uptr : pages_)
    {
      if (page_uptr)
        page_uptr->fill(null_index_);
    }

    entities_  .clear();
    components_.clear();
  }


  template<typename ...Args>
  constexpr
  bool emplace_back(entity_type const e, Args &&...args)
  {
    if (contains(e))
      return false;

    if (page_index(e) >= pages_.size())
      pages_.resize(page_index(e) + 1);
    if (!pages_[page_index(e)])
    {
      pages_[page_index(e)] = std::make_unique<page_type>();
      pages_[page_index(e)] ->fill(null_index_);
    }

    components_.emplace_back(std::forward<Args>(args)...);
    try
    {
      entities_.emplace_back(e);
    }
    catch (...)
    {
      components_.pop_back();
    }
    index(e) = size() - 1;

    return true;
  }


  constexpr
  bool pop_swap(entity_type const e)
  noexcept
  {
    if (!contains(e))
      return false;

    if (e != entities_.back())
    {
      entity_type const e_back = entities_.back();

      entities_[index(e)] = e_back;
      components_   [index(e)] = std::move(components_.back());

      index(e_back) = index(e);
    }

    entities_.pop_back();
    components_   .pop_back();

    index(e) = null_index_;

    return true;
  }


  constexpr
  void swap(map &other)
  noexcept
  {
    pages_     .swap(other.pages_);
    entities_  .swap(other.entities_);
    components_.swap(other.components_);
  }

  constexpr
  void swap(entity_type const a, entity_type const b)
  {
    if (!contains(a) || !contains(b))
      return;

    std::swap(entities_  [index(a)], entities_  [index(b)]);
    std::swap(components_[index(a)], components_[index(b)]);

    std::swap(index(a), index(b));
  }

};

template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void swap(
    map<Entity, Component, PageSize, Allocator> &lhs,
    map<Entity, Component, PageSize, Allocator> &rhs)
noexcept
{
  lhs.swap(rhs);
}



template<typename>
struct is_map
  : std::false_type
{ };

template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
struct is_map<
    map<Entity, Component, PageSize, Allocator>>
  : std::true_type
{ };

template<typename T>
constexpr
inline bool is_map_v
= is_map<T>::value;

}

#endif // HEIM_MAP_HPP
