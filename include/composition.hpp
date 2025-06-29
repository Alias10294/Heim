#ifndef HEIM_COMPOSITION_HPP
#define HEIM_COMPOSITION_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>


#include "entity.hpp"

namespace heim
{
class basic_composition
{
public:
  virtual ~basic_composition() = default;

};


template<typename Component>
class composition final : public basic_composition
{
  static_assert(std::is_move_constructible_v<Component>,
      "heim::composition: Component must be MoveConstructible");
  static_assert(std::is_move_assignable_v<Component>,
      "heim::composition: Component must be MoveAssignable");

public:
  using entity_type     = entity;
  using component_type  = Component;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type null_idx =
      std::numeric_limits<size_type>::max();
  static constexpr size_type page_size = 4096;

  using component_pointer       = component_type*;
  using const_component_pointer = const component_type*;

  using component_reference       = component_type&;
  using const_component_reference = const component_type&;





  template<typename Composition>
  class basic_iterator
  {
  public:
    struct proxy
    {
      entity_type         entity;
      component_reference component;
    };

    class arrow_proxy
    {
    public:
      explicit arrow_proxy(proxy p)
          noexcept :
        proxy_{std::move(p)}
      {}


      proxy* operator->()
      { return std::addressof(proxy_); }

    private:
      proxy proxy_;

    };


    using iterator_category = std::random_access_iterator_tag;
    using value_type        = proxy;
    using size_type         = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using pointer           = arrow_proxy;
    using reference         = value_type;

    using composition_type  = Composition;



    explicit constexpr basic_iterator(Composition* c, const std::size_t idx)
      noexcept :
      composition_(c),
      idx_(idx)
    { }



    constexpr reference operator*() const
    {
      return proxy
      { composition_->entities_[idx_], composition_->components_[idx_] };
    }

    constexpr pointer operator->() const
    { return pointer{ **this }; }


    constexpr basic_iterator& operator++()
    {
      ++idx_;
      return *this;
    }
    constexpr basic_iterator  operator++(int)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr basic_iterator& operator--()
    {
      --idx_;
      return *this;
    }
    constexpr basic_iterator  operator--(int)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }


    constexpr basic_iterator  operator+(const std::ptrdiff_t n) const
    { return basic_iterator(composition_, idx_ + n); }

    constexpr basic_iterator  operator-(const std::ptrdiff_t n) const
    { return basic_iterator(composition_, idx_ - n); }
    constexpr difference_type operator-(const basic_iterator& other) const
    { return idx_ - other.idx_; }


    constexpr bool operator==(const basic_iterator& other) const
    { return idx_ == other.idx_; }
    constexpr bool operator!=(const basic_iterator& other) const
    { return !(*this == other); }


    constexpr bool operator<(const basic_iterator& other) const
    { return idx_ < other.idx_; }
    constexpr bool operator>(const basic_iterator& other) const
    { return idx_ > other.idx_; }


    constexpr bool operator<=(const basic_iterator& other) const
    { return !(*this > other); }
    constexpr bool operator>=(const basic_iterator& other) const
    { return !(*this < other); }

  private:
    composition_type* composition_;
    size_type         idx_;

  };

  using iterator       = basic_iterator<composition>;
  using const_iterator = basic_iterator<const composition>;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using page_type = std::array<size_type, page_size>;



  component_reference       at(const entity_type e)
  {
    if (!contains(e))
      throw std::out_of_range("composition::at: entity out of range");
    return components_.at(sparse_.at(e / page_size).at(e % page_size));
  }
  const component_reference at(const entity_type e) const
  {
    if (!contains(e))
      throw std::out_of_range("composition::at: entity out of range");
    return components_.at(sparse_.at(e / page_size).at(e % page_size));
  }

  constexpr component_reference       operator[](const entity_type e)
      noexcept
  { return components_[sparse_[e / page_size][e % page_size]]; }
  constexpr const_component_reference operator[](const entity_type e) const
      noexcept
  { return components_[sparse_[e / page_size][e % page_size]]; }

  constexpr iterator       find(const entity_type e)
      noexcept
  {
    if (!contains(e))
      return end();
    size_type idx = sparse_[e / page_size][e % page_size];
    return iterator(this, idx);
  }
  constexpr const_iterator find(const entity_type e) const
      noexcept
  {
    if (!contains(e))
      return end();
    size_type idx = sparse_[e / page_size][e % page_size];
    return const_iterator(this, idx);
  }

  [[nodiscard]]
  constexpr bool contains(const entity_type e) const
      noexcept
  {
    return e / page_size < sparse_.size()
        && sparse_[e / page_size][e % page_size] < entities_.size()
        && entities_[sparse_[e / page_size][e % page_size]] == e;
  }



  [[nodiscard]]
  constexpr iterator       begin()
      noexcept
  { return iterator(this, 0); }
  [[nodiscard]]
  constexpr const_iterator begin() const
      noexcept
  { return const_iterator(this, 0); }

  [[nodiscard]]
  constexpr const_iterator cbegin() const
      noexcept
  { return begin(); }


  [[nodiscard]]
  constexpr iterator       end()
      noexcept
  { return iterator(this, entities_.size()); }
  [[nodiscard]]
  constexpr const_iterator end() const
      noexcept
  { return const_iterator(this, entities_.size()); }
  [[nodiscard]]
  constexpr const_iterator cend() const
      noexcept
  { return end(); }


  [[nodiscard]]
  constexpr reverse_iterator       rbegin()
      noexcept
  { return reverse_iterator(end()); }
  [[nodiscard]]
  constexpr const_reverse_iterator rbegin() const
      noexcept
  { return const_reverse_iterator(end()); }

  [[nodiscard]]
  constexpr const_reverse_iterator crbegin() const
      noexcept
  { return rbegin(); }


  [[nodiscard]]
  constexpr reverse_iterator       rend()
      noexcept
  { return reverse_iterator(begin()); }
  [[nodiscard]]
  constexpr const_reverse_iterator rend() const
      noexcept
  { return const_reverse_iterator(begin()); }
  [[nodiscard]]
  constexpr const_reverse_iterator crend() const
      noexcept
  { return rend(); }



  [[nodiscard]]
  constexpr bool empty() const
      noexcept
  { return entities_.empty(); }


  [[nodiscard]]
  constexpr size_type size()     const
      noexcept
  { return entities_.size(); }

  [[nodiscard]]
  constexpr size_type max_size() const
      noexcept
  { return entities_.max_size(); }


  constexpr void reserve(size_type n)
      noexcept(std::is_nothrow_move_constructible_v<component_type>)
  {
    entities_.reserve(n);
    components_.reserve(n);
  }

  [[nodiscard]]
  constexpr size_type capacity() const
      noexcept
  { return entities_.capacity(); }

  constexpr void shrink_to_fit()
      noexcept(std::is_nothrow_move_constructible_v<component_type>)
  {
    entities_.shrink_to_fit();
    components_.shrink_to_fit();
  }



  constexpr void clear()
  {
    sparse_.clear();
    sparse_.shrink_to_fit();
    entities_.clear();
    components_.clear();
  }


  template<typename... Args>
  constexpr bool emplace(const entity_type e, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<component_type, Args&&...>)
  {
    if (contains(e))
      return false;

    const size_type page_idx = e / page_size;
    const size_type line_idx = e % page_size;

    if (const size_type old_size = sparse_.size();
        page_idx >= old_size)
    {
      const size_type new_size = page_idx + 1;
      sparse_.resize(new_size);
      for (size_type i = old_size; i < new_size; ++i)
        sparse_[i].fill(null_idx);
    }

    sparse_[page_idx][line_idx] = entities_.size();
    entities_.emplace_back(e);
    components_.emplace_back(std::forward<Args>(args)...);
    return true;
  }


  constexpr void     erase(const entity_type e)
      noexcept
  {
    if (!contains(e))
      return;

    const size_type page_idx = e / page_size;
    const size_type line_idx = e % page_size;
    size_type idx = sparse_[page_idx][line_idx];

    if (idx != entities_.size() - 1)
    {
      entities_[idx]   = entities_.back();
      components_[idx] = std::move(components_.back());

      sparse_[entities_.back() / page_size][entities_.back() % page_size] = idx;
    }

    sparse_[page_idx][line_idx] = null_idx;

    entities_.pop_back();
    components_.pop_back();
  }
  constexpr iterator erase(iterator       pos)
      noexcept
  {
    erase((*pos).entity);
    return iterator(this, pos.idx_);
  }
  constexpr iterator erase(const_iterator pos)
      noexcept
  {
    erase((*pos).entity);
    return iterator(this, pos.idx_);
  }
  constexpr iterator erase(iterator       first, iterator       last)
      noexcept
  {
    while (first != last)
      first = erase(first);
    return first;
  }
  constexpr iterator erase(const_iterator first, const_iterator last)
      noexcept
  {
    while (first != last)
      first = erase(first);
    return first;
  }


  constexpr void sort(
      std::function<bool(const Component&, const Component&)> cmp)
      noexcept(std::is_nothrow_move_assignable_v<component_type>)
  {
    const size_type n = entities_.size();
    if (n <= 1)
      return;

    std::vector<size_type> order(n);
    std::iota(order.begin(), order.end(), 0ULL);

    std::sort(order.begin(), order.end(),
        [&](size_type lhs, size_type rhs)
        { return cmp(components_[lhs], components_[rhs]); });

    for (size_type i = 0; i < n; ++i)
    {
      size_type j = i;
      while (order[j] != i)
      {
        std::swap(entities_[j], entities_[order[j]]);
        std::swap(components_[j], components_[order[j]]);
        sparse_[entities_[j] / page_size][entities_[j] % page_size] = j;

        const size_type next = order[j];
        order[j] = j;
        j = next;
      }
      sparse_[entities_[j] / page_size][entities_[j] % page_size] = j;
      order[j] = j;
    }
  }

private:
  std::vector<page_type>      sparse_;
  std::vector<entity_type>    entities_;
  std::vector<component_type> components_;

};
}

#endif // HEIM_COMPOSITION_HPP
