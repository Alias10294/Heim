#ifndef HEIM_COMMON_SPARSE_HPP
#define HEIM_COMMON_SPARSE_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <contracts>
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
#include "unique_allocated_ptr.hpp"

namespace heim
{
template<typename = void>
struct default_page_size
  : std::integral_constant<std::size_t, 1024>
{ };

inline constexpr
std::size_t
default_page_size_v
= default_page_size<>::value;


namespace detail
{
template<
    typename    UInt,
    std::size_t PageSize  = default_page_size_v,
    typename    Allocator = std::allocator<UInt>>
requires std::unsigned_integral<UInt>
      && std::same_as          <UInt, typename std::allocator_traits<Allocator>::value_type>
class sparse_index
{
public:
  static constexpr std::size_t page_size = PageSize;
  static constexpr bool        is_paged  = page_size != 0;

  using value_type     = UInt;
  using allocator_type = Allocator;

private:
  using alloc_traits
  = std::allocator_traits<allocator_type>;

  using page
  = std::array<value_type, page_size>;

  using page_allocator    = typename alloc_traits::template rebind_alloc <page>;
  using page_alloc_traits = typename alloc_traits::template rebind_traits<page>;

  using page_pointer
  = unique_allocated_ptr<page, page_allocator, false>;

  using page_pointer_allocator    = typename alloc_traits::template rebind_alloc <page_pointer>;
  using page_pointer_alloc_traits = typename alloc_traits::template rebind_traits<page_pointer>;

  using container_type
  = std::conditional_t<
      is_paged,
      std::vector<page_pointer, page_pointer_allocator>,
      std::vector<value_type  , allocator_type        >>;

  using container_allocator    = typename container_type::allocator_type;
  using container_alloc_traits = std::allocator_traits<container_allocator>;

private:
  container_type m_cont;
  
private:
  constexpr
  sparse_index(sparse_index const &other, std::bool_constant<true>)
    : m_cont{container_alloc_traits::select_on_container_copy_construction(other.m_cont.get_allocator())}
  { this->m_copy(other.m_cont); }

  constexpr
  sparse_index(sparse_index const &other, std::bool_constant<false>)
    : m_cont{other.m_cont}
  { }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc, std::bool_constant<true>)
    : sparse_index{alloc}
  { this->m_copy(other.m_cont); }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc, std::bool_constant<false>)
    : m_cont{other.m_cont, container_allocator{alloc}}
  { }

  constexpr
  page_pointer
  m_null_page() const
  {
    return page_pointer{
        typename page_pointer::pointer     {},
        typename page_pointer::deleter_type{page_allocator{this->m_cont.get_allocator()}}};
  }

  constexpr
  void
  m_copy(container_type const &from)
  {
    this->m_cont.reserve(from.size());

    for (page_pointer const &ptr : from)
    {
      if (ptr)
        this->m_cont.emplace_back(allocate_unique<page>(page_allocator{this->m_cont.get_allocator()}, *ptr));
      else
        this->m_cont.emplace_back(m_null_page());
    }
  }

  template<typename R>
  constexpr
  void
  m_index(R &&rg)
  {
    value_type idx{};

    for (value_type val : rg)
      this->assure(val) = idx++;
  }

public:
  constexpr
  sparse_index(sparse_index const &other)
    : sparse_index{other, std::bool_constant<is_paged>{}}
  { }

  constexpr
  sparse_index(sparse_index &&)
  = default;

  explicit constexpr
  sparse_index(allocator_type const &alloc)
    : m_cont{container_allocator{alloc}}
  { }

  constexpr
  sparse_index(sparse_index const &other, allocator_type const &alloc)
    : sparse_index{other, alloc, std::bool_constant<is_paged>{}}
  { }

  constexpr
  sparse_index(sparse_index &&other, allocator_type const &alloc)
    : m_cont{std::move(other.m_cont), container_allocator{alloc}}
  { }

  template<typename R>
  explicit constexpr
  sparse_index(R &&rg, allocator_type const &alloc)
  requires std::ranges::input_range<R>
    : sparse_index{alloc}
  { this->m_index(std::forward<R>(rg)); }

  template<typename R>
  explicit constexpr
  sparse_index(R &&rg)
  requires std::ranges::input_range<R>
    : sparse_index{std::forward<R>(rg), allocator_type{}}
  { }

  constexpr
  ~sparse_index()
  = default;

  constexpr
  sparse_index &
  operator=(sparse_index const &other)
  {
    if constexpr (is_paged)
    {
      if (this == std::addressof(other))
        return *this;

      if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
      {
        auto * const ptr{std::addressof(this->m_cont)};

        std::destroy_at  (ptr);
        std::construct_at(ptr, other.m_cont.get_allocator());
      }
      else
        this->m_cont.clear();

      this->m_copy(other.m_cont);
    }
    else
      this->m_cont = other.m_cont;

    return *this;
  }

  constexpr
  sparse_index &
  operator=(sparse_index &&other)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (!is_paged || container_alloc_traits::propagate_on_container_move_assignment::value)
      this->m_cont = std::move(other.m_cont);
    else
    {
      if (this->m_cont.get_allocator() == other.m_cont.get_allocator())
      {
        container_type moved{std::move(other.m_cont), this->m_cont.get_allocator()};
        this->m_cont.swap(moved);
      }
      else
      {
        this->m_cont.clear  ();
        this->m_cont.reserve(other.m_cont.size());

        for (auto &ptr : other.m_cont)
          this->m_cont.emplace_back(std::move(ptr));

        other.m_cont.clear();
      }
    }

    return *this;
  }

  constexpr
  void
  swap(sparse_index &other)
  {
    using std::swap;

    swap(this->m_cont, other.m_cont);
  }

  friend constexpr
  void
  swap(sparse_index &lhs, sparse_index &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  { lhs.swap(rhs); }

  constexpr
  allocator_type
  get_allocator() const
  noexcept
  {
    if constexpr (is_paged)
      return allocator_type{this->m_cont.get_allocator()};
    else
      return this->m_cont.get_allocator();
  }


  constexpr
  auto
  max_size() const
  noexcept
  {
    constexpr
    std::size_t
    value_limit
    = []
        {
          if constexpr (std::numeric_limits<value_type>::digits >= std::numeric_limits<std::size_t>::digits)
            return std::numeric_limits<std::size_t>::max();
          else
            return static_cast<std::size_t>(std::numeric_limits<value_type>::max());
        }();

    if constexpr (is_paged)
    {
      auto const nb_pages{this->m_cont.max_size()};

      return nb_pages > value_limit / page_size
           ? value_limit
           : std::min(nb_pages * page_size, value_limit);
    }
    else
      return std::min(this->m_cont.max_size(), value_limit);
  }


  constexpr
  value_type &
  operator[](value_type val)
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
      return (*this->m_cont[idx / page_size])[idx % page_size];
    else
      return this->m_cont[idx];
  }

  constexpr
  value_type
  operator[](value_type val) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
      return (*this->m_cont[idx / page_size])[idx % page_size];
    else
      return this->m_cont[idx];
  }

  constexpr
  auto
  get(value_type val)
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size() || !this->m_cont[page_idx])
        return static_cast<value_type *>(nullptr);

      auto &value{(*this->m_cont[page_idx])[idx % page_size]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
    else
    {
      if (idx >= this->m_cont.size())
        return static_cast<value_type *>(nullptr);

      auto &value{this->m_cont[idx]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
  }

  constexpr
  auto
  get(value_type val) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size() || !this->m_cont[page_idx])
        return static_cast<value_type const *>(nullptr);

      auto const &value{(*this->m_cont[page_idx])[idx % page_size]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
    else
    {
      if (idx >= this->m_cont.size())
        return static_cast<value_type const *>(nullptr);

      auto const &value{this->m_cont[idx]};
      return value == std::numeric_limits<value_type>::max()
           ? nullptr
           : std::addressof(value);
    }
  }

  constexpr
  value_type &
  assure(value_type val)
  {
    constexpr auto null{std::numeric_limits<value_type>::max()};
    auto const     idx {static_cast<std::size_t>(val)};

    if constexpr (is_paged)
    {
      auto const page_idx{idx / page_size};

      if (page_idx >= this->m_cont.size())
      {
        this->m_cont.reserve(page_idx + 1);

        while (this->m_cont.size() <= page_idx)
          this->m_cont.emplace_back(this->m_null_page());
      }

      auto &ptr{this->m_cont[page_idx]};

      if (!ptr)
      {
        auto allocated{allocate_unique<page>(page_allocator{this->m_cont.get_allocator()})};

        std::destroy_at  (std::addressof(ptr));
        std::construct_at(std::addressof(ptr), std::move(allocated));
        ptr->fill(null);
      }

      return (*ptr)[idx % page_size];
    }
    else
    {
      if (idx >= this->m_cont.size())
        this->m_cont.resize(idx + 1, null);

      return this->m_cont[idx];
    }
  }

  constexpr
  void
  clear()
  noexcept
  { this->m_cont.clear(); }

  template<typename R>
  constexpr
  void
  reset(R &&rg)
  {
    this->clear  ();
    this->m_index(std::forward<R>(rg));
  }
};


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


struct unique_t
{ };

inline constexpr
unique_t
unique;


template<
    typename    UInt,
    std::size_t PageSize      = default_page_size_v,
    typename    Container     = std::vector<UInt>,
    typename    Allocator     = detail::sparse_set_default_allocator_t<UInt, Container>>
requires std::unsigned_integral<UInt>
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
    contract_assert(!this->m_index.get(value));
    contract_assert(this->size() < this->max_size());

    auto &idx{this->m_index.assure(value)};
    this->m_cont.emplace_back(value);

    idx = std::ranges::size(this->m_cont) - 1;

    return m_unchecked_find_with_idx(idx);
  }

  constexpr
  iterator
  m_unchecked_insert(const_iterator pos, value_type value)
  {
    contract_assert(!this->m_index.get(value));
    contract_assert(this->size() < this->max_size());

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
    : m_cont {other.m_cont , alloc}
    , m_index{other.m_index, alloc}
  { }

  constexpr
  sparse_set(sparse_set &&other, allocator_type const &alloc)
    : m_cont {std::move(other.m_cont ), alloc}
    , m_index{std::move(other.m_index), alloc}
  { }

  constexpr
  sparse_set(container_type const &cont, allocator_type const &alloc)
    : sparse_set{alloc}
  { this->insert_range(cont); }

  constexpr
  sparse_set(container_type &&cont, allocator_type const &alloc)
    : m_cont {std::move(cont), alloc}
    , m_index{alloc}
  { this->m_unique(); }

  constexpr
  sparse_set(unique_t, container_type const &cont, allocator_type const &alloc)
    : m_cont {cont        , alloc}
    , m_index{this->m_cont, alloc}
  { }

  constexpr
  sparse_set(unique_t, container_type &&cont, allocator_type const &alloc)
    : m_cont {std::move(cont), alloc}
    , m_index{this->m_cont   , alloc}
  { }

  explicit constexpr
  sparse_set(allocator_type const &alloc)
  noexcept
    : m_cont {alloc}
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
  = default;

  constexpr
  sparse_set &
  operator=(sparse_set &&other)
  = default;

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
  { return std::move(this->m_cont); }


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
    contract_assert(this->m_index.get(key));

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
    sparse_set other{std::move(cont), this->get_allocator()};
    this->swap(other);
  }

  constexpr
  void
  replace(unique_t, container_type &&cont)
  {
    sparse_set other{unique, std::move(cont), this->get_allocator()};
    this->swap(other);
  }
};


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
    contract_assert(std::ranges::size(this->m_conts.keys) == std::ranges::size(this->m_conts.values));

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
  { contract_assert(std::ranges::size(this->m_conts.keys) == std::ranges::size(this->m_conts.values)); }

  explicit constexpr
  sparse_map(allocator_type const &alloc)
  noexcept
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc})},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc})}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { }

  constexpr
  sparse_map(sparse_map const &other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, other.m_conts.keys)},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, other.m_conts.values)}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { }

  constexpr
  sparse_map(sparse_map &&other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(other.m_conts.keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(other.m_conts.values))}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
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
  { contract_assert(std::ranges::size(this->m_conts.keys) == std::ranges::size(this->m_conts.values)); }

  constexpr
  sparse_map(unique_t, key_container_type &&keys, mapped_container_type &&values, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(values))}}
    , m_index{this->m_conts.keys, key_allocator{alloc}}
  { contract_assert(std::ranges::size(this->m_conts.keys) == std::ranges::size(this->m_conts.values)); }

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
  operator=(sparse_map const &)
  = default;

  constexpr
  sparse_map &
  operator=(sparse_map &&)
  = default;

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
  {
    contract_assert(this->m_index.get(key));

    return this->m_conts.values[this->m_index[key]];
  }

  constexpr
  mapped_type const &
  operator[](key_type key) const
  noexcept
  {
    contract_assert(this->m_index.get(key));

    return this->m_conts.values[this->m_index[key]];
  }

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
  { return std::move(m_conts); }


  template<typename ...Args>
  constexpr
  std::pair<iterator, bool>
  emplace(key_type key, Args &&...args)
  {
    contract_assert(!this->m_index.get(key));

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
    contract_assert(this->m_index.get(key));

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
    sparse_map other{std::move(keys), std::move(values), this->get_allocator()};
    this->swap(other);
  }

  constexpr
  void
  replace(unique_t, key_container_type &&keys, mapped_container_type &&values)
  {
    sparse_map other{unique, std::move(keys), std::move(values), this->get_allocator()};
    this->swap(other);
  }
};

} // namespace heim

#endif // HEIM_COMMON_SPARSE_HPP
