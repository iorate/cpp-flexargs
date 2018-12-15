
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "common.hpp"
using namespace std::literals;
using namespace flexargs;

struct nonmovable {
    nonmovable() = default;
    nonmovable(nonmovable &&) = delete;
};

int main() {
    {
        auto e = match(
            parameter<int>(keywords::x),
            "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_convertible<std::string, int>>>(e));
    }
    {
        auto e = match(
            parameter<std::is_integral>(keywords::x),
            keywords::x = "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_satisfying<std::string, std::is_integral>>>(e));
    }
    {
        auto e = match(
            parameter<int>(keywords::x) = 2,
            "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_convertible<std::string, int>>>(e));
    }
    {
        auto e = match(
            parameter<std::is_integral>(keywords::x) = "world"s,
            keywords::x = 3);
        static_assert(is_instance_of<detail::type_error<detail::not_satisfying<std::string, std::is_integral>>>(e));
    }
    {
        auto e = match(
            keyword_parameter<int>(keywords::x),
            keywords::x = "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_convertible<std::string, int>>>(e));
    }
    {
        auto e = match(
            keyword_parameter<std::is_integral>(keywords::x),
            keywords::x = "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_satisfying<std::string, std::is_integral>>>(e));
    }
    {
        auto e = match(
            keyword_parameter<int>(keywords::x) = 2,
            keywords::x = "world"s);
        static_assert(is_instance_of<detail::type_error<detail::not_convertible<std::string, int>>>(e));
    }
    {
        auto e = match(
            keyword_parameter<std::is_integral>(keywords::x) = "world"s,
            keywords::x = 3);
        static_assert(is_instance_of<detail::type_error<detail::not_satisfying<std::string, std::is_integral>>>(e));
    }
    {
        auto e = match(
            parameter(keywords::x) = nonmovable());
        static_assert(is_instance_of<detail::type_error<detail::not_movable<nonmovable>>>(e));
    }
    {
        auto e = match(
            parameter<std::is_integral>(keywords::x) = nonmovable());
        static_assert(is_instance_of<detail::type_error<detail::not_satisfying<nonmovable, std::is_integral>>>(e));
    }
}
