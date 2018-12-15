
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <tuple>
#include <type_traits>
#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct lhs_> lhs;
    inline constexpr keyword<struct rhs_> rhs;
}

template <class ...Args>
constexpr auto multiply_v1(Args &&...args) {
    auto [lhs, rhs] = match(
        parameter<std::is_arithmetic>(keywords::lhs),  // A parameter can have a type constraint.
        parameter<std::is_arithmetic>(keywords::rhs),
        std::forward<Args>(args)...
    );
    return lhs * rhs;
}

template <
    class ...Args,
    class Params = decltype(match_(                    // A result type of 'match_()' is a tuple of parameter types.
        parameter<std::is_arithmetic>(keywords::lhs),  // You can use it for further validation of parameter types.
        parameter<std::is_arithmetic>(keywords::rhs),
        std::declval<Args>()...
    ))
>
constexpr auto multiply_v2(Args &&...args) {
    auto [lhs, rhs] = match(
        parameter<std::is_arithmetic>(keywords::lhs),
        parameter<std::is_arithmetic>(keywords::rhs),
        std::forward<Args>(args)...
    );
    return lhs * rhs;
}

int main() {
    using namespace keywords;
    static_assert(multiply_v1(lhs = 3, rhs = 4) == 12);
    static_assert(multiply_v1(lhs = 3, rhs = "4") == 12);
    static_assert(multiply_v2(lhs = 3, rhs = 4) == 12);
    static_assert(multiply_v2(lhs = 3, rhs = "4") == 12);
}

/*
$ g++ sfinae.cpp
sfinae.cpp: In instantiation of 'constexpr auto multiply_v1(Args&& ...) [with Args = {flexargs::detail::keyword_argument<keywords::lhs_, int>, flexargs::detail::keyword_argument<keywords::rhs_, const char (&)[2]>}]':
sfinae.cpp:52:49:   required from here
sfinae.cpp:22:10: error: cannot decompose class type 'flexargs::detail::type_error<flexargs::detail::not_satisfying<const char (&)[2], std::is_arithmetic> >' without non-static data members
     auto [lhs, rhs] = match(
          ^~~~~~~~~~
...
...
...
sfinae.cpp:54:49: error: no matching function for call to 'multiply_v2(flexargs::detail::keyword_argument<keywords::lhs_, int>, flexargs::detail::keyword_argument<keywords::rhs_, const char (&)[2]>)'
     static_assert(multiply_v2(lhs = 3, rhs = "4") == 12);
                                                 ^
sfinae.cpp:40:16: note: candidate: 'template<class ... Args, class Params> constexpr auto multiply_v2(Args&& ...)'
...
...
...
*/
