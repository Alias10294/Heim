#ifndef HEIM_INDEX_MAP_HPP
#define HEIM_INDEX_MAP_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace heim
{
template<
    typename    Index,
    typename    T,
    std::size_t PageSize = 4096,
    typename    Alloc    = std::allocator<std::pair<Index const, T>>>
class index_map
{
public:
  using index_type     = Index;
  using mapped_type    = T;

  constexpr static std::size_t
  page_size
  = PageSize;

  using allocator_type = Alloc;


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using value_type = std::pair<index_type const, mapped_type>;

  using reference       = std::pair<index_type const &, mapped_type &>;
  using const_reference = std::pair<index_type const &, mapped_type const &>;


  template<bool IsConst>
  class generic_iterator;

  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

private:
  using index_allocator_type
  = typename std::allocator_traits<allocator_type>
      ::template rebind_alloc<index_type>;

  using index_container_type
  = std::vector<index_type, index_allocator_type>;


  using mapped_allocator_type
  = typename std::allocator_traits<allocator_type>
      ::template rebind_alloc<mapped_type>;

  using mapped_container_type
  = std::vector<mapped_type, mapped_allocator_type>;


  constexpr
  static bool is_paged_v
  = PageSize > 0;

  using page_type
  = std::array<size_type, page_size>;

  // TODO: create custom deleter for pages to use allocator_type

  using position_allocator_type
  = typename std::allocator_traits<allocator_type>
      ::template rebind_alloc<std::conditional_t<
          is_paged_v,
          std::unique_ptr<page_type>,
          size_type>>;

  using position_container_type
  = std::conditional_t<
      is_paged_v,
      std::vector<
          std::unique_ptr<page_type>,
          position_allocator_type>,
      std::vector<
          size_type,
          position_allocator_type>>;

private:
  [[no_unique_address]]
  allocator_type allocator_;

  position_container_type positions_;
  index_container_type    indexes_;
  mapped_container_type   mapped_;

public:
  constexpr
  index_map()
  noexcept(noexcept(allocator_type{}))
    : index_map{allocator_type{}}
  { }

  constexpr explicit
  index_map(allocator_type const &alloc)
  noexcept
    : allocator_{alloc},
      positions_{position_allocator_type{alloc}},
      indexes_  {index_allocator_type   {alloc}},
      mapped_   {mapped_allocator_type  {alloc}}
  { }

  constexpr
  index_map(index_map const &other)
  requires (is_paged_v)
    : allocator_{std::allocator_traits<allocator_type>
          ::select_on_container_copy_construction(other.get_allocator())},
      positions_{position_allocator_type{allocator_}},
      indexes_  {other.indexes_, index_allocator_type   {allocator_}},
      mapped_   {other.mapped_ , mapped_allocator_type  {allocator_}}
  {
    positions_.reserve(other.positions_.capacity());
    for (auto const &page_uptr : other.positions_)
    {
      if (page_uptr)
        positions_.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        positions_.emplace_back(nullptr);
    }
  }

  constexpr
  index_map(index_map const &other)
  requires (!is_paged_v)
    : allocator_{std::allocator_traits<allocator_type>
          ::select_on_container_copy_construction(other.get_allocator())},
      positions_{other.positions_, position_allocator_type{allocator_}},
      indexes_  {other.indexes_  , index_allocator_type   {allocator_}},
      mapped_   {other.mapped_   , mapped_allocator_type  {allocator_}}
  { }

  constexpr
  index_map(index_map &&other)
  noexcept
  = default;

  constexpr
  index_map(
      index_map                            const &other,
      std::type_identity_t<allocator_type> const &alloc)
  requires (is_paged_v)
    : allocator_{alloc},
      positions_{position_allocator_type{allocator_}},
      indexes_  {other.indexes_  , index_allocator_type   {allocator_}},
      mapped_   {other.mapped_   , mapped_allocator_type  {allocator_}}
  {
    positions_.reserve(other.positions_.capacity());
    for (auto const &page_uptr : other.positions_)
    {
      if (page_uptr)
        positions_.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        positions_.emplace_back(nullptr);
    }
  }

  constexpr
  index_map(
      index_map                            const &other,
      std::type_identity_t<allocator_type> const &alloc)
  requires (!is_paged_v)
    : allocator_{alloc},
      positions_{other.positions_, position_allocator_type{allocator_}},
      indexes_  {other.indexes_  , index_allocator_type   {allocator_}},
      mapped_   {other.mapped_   , mapped_allocator_type  {allocator_}}
  { }

  constexpr
  index_map(
      index_map                                 &&other,
      std::type_identity_t<allocator_type> const &alloc)
    : allocator_{alloc},
      positions_{
          std::move(other.positions_),
          position_allocator_type{allocator_}},
      indexes_  {
          std::move(other.indexes_),
          index_allocator_type   {allocator_}},
      mapped_   {
          std::move(other.mapped_),
          mapped_allocator_type  {allocator_}}
  { }

  constexpr
  index_map(
      std::initializer_list<value_type> ilist,
      allocator_type const             &alloc = allocator_type{})
    : index_map{alloc}
  {
    // TODO: implement
  }


  constexpr
  ~index_map()
  noexcept
  = default;


  constexpr index_map &
  operator=(index_map const &other)
  {
    if (this == &other)
      return *this;

    using traits = std::allocator_traits<allocator_type>;
    if constexpr (traits::propagate_on_container_copy_assignment::value)
    {
      if constexpr (!traits::is_always_equal::value)
      {
        if (allocator_ != other.allocator_)
        {
          index_map tmp{other, other.allocator_};
          swap(tmp);
          return *this;
        }
      }
      allocator_ = other.allocator_;
    }

    if constexpr (!is_paged_v)
      positions_ = other.positions_;
    else
    {
      position_container_type positions{position_allocator_type{allocator_}};
      positions.reserve(other.positions_.capacity());
      for (auto const &page_uptr : other.positions_)
      {
        if (page_uptr)
          positions.emplace_back(std::make_unique<page_type>(*page_uptr));
        else
          positions.emplace_back(nullptr);
      }
      positions_.swap(positions);
    }
    indexes_ = other.indexes_;
    mapped_  = other.mapped_;

    return *this;
  }

  constexpr index_map &
  operator=(index_map &&other)
  noexcept(
      std::allocator_traits<allocator_type>
          ::propagate_on_container_copy_assignment::value
   && std::allocator_traits<allocator_type>
          ::is_always_equal::value)
  {
    if constexpr (
        std::allocator_traits<allocator_type>
            ::propagate_on_container_copy_assignment::value
     && std::allocator_traits<allocator_type>
            ::is_always_equal::value)
    {
      index_map tmp{allocator_};
      swap(other);
      tmp.swap(other);

      allocator_ = std::move(other.allocator_);
    }
    else
    {
      if (allocator_ == other.allocator_)
      {
        index_map tmp{allocator_};
        swap(other);
        tmp.swap(other);

        allocator_ = std::move(other.allocator_);
      }
      else
      {
        // allocator cannot move, delegate move assign to each vector
        positions_ = std::move(other.positions_);
        indexes_   = std::move(other.indexes_);
        mapped_    = std::move(other.mapped_);
      }
    }

    return *this;
  }

  constexpr index_map &
  operator=(std::initializer_list<value_type> ilist)
  {
    // TODO: implement

    return *this;
  }


  constexpr void
  swap(index_map &other)
  noexcept(/* TODO: complete */false)
  {
    // TODO: implement
  }

  friend constexpr void
  swap(index_map &lhs, index_map &rhs)
  noexcept(/* TODO: complete */false)
  {
    lhs.swap(rhs);
  }


  constexpr allocator_type
  get_allocator() const
  noexcept
  {
    return allocator_;
  }



  constexpr iterator
  begin()
  noexcept
  {
    // TODO: implement
    return iterator{};
  }

  constexpr const_iterator
  begin() const
  noexcept
  {
    // TODO: implement
    return const_iterator{};
  }


  constexpr iterator
  end()
  noexcept
  {
    // TODO: implement
    return iterator{};
  }

  constexpr const_iterator
  end() const
  noexcept
  {
    // TODO: implement
    return const_iterator{};
  }


  constexpr const_iterator
  cbegin() const
  noexcept
  {
    // TODO: implement
    return const_iterator{};
  }


  constexpr const_iterator
  cend() const
  noexcept
  {
    // TODO: implement
    return const_iterator{};
  }



  constexpr size_type
  size() const
  noexcept
  {
    // TODO: implement
    return size_type{};
  }


  constexpr size_type
  max_size() const
  noexcept
  {
    // TODO: implement
    return size_type{};
  }


  constexpr bool
  empty() const
  noexcept
  {
    return begin() == end();
  }


  constexpr void
  shrink_to_fit()
  noexcept
  {

  }



  constexpr bool
  contains(index_type const idx) const
  noexcept
  {
    // TODO: implement
    return false;
  }



  constexpr void
  clear()
  noexcept
  {
    // TODO: implement
  }



  friend constexpr bool
  operator==(index_map const &lhs, index_map const &rhs)
  noexcept
  {
    if (&lhs == &rhs)
      return true;

    if (lhs.size() != rhs.size())
      return false;

    for (auto [idx, val] : lhs)
    {
      if (!rhs.contains(idx))
        return false;

      if (rhs[idx] != val)
        return false;
    }

    return true;
  }


  friend constexpr bool
  operator!=(index_map const &lhs, index_map const &rhs)
  noexcept
  {
    return !(lhs == rhs);
  }

};


template<
    typename    Index,
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
template<bool IsConst>
class index_map<Index, T, PageSize, Alloc>::generic_iterator
{
public:
  constexpr static bool
  is_const
  = IsConst;

  // TODO: implement
};

}

#endif // HEIM_INDEX_MAP_HPP
