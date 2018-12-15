
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "common.hpp"
#include <boost/core/lightweight_test.hpp>
using namespace std::literals;
using namespace flexargs;

template <class ...Args>
void f(Args &&...args) {
    auto &&[x, y, z, w] = match(
        parameter(keywords::x),
        parameter<int>(keywords::y) = 2,
        keyword_parameter<std::is_integral>(keywords::z),
        keyword_parameter(keywords::w) = 4,
        std::forward<Args>(args)...);
    BOOST_TEST_EQ(x, 1);
    BOOST_TEST_EQ(y, 2);
    BOOST_TEST_EQ(z, 3);
    BOOST_TEST_EQ(w, 4);
}

int main() {
    using namespace keywords;
    f(1, z = 3);
    f(1, 2, z = 3, w = 4);
    f(x = 1, y = 2, z = 3, w = 4);
    f(w = 4, z = 3, y = 2, x = 1);
    return boost::report_errors();
}
