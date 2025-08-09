#ifndef HEIM_SYSTEM_HARMONIZED_ITERATOR_HPP
#define HEIM_SYSTEM_HARMONIZED_ITERATOR_HPP

#include <utility>
#include "../harmonized/harmonized.hpp"
#include "../component/iterator.hpp"

namespace heim
{
/**
 * @brief The specialized iterator for known-to-be harmonized compositions.
 *
 * @tparam Iterators The type of iterators of each harmonized compositions.
 */
template<typename ...Iterators>
requires (iterator<Iterators> && ...)
class harmonized_iterator
{
public:
  using proxy_type = std::tuple<
      std::tuple_element_t<0, typename
          std::tuple_element_t<0, std::tuple<Iterators ...>>::proxy_type>,
      std::tuple_element_t<1, typename Iterators::proxy_type> ...>;

public:
  constexpr
  harmonized_iterator()
  = default;
  constexpr
  harmonized_iterator(harmonized_iterator const &)
  = default;
  constexpr
  harmonized_iterator(harmonized_iterator &&)
  noexcept
  = default;
  constexpr
  explicit
  harmonized_iterator(Iterators &&...iterators)
    : iterators_{std::forward<Iterators>(iterators)...}
  { }

  constexpr
  ~harmonized_iterator()
  noexcept
  = default;


  constexpr
  harmonized_iterator &operator=(harmonized_iterator const &)
  = default;
  constexpr
  harmonized_iterator &operator=(harmonized_iterator &&)
  noexcept
  = default;



  [[nodiscard]]
  constexpr
  proxy_type operator*() const
  noexcept
  {
    return std::apply(
        [](auto const &...iterators)
        {
          return proxy_type{
              std::get<0>(*iterators),
              std::get<1>(*iterators)...};
        },
        iterators_);
  }



  constexpr
  harmonized_iterator &operator++()
  noexcept
  {
    std::apply(
        [](auto &...iterators)
        {
          (++iterators, ...);
        },
        iterators_);
    return *this;
  }

  constexpr
  harmonized_iterator &operator--()
  noexcept
  {
    std::apply(
        [](auto &...iterators)
        {
          (--iterators, ...);
        },
        iterators_);
    return *this;
  }


  [[nodiscard]]
  constexpr
  harmonized_iterator operator+(std::ptrdiff_t const dist)
  noexcept
  {
    return std::apply(
        [dist](auto const &...iterators)
        {
          return harmonized_iterator{(iterators + dist)...};
        },
        iterators_);
  }



  [[nodiscard]]
  constexpr
  bool operator==(harmonized_iterator const &other) const
  noexcept
  {
    return iterators_ == other.iterators_;
  }



  constexpr
  void swap(harmonized_iterator &other)
  noexcept
  {
    iterators_.swap(other.iterators_);
  }

private:
  std::tuple<Iterators ...> iterators_;

};

}

#endif // HEIM_SYSTEM_HARMONIZED_ITERATOR_HPP
