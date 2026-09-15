#ifndef HEIM_PROPERTY_PROPERTY_MAP_HPP
#define HEIM_PROPERTY_PROPERTY_MAP_HPP

#include <array>
#include <cstddef>
#include <deque>
#include <flat_map>
#include <functional>
#include <iterator>
#include <map>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "primitives.hpp"

namespace heim
{
struct no_property_getter_t { explicit no_property_getter_t() = default; };
struct no_property_setter_t { explicit no_property_setter_t() = default; };

inline constexpr no_property_getter_t no_property_getter{};
inline constexpr no_property_setter_t no_property_setter{};


template<
    typename Get,
    typename Set>
class property_map
{
public:
  using getter_type = Get;
  using setter_type = Set;

private:
  [[no_unique_address]] getter_type m_getter;
  [[no_unique_address]] setter_type m_setter;

public:
  constexpr
  property_map(getter_type g, setter_type s)
  noexcept(
      std::is_nothrow_move_constructible_v<getter_type>
   && std::is_nothrow_move_constructible_v<setter_type>)
    : m_getter{std::move(g)}
    , m_setter{std::move(s)}
  { }


  template<typename K>
  [[nodiscard]] constexpr
  decltype(auto)
  get(K &&k)
  noexcept(std::is_nothrow_invocable_v<getter_type &, K &&>)
  requires std::is_invocable_v        <getter_type &, K &&>
  { return std::invoke(this->m_getter, std::forward<K>(k)); }

  template<typename K>
  [[nodiscard]] constexpr
  decltype(auto)
  get(K &&k) const
  noexcept(std::is_nothrow_invocable_v<getter_type const &, K &&>)
  requires std::is_invocable_v        <getter_type const &, K &&>
  { return std::invoke(this->m_getter, std::forward<K>(k)); }

  template<typename K, typename ...Args>
  constexpr
  void
  set(K &&k, Args &&...args)
  noexcept(std::is_nothrow_invocable_v<setter_type &, K &&, Args &&...>)
  requires std::is_invocable_v        <setter_type &, K &&, Args &&...>
  { std::invoke(this->m_setter, std::forward<K>(k), std::forward<Args>(args)...); }

  template<typename K, typename ...Args>
  constexpr
  void
  set(K &&k, Args &&...args) const
  noexcept(std::is_nothrow_invocable_v<setter_type const &, K &&, Args &&...>)
  requires std::is_invocable_v        <setter_type const &, K &&, Args &&...>
  { std::invoke(this->m_setter, std::forward<K>(k), std::forward<Args>(args)...); }
};


struct identity_arg_t               { explicit identity_arg_t              () = default; };
struct constant_arg_t               { explicit constant_arg_t              () = default; };
struct associative_container_arg_t  { explicit associative_container_arg_t () = default; };
struct random_access_iterator_arg_t { explicit random_access_iterator_arg_t() = default; };
struct random_access_range_arg_t    { explicit random_access_range_arg_t   () = default; };
struct member_arg_t                 { explicit member_arg_t                () = default; };
struct pointer_arg_t                { explicit pointer_arg_t               () = default; };
struct invocable_arg_t              { explicit invocable_arg_t             () = default; };
struct composable_arg_t             { explicit composable_arg_t            () = default; };

inline constexpr identity_arg_t               identity_arg              {};
inline constexpr constant_arg_t               constant_arg              {};
inline constexpr associative_container_arg_t  associative_container_arg {};
inline constexpr random_access_iterator_arg_t random_access_iterator_arg{};
inline constexpr random_access_range_arg_t    random_access_range_arg   {};
inline constexpr member_arg_t                 member_arg                {};
inline constexpr pointer_arg_t                pointer_arg               {};
inline constexpr invocable_arg_t              invocable_arg             {};
inline constexpr composable_arg_t             composable_arg            {};


namespace detail
{
[[nodiscard]] constexpr
auto
make_property_map_lambdas(identity_arg_t)
noexcept
{
  return std::pair{
      []
          <typename K>
          (K &&k)
          noexcept
          -> decltype(auto)
          { return std::forward<K>(k); },
      no_property_setter};
}

template<typename T>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(constant_arg_t, T &&t)
noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T &&>)
requires std::is_constructible_v        <std::decay_t<T>, T &&>
{
  return std::pair{
      [u{std::forward<T>(t)}]
          <typename K>
          (K &&)
          noexcept
          -> decltype(auto)
          { return (u); },
      no_property_setter};
}

template<typename Cont>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(associative_container_arg_t, Cont &cont)
noexcept
{
  return std::pair{
      [&cont]
          <typename K>
          (K &&k)
          -> decltype(auto)
          requires requires { cont.at(std::forward<K>(k)); }
          { return cont.at(std::forward<K>(k)); },
      [&cont]
          <typename K, typename ...Args>
          (K &&k, Args &&...args)
          requires requires { cont.insert_or_assign(std::forward<K>(k), std::forward<Args>(args)...); }
          { cont.insert_or_assign(std::forward<K>(k), std::forward<Args>(args)...); }};
}

template<typename It>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(random_access_iterator_arg_t, It it)
noexcept(std::is_nothrow_copy_constructible_v<It>)
requires std::is_copy_constructible_v        <It>
      && std::random_access_iterator<It>
{
  return std::pair{
      [it]
          (std::iter_difference_t<It> idx)
          noexcept(noexcept(*(it + idx)))
          -> decltype(auto)
          { return *(it + idx); },
      [it]
          <typename V>
          (std::iter_difference_t<It> idx, V &&v)
          noexcept(noexcept(*(it + idx) = std::forward<V>(v)))
          requires std::indirectly_writable<It, V &&>
          { *(it + idx) = std::forward<V>(v); }};
}

template<typename It, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(random_access_iterator_arg_t, It it, IndexMap imap)
noexcept(std::is_nothrow_copy_constructible_v<IndexMap>
      && std::is_nothrow_copy_constructible_v<It>)
requires std::is_copy_constructible_v<IndexMap>
      && std::is_copy_constructible_v<It>
      && std::random_access_iterator <It>
{
  return std::pair{
      [it, imap]
          <typename K>
          (K &&k)
          noexcept(noexcept(  *(it + properties::get(imap, std::forward<K>(k)))))
          -> decltype(auto)
          requires requires { *(it + properties::get(imap, std::forward<K>(k))); }
          { return *(it + properties::get(imap, std::forward<K>(k))); },
      [it, imap]
          <typename K, typename V>
          (K &&k, V &&v)
          noexcept(noexcept(  *(it + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v)))
          requires requires { *(it + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v); }
          { *(it + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v); }};
}

template<typename R>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(random_access_range_arg_t, R &rg)
requires std::ranges::random_access_range<R>
{
  return std::pair{
      [&rg]
          (std::ranges::range_difference_t<R> n)
          noexcept(noexcept(*(std::ranges::begin(rg) + n)))
          -> decltype(auto)
          { return *(std::ranges::begin(rg) + n); },
      [&rg]
          <typename V>
          (std::ranges::range_difference_t<R> n, V &&v)
          noexcept(noexcept(*(std::ranges::begin(rg) + n) = std::forward<V>(v)))
          requires std::indirectly_writable<std::ranges::iterator_t<R>, V &&>
          { return *(std::ranges::begin(rg) + n) = std::forward<V>(v); }};
}

template<typename R, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(random_access_range_arg_t, R &rg, IndexMap imap)
noexcept(std::is_nothrow_copy_constructible_v<IndexMap>)
requires std::is_copy_constructible_v        <IndexMap>
      && std::ranges::random_access_range<R>
{
  return std::pair{
      [&rg, imap]
          <typename K>
          (K &&k)
          noexcept(noexcept(  *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k)))))
          -> decltype(auto)
          requires requires { *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k))); }
          { return *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k))); },
      [&rg, imap]
          <typename K, typename V>
          (K &&k, V &&v)
          noexcept(noexcept(  *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v)))
          requires requires { *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v); }
          { return *(std::ranges::begin(rg) + properties::get(imap, std::forward<K>(k))) = std::forward<V>(v); }};
}


template<typename M, typename C>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(member_arg_t, M C::*m)
noexcept
{
  return std::pair{
      [m]
          <typename K>
          (K &&k)
          noexcept(noexcept(  std::invoke(m, std::forward<K>(k))))
          -> decltype(auto)
          requires requires { std::invoke(m, std::forward<K>(k)); }
          { return std::invoke(m, std::forward<K>(k)); },
      [m]
          <typename K, typename V>
          (K &&k, V &&v)
          noexcept(noexcept(  std::invoke(m, std::forward<K>(k)) = std::forward<V>(v)))
          requires requires { std::invoke(m, std::forward<K>(k)) = std::forward<V>(v); }
          { return std::invoke(m, std::forward<K>(k)) = std::forward<V>(v); }};
}

[[nodiscard]] constexpr
auto
make_property_map_lambdas(pointer_arg_t)
noexcept
{
  return std::pair{
      []
          <typename K>
          (K &&k)
          noexcept(noexcept(  *std::forward<K>(k)))
          -> decltype(auto)
          requires requires { *std::forward<K>(k); }
          { return *std::forward<K>(k); },
      []
          <typename K, typename V>
          (K &&k, V &&v)
          noexcept(noexcept(  *std::forward<K>(k) = std::forward<V>(v)))
          requires requires { *std::forward<K>(k) = std::forward<V>(v); }
          { *std::forward<K>(k) = std::forward<V>(v); }};
}

template<typename F>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(invocable_arg_t, F f)
noexcept(std::is_nothrow_copy_constructible_v<F>)
requires std::is_copy_constructible_v        <F>
{
  return std::pair{
      [f]
          <typename K>
          (K &&k)
          noexcept(noexcept(  std::invoke(f, std::forward<K>(k))))
          -> decltype(auto)
          requires requires { std::invoke(f, std::forward<K>(k)); }
          { return std::invoke(f, std::forward<K>(k)); },
      [f]
          <typename K, typename V>
          (K &&k, V &&v)
          noexcept(noexcept(  std::invoke(f, std::forward<K>(k)) = std::forward<V>(v)))
          requires requires { std::invoke(f, std::forward<K>(k)) = std::forward<V>(v); }
          { return std::invoke(f, std::forward<K>(k)) = std::forward<V>(v); }};
}

template<typename FMap, typename GMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(composable_arg_t, FMap f, GMap g)
noexcept(std::is_nothrow_copy_constructible_v<FMap>
      && std::is_nothrow_copy_constructible_v<GMap>)
requires std::is_copy_constructible_v<FMap>
      && std::is_copy_constructible_v<GMap>
{
  return std::pair{
      [f, g]
          <typename K>
          (K &&k)
          noexcept(noexcept(  properties::get(f, properties::get(g, std::forward<K>(k)))))
          -> decltype(auto)
          requires requires { properties::get(f, properties::get(g, std::forward<K>(k))); }
          { return properties::get(f, properties::get(g, std::forward<K>(k))); },
      [f, g]
          <typename K, typename ...Args>
          (K &&k, Args &&...args)
          noexcept(noexcept(  properties::set(f, properties::get(g, std::forward<K>(k)), std::forward<Args>(args)...)))
          requires requires { properties::set(f, properties::get(g, std::forward<K>(k)), std::forward<Args>(args)...); }
          { return properties::set(f, properties::get(g, std::forward<K>(k)), std::forward<Args>(args)...); }};
}

template<typename K, typename T, typename Cmp, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::map<K, T, Cmp, Alloc> &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename K, typename T, typename Cmp, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::map<K, T, Cmp, Alloc> const &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename K, typename T, typename Hash, typename KeyEqual, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::unordered_map<K, T, Hash, KeyEqual, Alloc> &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename K, typename T, typename Hash, typename KeyEqual, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::unordered_map<K, T, Hash, KeyEqual, Alloc> const &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename K, typename T, typename Cmp, typename KCont, typename MCont>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::flat_map<K, T, Cmp, KCont, MCont> &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename K, typename T, typename Cmp, typename KCont, typename MCont>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::flat_map<K, T, Cmp, KCont, MCont> const &cont)
noexcept(noexcept(make_property_map_lambdas(associative_container_arg, cont)))
{ return make_property_map_lambdas(associative_container_arg, cont); }

template<typename It>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(It it)
noexcept(noexcept(make_property_map_lambdas(random_access_iterator_arg, it)))
requires std::random_access_iterator<It>
{ return make_property_map_lambdas(random_access_iterator_arg, it); }

template<typename It, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(It it, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_iterator_arg, it, imap)))
requires std::random_access_iterator<It>
{ return make_property_map_lambdas(random_access_iterator_arg, it, imap); }

template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::span<T, N> cont)
noexcept(noexcept(make_property_map_lambdas(random_access_iterator_arg, cont.begin())))
{ return make_property_map_lambdas(random_access_iterator_arg, cont.begin()); }

template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::span<T, N> cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_iterator_arg, cont.begin(), imap)))
{ return make_property_map_lambdas(random_access_iterator_arg, cont.begin(), imap); }

template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::array<T, N> &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::array<T, N> const &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::array<T, N> &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::array<T, N> const &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(T (&cont)[N])
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(T const (&cont)[N])
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(T (&cont)[N], IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(T const (&cont)[N], IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::vector<T, Alloc> &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::vector<T, Alloc> const &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::vector<T, Alloc> &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::vector<T, Alloc> const &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::deque<T, Alloc> &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::deque<T, Alloc> const &cont)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont)))
{ return make_property_map_lambdas(random_access_range_arg, cont); }

template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::deque<T, Alloc> &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(std::deque<T, Alloc> const &cont, IndexMap imap)
noexcept(noexcept(make_property_map_lambdas(random_access_range_arg, cont, imap)))
{ return make_property_map_lambdas(random_access_range_arg, cont, imap); }

template<typename F>
[[nodiscard]] constexpr
auto
make_property_map_lambdas(F &&f)
noexcept(noexcept(make_property_map_lambdas(invocable_arg, std::forward<F>(f))))
requires std::is_function_v<std::remove_reference_t<F>>
{ return make_property_map_lambdas(invocable_arg, std::forward<F>(f)); }

} // namespace detail


template<typename ...Args>
[[nodiscard]] constexpr
auto
make_property_map(Args &&...args)
{
  auto lambdas = detail::make_property_map_lambdas(std::forward<Args>(args)...);
  return property_map{std::move(lambdas.first), std::move(lambdas.second)};
}

template<typename ...Args>
[[nodiscard]] constexpr
auto
make_readable_property_map(Args &&...args)
{
  auto lambdas = detail::make_property_map_lambdas(std::forward<Args>(args)...);
  return property_map{std::move(lambdas.first), no_property_setter};
}

template<typename ...Args>
[[nodiscard]] constexpr
auto
make_writable_property_map(Args &&...args)
{
  auto lambdas = detail::make_property_map_lambdas(std::forward<Args>(args)...);
  return property_map{no_property_getter, std::move(lambdas.second)};
}

} // namespace heim

#endif // HEIM_PROPERTY_PROPERTY_MAP_HPP
