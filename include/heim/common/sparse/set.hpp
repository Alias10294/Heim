#ifndef HEIM_COMMON_SPARSE_SET_HPP
#define HEIM_COMMON_SPARSE_SET_HPP

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
template<typename, typename Container, bool = requires { typename Container::allocator_type; }>
struct sparse_set_default_allocator
{ };

template<typename T, typename Container>
struct sparse_set_default_allocator<T, Container, false>
  : std::type_identity<std::allocator<T>>
{ };

template<typename T, typename Container>
struct sparse_set_default_allocator<T, Container, true>
  : std::type_identity<typename Container::allocator_type>
{ };

template<typename T, typename Container>
using sparse_set_default_allocator_t
= typename sparse_set_default_allocator<T, Container>::type;

} // namespace detail


template<
    typename    UInt,
    std::size_t PageSize      = default_page_size_v,
    typename    Container     = std::vector<UInt>,
    typename    Allocator     = detail::sparse_set_default_allocator_t<UInt, Container>>
requires
    std::unsigned_integral<UInt>
 && std::same_as<UInt, std::ranges::range_value_t<Container>>
 && (!requires { typename Container::allocator_type; } || std::uses_allocator_v<Container, Allocator>)
class sparse_set
{
public:
  static constexpr std::size_t page_size
  = PageSize;

  using key_type               = UInt;
  using container_type         = Container;
  using allocator_type         = Allocator;
  using value_type             = UInt;
  using reference              = value_type &;
  using const_reference        = value_type const &;
  using size_type              = std::size_t;
  using difference_type        = typename container_type::difference_type;
  using reverse_iterator       = typename container_type::const_iterator;
  using const_reverse_iterator = typename container_type::const_iterator;
  using iterator               = std::reverse_iterator<reverse_iterator>;
  using const_iterator         = std::reverse_iterator<const_reverse_iterator>;

private:
  using index_type
  = detail::sparse_index<UInt, PageSize, Allocator>;

private:
  container_type m_cont;
  index_type     m_index;

private:
  constexpr
  void
  m_unique()
  {
    auto const original_size{this->m_cont.size()};
    auto const index_limit  {this->m_index.max_size()};
    size_type  out          {};

    auto unique
    = [&]
        <bool AddCapacicityCheck>
        {
          for (size_type in{}; in != original_size; ++in)
          {
            key_type const key{this->m_cont[in]};
            auto          &idx{this->m_index.assure(key)};

            if (idx != std::numeric_limits<key_type>::max())
              continue;

            if constexpr (AddCapacicityCheck)
            {
              if (out == index_limit) [[unlikely]]
                throw std::length_error{"sparse_set::m_unique"};
            }

            if (out != in)
              this->m_cont[out] = std::move(this->m_cont[in]);

            idx = static_cast<key_type>(out);
            ++out;
          }
        };

    if (original_size > index_limit)
      unique.template operator()<true>();
    else
      unique.template operator()<false>();

    auto const new_end{std::ranges::begin(this->m_cont) + static_cast<difference_type>(out)};

    this->m_cont.erase(new_end, std::ranges::end(this->m_cont));
  }

  constexpr
  allocator_type
  m_choose_index_allocator() const
  noexcept
  {
    if constexpr (std::uses_allocator_v<container_type, allocator_type>)
      return this->m_cont.get_allocator();
    else
      return allocator_type{};
  }

  constexpr
  iterator
  m_unchecked_find_with_idx(difference_type idx)
  noexcept
  { return std::make_reverse_iterator(std::next(std::ranges::cbegin(this->m_cont) + idx)); }

  constexpr
  const_iterator
  m_unchecked_find_with_idx(difference_type idx) const
  noexcept
  { return std::make_reverse_iterator(std::next(std::ranges::cbegin(this->m_cont) + idx)); }


  constexpr
  void
  m_increment_indices(
      std::ranges::iterator_t<container_type> first,
      std::ranges::iterator_t<container_type> last)
  noexcept
  {
    for (; first != last; ++first)
      ++this->m_index[*first];
  }

  constexpr
  iterator
  m_unchecked_insert(value_type value)
  {
    auto &idx{this->m_index.assure(value)};
    this->m_cont.emplace_back(value);

    idx = std::ranges::size(this->m_cont) - 1;

    return m_unchecked_find_with_idx(idx);
  }

  constexpr
  iterator
  m_unchecked_insert(const_iterator pos, value_type value)
  {
    auto &idx {this->m_index.assure(value)};
    auto first{this->m_cont .emplace(pos.base(), value)};

    idx = std::distance(std::ranges::begin(this->m_cont), first);
    this->m_increment_indices(++first, std::ranges::end(this->m_cont));

    return m_unchecked_find_with_idx(idx);
  }

public:
  constexpr
  sparse_set()
    : sparse_set{allocator_type{}}
  { }

  constexpr
  sparse_set(sparse_set const &other)
  = default;

  constexpr
  sparse_set(sparse_set &&)
  = default;

  explicit constexpr
  sparse_set(container_type &&cont)
    : m_cont {std::move(cont)}
    , m_index{this->m_choose_index_allocator()}
  { this->m_unique(); }

  explicit constexpr
  sparse_set(unique_t, container_type const &cont)
    : m_cont {cont}
    , m_index{this->m_cont, this->m_choose_index_allocator()}
  { }

  explicit constexpr
  sparse_set(unique_t, container_type &&cont)
    : m_cont {std::move(cont)}
    , m_index{this->m_cont, this->m_choose_index_allocator()}
  { }

  constexpr
  sparse_set(sparse_set const &other, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, other.m_cont)}
    , m_index{other.m_index, alloc}
  { }

  constexpr
  sparse_set(sparse_set &&other, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(other.m_cont))}
    , m_index{std::move(other.m_index), alloc}
  { }

  constexpr
  sparse_set(container_type const &cont, allocator_type const &alloc)
    : sparse_set{alloc}
  { this->insert_range(cont); }

  constexpr
  sparse_set(container_type &&cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(cont))}
    , m_index{alloc}
  { this->m_unique(); }

  constexpr
  sparse_set(unique_t, container_type const &cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, cont)}
    , m_index{this->m_cont, alloc}
  { }

  constexpr
  sparse_set(unique_t, container_type &&cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(cont))}
    , m_index{this->m_cont, alloc}
  { }

  explicit constexpr
  sparse_set(allocator_type const &alloc)
  noexcept
    : m_cont {std::make_obj_using_allocator<container_type>(alloc)}
    , m_index{alloc}
  { }

  template<typename InputIt>
  constexpr
  sparse_set(InputIt first, InputIt last)
    : sparse_set{}
  { this->insert(first, last); }

  template<typename InputIt>
  constexpr
  sparse_set(unique_t, InputIt first, InputIt last)
    : m_cont {first, last}
    , m_index{this->m_cont, this->m_choose_index_allocator()}
  { }

  template<typename InputIt>
  constexpr
  sparse_set(InputIt first, InputIt last, allocator_type const &alloc)
    : sparse_set{alloc}
  { this->insert(first, last); }

  template<typename InputIt>
  constexpr
  sparse_set(unique_t, InputIt first, InputIt last, allocator_type const &alloc)
    : m_cont {first, last , alloc}
    , m_index{this->m_cont, alloc}
  { }

  template<typename R>
  constexpr
  sparse_set(std::from_range_t, R &&rg)
    : sparse_set{std::ranges::begin(rg), std::ranges::end(rg)}
  { }

  template<typename R>
  constexpr
  sparse_set(std::from_range_t, R &&rg, allocator_type const &alloc)
    : sparse_set{std::ranges::begin(rg), std::ranges::end(rg), alloc}
  { }

  constexpr
  sparse_set(std::initializer_list<value_type> init)
    : sparse_set{init.begin(), init.end()}
  { }

  constexpr
  sparse_set(unique_t, std::initializer_list<value_type> init)
    : sparse_set{unique, init.begin(), init.end()}
  { }

  constexpr
  sparse_set(std::initializer_list<value_type> init, allocator_type const &alloc)
    : sparse_set{init.begin(), init.end(), alloc}
  { }

  constexpr
  sparse_set(unique_t, std::initializer_list<value_type> init, allocator_type const &alloc)
    : sparse_set{unique, init.begin(), init.end(), alloc}
  { }

  constexpr
  ~sparse_set()
  = default;

  constexpr
  sparse_set &
  operator=(sparse_set const &other)
  {
    if (this == std::addressof(other))
      return *this;

    detail::clear_guard guard{this};

    this->m_cont  = other.m_cont;
    this->m_index = other.m_index;

    guard.disable();
    return *this;
  }

  constexpr
  sparse_set &
  operator=(sparse_set &&other)
  {
    if (this == std::addressof(other))
      return *this;

    detail::clear_guard guard      {this};
    detail::clear_guard other_guard{std::addressof(other)};

    this->m_cont  = std::move(other.m_cont);
    this->m_index = std::move(other.m_index);

    guard.disable();
    return *this;
  }

  constexpr
  sparse_set &
  operator=(std::initializer_list<value_type> ilist)
  {
    this->clear();
    this->insert_range(ilist);

    return *this;
  }

  constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return this->m_index.get_allocator(); }

  constexpr
  void
  swap(sparse_set &other)
  {
    std::ranges::swap(this->m_cont , other.m_cont);
    std::ranges::swap(this->m_index, other.m_index);
  }


  friend constexpr
  void
  swap(sparse_set &lhs, sparse_set &rhs)
  { lhs.swap(rhs); }

  friend constexpr
  bool
  operator==(sparse_set const &lhs, sparse_set const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (key_type key : lhs)
    { if (!rhs.contains(key)) return false; }

    return true;
  }


  constexpr
  iterator
  begin()
  noexcept
  { return std::ranges::rbegin(this->m_cont); }

  constexpr
  const_iterator
  begin() const
  noexcept
  { return std::ranges::crbegin(this->m_cont); }

  constexpr
  const_iterator
  cbegin() const
  noexcept
  { return std::ranges::crbegin(this->m_cont); }

  constexpr
  iterator
  end()
  noexcept
  { return std::ranges::rend(this->m_cont); }

  constexpr
  const_iterator
  end() const
  noexcept
  { return std::ranges::crend(this->m_cont); }

  constexpr
  const_iterator
  cend() const
  noexcept
  { return std::ranges::crend(this->m_cont); }

  constexpr
  reverse_iterator
  rbegin()
  noexcept
  { return std::ranges::begin(this->m_cont); }

  constexpr
  const_reverse_iterator
  rbegin() const
  noexcept
  { return std::ranges::cbegin(this->m_cont); }

  constexpr
  const_reverse_iterator
  crbegin() const
  noexcept
  { return std::ranges::cbegin(this->m_cont); }

  constexpr
  reverse_iterator
  rend()
  noexcept
  { return std::ranges::end(this->m_cont); }

  constexpr
  const_reverse_iterator
  rend() const
  noexcept
  { return std::ranges::cend(this->m_cont); }

  constexpr
  const_reverse_iterator
  crend() const
  noexcept
  { return std::ranges::cend(this->m_cont); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return std::ranges::empty(this->m_cont); }

  [[nodiscard]] constexpr
  size_type
  size() const
  noexcept
  { return std::ranges::size(this->m_cont); }

  [[nodiscard]] constexpr
  size_type
  max_size() const
  noexcept
  { return std::min(this->m_cont.max_size(), this->m_index.max_size()); }


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
  container_type
  extract() &&
  noexcept
  {
    detail::clear_guard guard{this};

    return std::move(this->m_cont);
  }


  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  emplace(Args &&...args)
  { return std::pair{this->m_unchecked_insert(value_type{std::forward<Args>(args)...}), true}; }

  template<typename ...Args>
  constexpr
  iterator
  emplace_hint(const_iterator hint, Args &&...args)
  { return this->m_unchecked_insert(hint, value_type{std::forward<Args>(args)...}); }

  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  try_emplace(Args &&...args)
  { return this->insert(value_type{std::forward<Args>(args)...}); }

  template<typename ...Args>
  constexpr
  iterator
  try_emplace_hint(const_iterator hint, Args &&...args)
  { return this->insert(hint, value_type{std::forward<Args>(args)...}); }

  constexpr
  std::pair<iterator, bool>
  insert(value_type value)
  {
    auto res{this->m_index.get(value)};
    if  (res)
      return std::pair{this->m_unchecked_find_with_idx(*res), false};

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_set::insert"};

    return std::pair{this->m_unchecked_insert(value), true};
  }

  constexpr
  iterator
  insert(const_iterator pos, value_type value)
  {
    auto res{this->m_index.get(value)};
    if  (res)
      return this->m_unchecked_find_with_idx(*res);

    if (this->size() >= this->max_size())
      throw std::length_error{"sparse_set::insert"};

    return this->m_unchecked_insert(pos, value);
  }

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

  constexpr
  void
  erase(key_type key)
  noexcept
  {
    auto &idx{this->m_index[key]};

    if (idx != this->m_cont.size() - 1)
    {
      auto back{this->m_cont.back()};

      this->m_cont [idx ] = back;
      this->m_index[back] = idx;
    }

    this->m_cont.pop_back();
    idx = std::numeric_limits<value_type>::max();
  }

  constexpr
  iterator
  erase(iterator pos)
  noexcept
  {
    auto const offset{pos - this->begin()};

    this->erase(*pos);
    return this->begin() + offset;
  }

  constexpr
  iterator
  erase(iterator first, iterator last)
  noexcept
  {
    while (first != last)
      first = this->erase(first);

    return first;
  }

  constexpr
  bool
  try_erase(key_type key)
  noexcept
  {
    auto res{this->m_index.get(key)};
    if (!res)
      return false;

    if (*res != this->m_cont.size() - 1)
    {
      auto back{this->m_cont.back()};

      this->m_cont [*res] = back;
      this->m_index[back] = *res;
    }

    this->m_cont.pop_back();
    *res = std::numeric_limits<value_type>::max();

    return true;
  }

  constexpr
  void
  clear()
  noexcept
  {
    this->m_cont .clear();
    this->m_index.clear();
  }

  constexpr
  void
  replace(container_type &&cont)
  {
    detail::clear_guard guard{this};

    sparse_set other{std::move(cont), this->get_allocator()};
    this->swap(other);

    guard.disable();
  }

  constexpr
  void
  replace(unique_t, container_type &&cont)
  {
    detail::clear_guard guard{this};

    sparse_set other{unique, std::move(cont), this->get_allocator()};
    this->swap(other);

    guard.disable();
  }
};

} // namespace heim

#endif // HEIM_COMMON_SPARSE_SET_HPP
