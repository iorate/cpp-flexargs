
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "common.hpp"
using namespace flexargs;

int main() {
    {
        auto e = match(
            parameter(keywords::x) = 1,
            parameter<int>(keywords::y)
        );
        static_assert(is_instance_of<detail::syntax_error<detail::non_default_parameter_after_default_parameter<keywords::y_>>>(e));
    }
    {
        auto e = match(
            keyword_parameter(keywords::x),
            parameter(keywords::y)
        );
        static_assert(is_instance_of<detail::syntax_error<detail::non_keyword_parameter_after_keyword_parameter<keywords::y_>>>(e));
    }
    {
        auto e = match(
            parameter<std::is_integral>(keywords::x) = 3,
            keyword_parameter<int>(keywords::x)
        );
        static_assert(is_instance_of<detail::syntax_error<detail::duplicate_parameter<keywords::x_>>>(e));
    }
    {
        auto e = match(
            keyword_parameter<std::is_integral>(keywords::x),
            keyword_parameter(keywords::y) = 4,
            keywords::x = 5,
            6
        );
        static_assert(is_instance_of<detail::syntax_error<detail::non_keyword_argument_after_keyword_argument>>(e));
    }
    {
        auto e = match(
            keyword_parameter<int>(keywords::x) = 7,
            keywords::x = 8,
            keywords::x = 9
        );
        static_assert(is_instance_of<detail::syntax_error<detail::duplicate_argument<keywords::x_>>>(e));
    }
    {
        auto e = match(
            parameter<int>(keywords::x) = 7,
            8,
            keywords::x = 9
        );
        static_assert(is_instance_of<detail::syntax_error<detail::duplicate_argument<keywords::x_>>>(e));
    }
    {
        auto e = match(
            parameter(keywords::x)
        );
        static_assert(is_instance_of<detail::syntax_error<detail::missing_argument<keywords::x_>>>(e));
    }
    {
        auto e = match(
            15
        );
        static_assert(is_instance_of<detail::syntax_error<detail::extra_non_keyword_argument>>(e));
    }
}
