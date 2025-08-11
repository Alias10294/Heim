#ifndef HEIM_HARMONY_ARRANGER_HPP
#define HEIM_HARMONY_ARRANGER_HPP

#include <type_traits>
#include "core/iterator.hpp"

namespace heim
{
template<typename T>
concept arrangeable = requires (
    T &t,
    T &u,
    typename T::entity_type a,
    typename T::entity_type b)
{
  core::iterator<typename T::iterator>;
  core::iterator<typename T::const_iterator>;
  { t.begin() } noexcept;
  { t.end()   } noexcept;

  { t.empty() } noexcept -> std::same_as<bool>;
  { t.size()  } noexcept -> std::same_as<std::size_t>;

  { t.contains(a) } noexcept -> std::same_as<bool>;

  { t.swap(u) };
  { t.swap(t) };

};

template<typename T>
requires arrangeable<T>
class arranger
{
public:
  using entity_type   = typename T::entity_type;
  using arranged_type = T;

public:
  constexpr
  arranger()
  = default;
  constexpr
  arranger(arranger const &)
  = default;
  constexpr
  arranger(arranger &&)
  noexcept
  = default;
  constexpr
  arranger(T& arranged)
    : arranged_{arranged}
  { }

  constexpr
  ~arranger()
  noexcept
  = default;


  constexpr
  arranger &operator=(arranger const &)
  = default;
  constexpr
  arranger &operator=(arranger &&)
  noexcept
  = default;


  constexpr
  void swap(arranger &other)
  noexcept(noexcept(std::declval<T &>().swap(std::declval<T &>())))
  {
    arranged_.swap(other.arranged_);
  }



  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return arranged_.size();
  }



  constexpr
  void operator()(entity_type const lhs, std::ptrdiff_t const len)
  noexcept(noexcept(std::declval<T &>().swap(0, 0)))
  {
    arranged_.swap(lhs, std::get<0>(*(arranged_.begin() + len)));
  }

private:
  T &arranged_;

};

}

#endif // HEIM_HARMONY_ARRANGER_HPP
