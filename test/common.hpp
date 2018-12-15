
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <type_traits>
#include <utility>
#include "../flexargs.hpp"

namespace keywords {
    constexpr flexargs::keyword<struct x_> x;
    constexpr flexargs::keyword<struct y_> y;
    constexpr flexargs::keyword<struct z_> z;
    constexpr flexargs::keyword<struct w_> w;
}

template <class T, class U>
constexpr bool is_instance_of(U const &) {
    return std::is_same_v<T, U>;
}
