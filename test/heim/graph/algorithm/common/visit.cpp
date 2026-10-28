#include <heim/graph/algorithm/visit.hpp>

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
namespace hg = heim::graphs;

template <typename Fn>
concept visit_invocable = requires(Fn &&fn) { hg::detail::invoke_visit(std::forward<Fn>(fn)); };

struct void_visitor
{
  int *calls;

  constexpr void operator()() const noexcept { ++*calls; }
};

struct enum_visitor
{
  hg::visit control;

  constexpr hg::visit operator()() const noexcept { return control; }
};

struct invalid_visitor
{
  int operator()() const noexcept { return 1; }
};

struct rvalue_visitor
{
  hg::visit operator()() & = delete;
  hg::visit operator()() && noexcept { return hg::visit::skip; }
};

struct lvalue_visitor
{
  hg::visit operator()() & noexcept { return hg::visit::prune; }
  hg::visit operator()() && = delete;
};

static_assert(visit_invocable<void_visitor>);
static_assert(visit_invocable<enum_visitor>);
static_assert(visit_invocable<lvalue_visitor &>);
static_assert(visit_invocable<rvalue_visitor>);
static_assert(!visit_invocable<invalid_visitor>);
static_assert(!visit_invocable<lvalue_visitor>);
static_assert(!visit_invocable<rvalue_visitor &>);
static_assert(std::same_as<decltype(hg::detail::invoke_visit(enum_visitor{})), hg::visit>);
static_assert(std::same_as<decltype(hg::detail::invoke_visit(void_visitor{})), hg::visit>);
static_assert(hg::detail::invoke_visit(enum_visitor{hg::visit::stop}) == hg::visit::stop);
}

TEST_CASE("invoke_visit returns proceed for a void callback", "[graph][algorithm][common][visit]")
{
  int calls{};
  REQUIRE(hg::detail::invoke_visit(void_visitor{&calls}) == hg::visit::proceed);
  REQUIRE(calls == 1);
}

TEST_CASE("invoke_visit preserves each control value", "[graph][algorithm][common][visit]")
{
  for (auto control : {hg::visit::proceed, hg::visit::prune, hg::visit::skip, hg::visit::stop})
    REQUIRE(hg::detail::invoke_visit(enum_visitor{control}) == control);
}

TEST_CASE("invoke_visit preserves callback value categories", "[graph][algorithm][common][visit]")
{
  lvalue_visitor first;
  REQUIRE(hg::detail::invoke_visit(first) == hg::visit::prune);
  REQUIRE(hg::detail::invoke_visit(rvalue_visitor{}) == hg::visit::skip);
}

TEST_CASE("invoke_visit forwards exception from a void visitor", "[graph][algorithm][common][visit]")
{
  REQUIRE_THROWS_AS(hg::detail::invoke_visit([]() -> void { throw std::runtime_error{"void visitor"}; }),
                    std::runtime_error);
}

TEST_CASE("invoke_visit forwards exception from a control visitor", "[graph][algorithm][common][visit]")
{
  REQUIRE_THROWS_AS(hg::detail::invoke_visit([]() -> hg::visit { throw std::runtime_error{"visitor"}; }),
                    std::runtime_error);
}

TEST_CASE("invoke_visit accepts a callable passed by reference", "[graph][algorithm][common][visit]")
{
  int calls{};
  auto callback = [&calls]() { ++calls; };
  REQUIRE(hg::detail::invoke_visit(std::ref(callback)) == hg::visit::proceed);
  REQUIRE(calls == 1);
}
