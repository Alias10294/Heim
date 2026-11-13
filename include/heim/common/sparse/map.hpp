#ifndef HEIM_COMMON_SPARSE_MAP_HPP
#define HEIM_COMMON_SPARSE_MAP_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include "index.hpp"

namespace heim
{
namespace detail
{
template<
    typename,
    typename,
    typename KeyContainer,
    typename MappedContainer,
    bool     = requires { typename KeyContainer::allocator_type; typename MappedContainer::allocator_type; }>
struct sparse_map_default_allocator;

template<
    typename UInt,
    typename T,
    typename KeyContainer,
    typename MappedContainer>
struct sparse_map_default_allocator<UInt, T, KeyContainer, MappedContainer, false>
  : std::type_identity<std::allocator<std::pair<UInt, T>>>
{ };

template<
    typename UInt,
    typename T,
    typename KeyContainer,
    typename MappedContainer>
struct sparse_map_default_allocator<UInt, T, KeyContainer, MappedContainer, true>
  : std::type_identity<
        typename std::allocator_traits<typename KeyContainer::allocator_type>
            ::template rebind_alloc<std::pair<UInt, T>>>
{ };

template<
    typename UInt,
    typename T,
    typename KeyContainer,
    typename MappedContainer>
using sparse_map_default_allocator_t
= typename sparse_map_default_allocator<UInt, T, KeyContainer, MappedContainer>::type;

} // namespace detail

template<
    typename    UInt,
    typename    T,
    std::size_t PageSize        = default_page_size_v,
    typename    KeyContainer    = std::vector<UInt>,
    typename    MappedContainer = std::vector<T>,
    typename    Allocator       = detail::sparse_map_default_allocator_t<UInt, T, KeyContainer, MappedContainer>>
requires std::unsigned_integral<UInt>
      && std::same_as<UInt              , std::ranges::range_value_t<KeyContainer>>
      && std::same_as<T                 , std::ranges::range_value_t<MappedContainer>>
      && std::same_as<std::pair<UInt, T>, typename std::allocator_traits<Allocator>::value_type>
      && (!requires { typename KeyContainer   ::allocator_type; } || std::uses_allocator_v<KeyContainer   , Allocator>)
      && (!requires { typename MappedContainer::allocator_type; } || std::uses_allocator_v<MappedContainer, Allocator>)
class sparse_map
{
public:
  static constexpr std::size_t page_size
  = PageSize;

  using key_type              = UInt;
  using mapped_type           = T;
  using key_container_type    = KeyContainer;
  using mapped_container_type = MappedContainer;
  using allocator_type        = Allocator;
  using value_type            = std::pair<key_type        , mapped_type>;
  using reference             = std::pair<key_type const &, mapped_type &>;
  using const_reference       = std::pair<key_type const &, mapped_type const &>;
  using size_type             = std::size_t;
  using difference_type       = std::ptrdiff_t;

  struct containers
  {
    key_container_type    keys;
    mapped_container_type values;
  };

private:
  using alloc_traits
  = std::allocator_traits<allocator_type>;

  using key_allocator   = typename alloc_traits::template rebind_alloc<key_type>;
  using value_allocator = typename alloc_traits::template rebind_alloc<mapped_type>;

  using index_type
  = detail::sparse_index<UInt, PageSize, key_allocator>;


  template<bool IsConst>
  class generic_iterator
  {
    template<bool>
    friend class generic_iterator;
    friend class sparse_map;

  public:
    using value_type        = std::pair<key_type, mapped_type>;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    using reference
    = std::pair<
        key_type const &,
        std::conditional_t<IsConst, mapped_type const &, mapped_type &>>;

    class pointer
    {
    public:
      reference ref;

    public:
      constexpr
      reference const *
      operator->() const
      noexcept
      { return std::addressof(ref); }
    };

  private:
    using key_iterator
    = std::reverse_iterator<std::ranges::iterator_t<key_container_type const>>;

    using value_iterator
    = std::reverse_iterator<
        std::conditional_t<
            IsConst,
            std::ranges::iterator_t<mapped_container_type const>,
            std::ranges::iterator_t<mapped_container_type>>>;

  private:
    key_iterator   m_key_it;
    value_iterator m_value_it;

  private:
    constexpr
    generic_iterator(key_iterator key_it, value_iterator value_it)
      : m_key_it  {key_it}
      , m_value_it{value_it}
    { }

  public:
    constexpr
    generic_iterator()
    = default;

    constexpr
    generic_iterator(generic_iterator<!IsConst> it)
    noexcept
    requires IsConst
      : m_key_it  {it.m_key_it}
      , m_value_it{it.m_value_it}
    { }

    friend constexpr
    bool
    operator==(generic_iterator lhs, generic_iterator rhs)
    noexcept
    { return lhs.m_key_it == rhs.m_key_it; }

    friend constexpr
    auto
    operator<=>(generic_iterator lhs, generic_iterator rhs)
    noexcept
    { return lhs.m_key_it <=> rhs.m_key_it; }


    constexpr
    reference
    operator*() const
    noexcept
    { return reference{*this->m_key_it, *this->m_value_it}; }

    constexpr
    pointer
    operator->() const
    noexcept
    { return pointer{**this}; }

    constexpr
    reference
    operator[](difference_type n) const
    noexcept
    { return *(*this + n); }


    constexpr
    generic_iterator &
    operator++()
    noexcept
    { ++m_key_it; ++m_value_it; return *this; }

    constexpr
    generic_iterator
    operator++(int)
    noexcept
    { generic_iterator tmp{*this}; ++*this; return tmp; }

    constexpr
    generic_iterator &
    operator--()
    noexcept
    { --m_key_it; --m_value_it; return *this; }

    constexpr
    generic_iterator
    operator--(int)
    noexcept
    { generic_iterator tmp{*this}; --*this; return tmp; }

    constexpr
    generic_iterator &
    operator+=(difference_type n)
    noexcept
    { m_key_it += n; m_value_it += n; return *this; }

    constexpr
    generic_iterator &
    operator-=(difference_type n)
    noexcept
    { m_key_it -= n; m_value_it -= n; return *this; }

    friend constexpr
    generic_iterator
    operator+(generic_iterator it, difference_type n)
    noexcept
    { it += n; return it; }

    friend constexpr
    generic_iterator
    operator+(difference_type n, generic_iterator it)
    noexcept
    { it += n; return it; }

    friend constexpr
    generic_iterator
    operator-(generic_iterator it, difference_type n)
    noexcept
    { it -= n; return it; }

    friend constexpr
    difference_type
    operator-(generic_iterator lhs, generic_iterator rhs)
    noexcept
    { return lhs.m_key_it - rhs.m_key_it; }
  };

public:
  using iterator               = generic_iterator<false>;
  using const_iterator         = generic_iterator<true>;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  containers m_conts;
  index_type m_index;

private:
  [[nodiscard]] constexpr
  allocator_type
  m_choose_index_allocator()
  noexcept
  requires std::uses_allocator_v          <key_container_type, key_allocator>
        || std::is_default_constructible_v<key_allocator>
  {
    if constexpr (std::uses_allocator_v<key_container_type, key_allocator>)
      return key_allocator{this->m_conts.keys.get_allocator()};
    else
      return key_allocator{};
  }

  constexpr
  void
  m_unique()
  {
    auto const original_size{static_cast<size_type>(std::ranges::size(this->m_conts.keys))};
    auto const index_limit  {static_cast<size_type>(this->m_index.max_size())};
    size_type  out          {};

    auto unique
    = [&]
        <bool AddCapacicityCheck>
        {
          for (size_type in{}; in != original_size; ++in)
          {
            key_type const key{this->m_conts.keys[in]};
            auto          &idx{this->m_index.assure(key)};

            if (idx != std::numeric_limits<key_type>::max())
              continue;

            if constexpr (AddCapacicityCheck)
            {
              if (out == index_limit) [[unlikely]]
                throw std::length_error{"sparse_map::m_unique"};
            }

            if (out != in)
            {
              this->m_conts.values[out] = std::move(this->m_conts.values[in]);
              this->m_conts.keys  [out] = std::move(this->m_conts.keys  [in]);
            }

            idx = static_cast<key_type>(out);
            ++out;
          }
        };

    if (original_size > index_limit)
      unique.template operator()<true>();
    else
      unique.template operator()<false>();

    auto const key_new_end{
        std::ranges::begin(this->m_conts.keys)
      + static_cast<std::ranges::range_difference_t<key_container_type   >>(out)};
    auto const value_new_end{
        std::ranges::begin(this->m_conts.values)
      + static_cast<std::ranges::range_difference_t<mapped_container_type>>(out)};

    this->m_conts.keys  .erase(key_new_end  , std::ranges::end(this->m_conts.keys));
    this->m_conts.values.erase(value_new_end, std::ranges::end(this->m_conts.values));
  }

  [[nodiscard]] constexpr
  iterator
  m_unchecked_find_with_idx(difference_type idx)
  noexcept
  {
    return iterator{
        std::make_reverse_iterator(
            std::next(
                std::ranges::cbegin(this->m_conts.keys)
              + static_cast<std::ranges::range_difference_t<key_container_type   >>(idx))),
        std::make_reverse_iterator(
            std::next(
                std::ranges::begin (this->m_conts.values)
              + static_cast<std::ranges::range_difference_t<mapped_container_type>>(idx)))};
  }

  [[nodiscard]] constexpr
  const_iterator
  m_unchecked_find_with_idx(difference_type idx) const
  noexcept
  {
    return const_iterator{
        std::make_reverse_iterator(
            std::next(
                std::ranges::cbegin(this->m_conts.keys)
              + static_cast<std::ranges::range_difference_t<key_container_type   >>(idx))),
        std::make_reverse_iterator(
            std::next(
                std::ranges::cbegin(this->m_conts.values)
              + static_cast<std::ranges::range_difference_t<mapped_container_type>>(idx)))};
  }

  constexpr
  void
  m_increment_indices(
      std::ranges::iterator_t<key_container_type> first,
      std::ranges::iterator_t<key_container_type> last)
  noexcept
  {
    for (; first != last; ++first)
      ++this->m_index[*first];
  }

public:
  constexpr
  sparse_map()
  requires std::is_default_constructible_v<allocator_type>
    : sparse_map{allocator_type{}}
  { }

  constexpr
  sparse_map(sparse_map const &)
  = default;

  constexpr
  sparse_map(sparse_map &&)
  = default;

  constexpr
  sparse_map(key_container_type &&keys, mapped_container_type &&values)
    : m_conts{
          .keys  {std::move(keys)},
          .values{std::move(values)}}
    , m_index{this->m_choose_index_allocator()}
  { this->m_unique(); }

  constexpr
  sparse_map(unique_t, key_container_type &&keys, mapped_container_type &&values)
    : m_conts{
          .keys  {std::move(keys)},
          .values{std::move(values)}}
    , m_index{this->m_conts.keys, this->m_choose_index_allocator()}
  { }

  explicit constexpr
  sparse_map(allocator_type const &alloc)
  noexcept
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc})},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc})}}
    , m_index{key_allocator{alloc}}
  { }

  constexpr
  sparse_map(sparse_map const &other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, other.m_conts.keys)},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, other.m_conts.values)}}
    , m_index{other.m_index, key_allocator{alloc}}
  { }

  constexpr
  sparse_map(sparse_map &&other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(other.m_conts.keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(other.m_conts.values))}}
    , m_index{std::move(other.m_index), key_allocator{alloc}}
  { }

  constexpr
  sparse_map(key_container_type const &keys, mapped_container_type const &values, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, keys)},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, values)}}
    , m_index{key_allocator{alloc}}
  { this->m_unique(); }

  constexpr
  sparse_map(key_container_type &&keys, mapped_container_type &&values, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(values))}}
    , m_index{key_allocator{alloc}}
  { this->m_unique(); }

  constexpr
  sparse_map(unique_t, key_container_type const &keys, mapped_container_type const &values, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, keys)},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, values)}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { }

  constexpr
  sparse_map(unique_t, key_container_type &&keys, mapped_container_type &&values, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(values))}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { }

  template<typename ForwardIt>
  constexpr
  sparse_map(ForwardIt first, ForwardIt last)
  requires std::forward_iterator<ForwardIt>
    : sparse_map{first, last, allocator_type{}}
  { }

  template<typename ForwardIt>
  constexpr
  sparse_map(unique_t, ForwardIt first, ForwardIt last)
  requires std::forward_iterator<ForwardIt>
    : sparse_map{unique, first, last, allocator_type{}}
  { }

  template<typename ForwardIt>
  constexpr
  sparse_map(ForwardIt first, ForwardIt last, allocator_type const &alloc)
  requires std::forward_iterator<ForwardIt>
    : sparse_map{alloc}
  { this->insert(first, last); }

  template<typename ForwardIt>
  constexpr
  sparse_map(unique_t, ForwardIt first, ForwardIt last, allocator_type const &alloc)
  requires std::forward_iterator<ForwardIt>
    : m_conts{
          .keys  {
              std::make_obj_using_allocator<key_container_type   >(
                  key_allocator{alloc},
                  std::from_range, std::ranges::subrange{first, last} | std::views::keys)},
          .values{
              std::make_obj_using_allocator<mapped_container_type>(
                  value_allocator{alloc},
                  std::from_range, std::ranges::subrange{first, last} | std::views::values)}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { }

  template<typename R>
  constexpr
  sparse_map(std::from_range_t, R &&rg)
  requires std::ranges::forward_range<R>
    : sparse_map{std::ranges::begin(rg), std::ranges::end(rg)}
  { }

  template<typename R>
  constexpr
  sparse_map(std::from_range_t, R &&rg, allocator_type const &alloc)
  requires std::ranges::forward_range<R>
    : sparse_map{std::ranges::begin(rg), std::ranges::end(rg), alloc}
  { }

  constexpr
  sparse_map(std::initializer_list<value_type> init)
    : sparse_map{init.begin(), init.end()}
  { }

  constexpr
  sparse_map(unique_t, std::initializer_list<value_type> init)
    : sparse_map{unique, init.begin(), init.end()}
  { }

  constexpr
  sparse_map(std::initializer_list<value_type> init, allocator_type const &alloc)
    : sparse_map{init.begin(), init.end(), alloc}
  { }

  constexpr
  sparse_map(unique_t, std::initializer_list<value_type> init, allocator_type const &alloc)
    : sparse_map{unique, init.begin(), init.end(), alloc}
  { }

  constexpr
  ~sparse_map()
  = default;

  constexpr
  sparse_map &
  operator=(sparse_map const &other)
  {
    if (this == std::addressof(other))
      return *this;

    detail::clear_guard guard{this};

    this->m_conts = other.m_conts;
    this->m_index = other.m_index;

    guard.disable();
    return *this;
  }

  constexpr
  sparse_map &
  operator=(sparse_map &&other)
  {
    if (this == std::addressof(other))
      return *this;

    detail::clear_guard guard      {this};
    detail::clear_guard other_guard{std::addressof(other)};

    this->m_conts = std::move(other.m_conts);
    this->m_index = std::move(other.m_index);

    guard.disable();
    return *this;
  }

  constexpr
  sparse_map &
  operator=(std::initializer_list<value_type> ilist)
  {
    this->clear();
    this->insert(ilist);

    return *this;
  }

  constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{this->m_index.get_allocator()}; }

  constexpr
  void
  swap(sparse_map &other)
  {
    std::ranges::swap(this->m_conts.keys  , other.m_conts.keys);
    std::ranges::swap(this->m_conts.values, other.m_conts.values);
    std::ranges::swap(this->m_index       , other.m_index);
  }

  friend constexpr
  void
  swap(sparse_map &lhs, sparse_map &rhs)
  { lhs.swap(rhs); }

  friend constexpr
  bool
  operator==(sparse_map const &lhs, sparse_map const &rhs)
  {
    if (lhs.size() != rhs.size())
      return false;

    for (auto [key, val] : lhs)
    {
      if (!rhs.contains(key))
        return false;
      if (rhs[key] != val)
        return false;
    }

    return true;
  }


  constexpr
  iterator
  begin()
  noexcept
  { return iterator{std::ranges::rbegin(this->m_conts.keys), std::ranges::rbegin(this->m_conts.values)}; }

  constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator{std::ranges::crbegin(this->m_conts.keys), std::ranges::crbegin(this->m_conts.values)}; }

  constexpr
  const_iterator
  cbegin() const
  noexcept
  { return const_iterator{std::ranges::crbegin(this->m_conts.keys), std::ranges::crbegin(this->m_conts.values)}; }

  constexpr
  iterator
  end()
  noexcept
  { return iterator{std::ranges::rend(this->m_conts.keys), std::ranges::rend(this->m_conts.values)}; }

  constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator{std::ranges::crend(this->m_conts.keys), std::ranges::crend(this->m_conts.values)}; }

  constexpr
  const_iterator
  cend() const
  noexcept
  { return const_iterator{std::ranges::crend(this->m_conts.keys), std::ranges::crend(this->m_conts.values)}; }

  constexpr
  reverse_iterator
  rbegin()
  noexcept
  { return std::make_reverse_iterator(this->end()); }

  constexpr
  const_reverse_iterator
  rbegin() const
  noexcept
  { return std::make_reverse_iterator(this->end()); }

  constexpr
  const_reverse_iterator
  crbegin() const
  noexcept
  { return std::make_reverse_iterator(this->cend()); }

  constexpr
  reverse_iterator
  rend()
  noexcept
  { return std::make_reverse_iterator(this->begin()); }

  constexpr
  const_reverse_iterator
  rend() const
  noexcept
  { return std::make_reverse_iterator(this->begin()); }

  constexpr
  const_reverse_iterator
  crend() const
  noexcept
  { return std::make_reverse_iterator(this->cbegin()); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return std::ranges::empty(this->m_conts.keys); }

  [[nodiscard]] constexpr
  size_type
  size() const
  noexcept
  { return static_cast<size_type>(std::ranges::size(this->m_conts.keys)); }

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  {
    return static_cast<size_type>(
        std::min({
            this->m_conts.keys  .max_size(),
            this->m_conts.values.max_size(),
            this->m_index.max_size()}));
  }


  constexpr
  bool
  contains(key_type key) const
  noexcept
  { return static_cast<bool>(this->m_index.get(key)); }

  constexpr
  iterator
  find(key_type key)
  noexcept
  {
    auto   res{this->m_index.get(key)};
    return res
         ? this->m_unchecked_find_with_idx(*res)
         : this->end();
  }

  constexpr
  const_iterator
  find(key_type key) const
  noexcept
  {
    auto   res{this->m_index.get(key)};
    return res
         ? this->m_unchecked_find_with_idx(*res)
         : this->end();
  }

  constexpr
  mapped_type &
  operator[](key_type key)
  noexcept
  { return this->m_conts.values[this->m_index[key]]; }

  constexpr
  mapped_type const &
  operator[](key_type key) const
  noexcept
  { return this->m_conts.values[this->m_index[key]]; }

  constexpr
  mapped_type &
  at(key_type key)
  {
    if (auto res{this->m_index.get(key)})
      return this->m_conts.values[*res];

    throw std::out_of_range{"sparse_map::at"};
  }

  constexpr
  mapped_type const &
  at(key_type key) const
  {
    if (auto res{this->m_index.get(key)})
      return this->m_conts.values[*res];

    throw std::out_of_range{"sparse_map::at"};
  }

  constexpr
  containers
  extract() &&
  noexcept
  {
    detail::clear_guard guard{this};

    return std::move(m_conts);
  }


  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  emplace(key_type key, Args &&...args)
  {
    auto &idx{this->m_index.assure(key)};

    this->m_conts.keys.emplace_back(key);

    try
    { this->m_conts.values.emplace_back(std::forward<Args>(args)...); }
    catch (...)
    { this->m_conts.keys.pop_back(); throw; }

    idx = static_cast<key_type>(std::ranges::size(this->m_conts.keys) - 1);
    return std::pair{this->m_unchecked_find_with_idx(idx), true};
  }

  template<typename ...Args>
  constexpr
  iterator
  emplace_hint(const_iterator, key_type key, Args &&...args)
  { return this->emplace(key, std::forward<Args>(args)...).first; }

  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  try_emplace(key_type key, Args &&...args)
  {
    auto res{this->m_index.get(key)};
    if  (res)
      return std::pair{this->m_unchecked_find_with_idx(*res), false};

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::try_emplace"};

    return this->emplace(key, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  constexpr
  iterator
  try_emplace_hint(const_iterator hint, key_type key, Args &&...args)
  {
    auto res{this->m_index.get(key)};
    if  (res)
      return this->m_unchecked_find_with_idx(*res);

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::try_emplace_hint"};

    return this->emplace_hint(hint, key, std::forward<Args>(args)...);
  }

  constexpr
  std::pair<iterator, bool>
  insert(value_type const &value)
  {
    auto res{this->m_index.get(value.first)};
    if  (res)
      return std::pair{this->m_unchecked_find_with_idx(*res), false};

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert"};

    return this->emplace(value.first, value.second);
  }

  constexpr
  std::pair<iterator, bool>
  insert(value_type &&value)
  {
    auto res{this->m_index.get(value.first)};
    if  (res)
      return std::pair{this->m_unchecked_find_with_idx(*res), false};

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert"};

    return this->emplace(std::move(value.first), std::move(value.second));
  }

  constexpr
  iterator
  insert(const_iterator pos, value_type const &value)
  {
    auto res{this->m_index.get(value.first)};
    if  (res)
      return this->m_unchecked_find_with_idx(*res);

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert"};

    return this->emplace_hint(pos, value.first, value.second);
  }

  constexpr
  iterator
  insert(const_iterator pos, value_type &&value)
  {
    auto res{this->m_index.get(value.first)};
    if  (res)
      return this->m_unchecked_find_with_idx(*res);

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert"};

    return this->emplace_hint(pos, std::move(value.first), std::move(value.second));
  }

  template<typename P>
  constexpr
  std::pair<iterator, bool>
  insert(P &&value)
  { return this->insert(value_type{std::forward<P>(value)}); }

  template<typename P>
  constexpr
  iterator
  insert(const_iterator pos, P &&value)
  { return this->insert(pos, value_type{std::forward<P>(value)}); }

  template<typename InputIt>
  constexpr
  void
  insert(InputIt first, InputIt last)
  { for (; first != last; ++first) this->insert(*first); }

  constexpr
  void
  insert(std::initializer_list<value_type> ilist)
  { this->insert(ilist.begin(), ilist.end()); }

  template<typename R>
  constexpr
  void
  insert_range(R &&rg)
  { this->insert(std::ranges::begin(rg), std::ranges::end(rg)); }

  template<typename M>
  requires std::is_assignable_v<mapped_type &, M &&>
  constexpr
  std::pair<iterator, bool>
  insert_or_assign(key_type key, M &&val)
  {
    if (auto res{this->m_index.get(key)})
    {
      this->m_conts.values[*res] = std::forward<M>(val);
      return std::pair{this->m_unchecked_find_with_idx(*res), false};
    }

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert_or_assign"};

    return this->emplace(key, std::forward<M>(val));
  }

  template<typename M>
  requires std::is_assignable_v<mapped_type &, M &&>
  constexpr
  iterator
  insert_or_assign(const_iterator pos, key_type key, M &&val)
  {
    if (auto res{this->m_index.get(key)})
    {
      this->m_conts.values[*res] = std::forward<M>(val);
      return this->m_unchecked_find_with_idx(*res);
    }

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_map::insert_or_assign"};

    return this->emplace_hint(pos, key, std::forward<M>(val));
  }

  constexpr
  void
  erase(key_type key)
  {
    auto &idx{this->m_index[key]};

    if (idx != std::ranges::size(this->m_conts.keys) - 1)
    {
      auto key_back{this->m_conts.keys.back()};

      this->m_conts.values[idx] = std::move(this->m_conts.values.back());
      this->m_conts.keys  [idx] = key_back;
      this->m_index[key_back  ] = idx;
    }

    this->m_conts.values.pop_back();
    this->m_conts.keys  .pop_back();
    idx = std::numeric_limits<key_type>::max();
  }

  constexpr
  iterator
  erase(iterator pos)
  {
    difference_type const offset{pos - this->begin()};
    key_type        const key   {pos->first};

    this->erase(key);

    return this->begin() + offset;
  }

  constexpr
  iterator
  erase(iterator first, iterator last)
  {
    while (first != last)
      first = this->erase(first);

    return first;
  }

  constexpr
  bool
  try_erase(key_type key)
  {
    auto res{this->m_index.get(key)};
    if (!res)
      return false;

    if (*res != std::ranges::size(this->m_conts.keys) - 1)
    {
      auto key_back{this->m_conts.keys.back()};

      this->m_conts.values[*res] = std::move(this->m_conts.values.back());
      this->m_conts.keys  [*res] = key_back;
      this->m_index[key_back   ] = *res;
    }

    this->m_conts.values.pop_back();
    this->m_conts.keys  .pop_back();
    *res = std::numeric_limits<key_type>::max();

    return true;
  }

  constexpr
  void
  clear()
  noexcept
  {
    this->m_conts.keys  .clear();
    this->m_conts.values.clear();
    this->m_index.clear();
  }

  constexpr
  void
  replace(key_container_type &&keys, mapped_container_type &&values)
  {
    detail::clear_guard guard{this};

    sparse_map other{std::move(keys), std::move(values), this->get_allocator()};
    this->swap(other);

    guard.disable();
  }

  constexpr
  void
  replace(unique_t, key_container_type &&keys, mapped_container_type &&values)
  {
    detail::clear_guard guard{this};

    sparse_map other{unique, std::move(keys), std::move(values), this->get_allocator()};
    this->swap(other);

    guard.disable();
  }
};

} // namespace heim

#endif // HEIM_COMMON_SPARSE_MAP_HPP
