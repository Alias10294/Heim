#ifndef HEIM_PROPERTY_PRIMITIVES_HPP
#define HEIM_PROPERTY_PRIMITIVES_HPP

#include <utility>

namespace heim::properties
{
namespace detail
{
struct get_fn
{
private:
  template<typename M, typename K>
  static constexpr bool s_member_fn
  = requires { std::declval<M &&>().get( std::declval<K &&>()); };

  template<typename M, typename K>
  static constexpr bool s_adl_fn
  = requires { get(std::declval<M &&>(), std::declval<K &&>()); };


  template<typename M, typename K>
  requires s_member_fn<M, K> || s_adl_fn<M, K>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<M, K>)
      return noexcept(std::declval<M &&>().get( std::declval<K &&>()));
    else
      return noexcept(get(std::declval<M &&>(), std::declval<K &&>()));
  }

public:
  template<typename M, typename K>
  requires s_member_fn<M, K> || s_adl_fn<M, K>
  [[nodiscard]] constexpr
  decltype(auto)
  operator()(M &&m, K &&k) const
  noexcept(s_noexcept<M, K>())
  {
    if constexpr (s_member_fn<M, K>)
      return std::forward<M>(m).get( std::forward<K>(k));
    else
      return get(std::forward<M>(m), std::forward<K>(k));
  }
};

} // namespace detail

inline constexpr
detail::get_fn
get;


namespace detail
{
struct set_fn
{
private:
  template<typename M, typename K, typename ...Args>
  static constexpr bool s_member_fn
  = requires { std::declval<M &&>().set( std::declval<K &&>(), std::declval<Args &&>()...); };

  template<typename M, typename K, typename ...Args>
  static constexpr bool s_adl_fn
  = requires { set(std::declval<M &&>(), std::declval<K &&>(), std::declval<Args &&>()...); };


  template<typename M, typename K, typename ...Args>
  requires s_member_fn<M, K, Args ...> || s_adl_fn<M, K, Args ...>
  static constexpr
  bool
  s_noexcept()
  noexcept
  {
    if constexpr (s_member_fn<M, K, Args &&...>)
      return noexcept(std::declval<M &&>().set( std::declval<K &&>(), std::declval<Args &&>()...));
    else
      return noexcept(set(std::declval<M &&>(), std::declval<K &&>(), std::declval<Args &&>()...));
  }

public:
  template<typename M, typename K, typename ...Args>
  requires s_member_fn<M, K, Args ...> || s_adl_fn<M, K, Args ...>
  constexpr
  void
  operator()(M &&m, K &&k, Args &&...args) const
  noexcept(s_noexcept<M, K, Args ...>())
  {
    if constexpr (s_member_fn<M, K, Args ...>)
      std::forward<M>(m).set( std::forward<K>(k), std::forward<Args>(args)...);
    else
      set(std::forward<M>(m), std::forward<K>(k), std::forward<Args>(args)...);
  }
};

} // namespace detail

inline constexpr
detail::set_fn
set;

} // namespace heim::properties

#endif // HEIM_PROPERTY_PRIMITIVES_HPP
