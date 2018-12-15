
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct lhs_> lhs;
    inline constexpr keyword<struct rhs_> rhs;
}

template <class ...Args>
constexpr auto add(Args &&...args) {
    auto [lhs, rhs] = match(
        parameter(keywords::lhs),
        parameter(keywords::rhs),
        std::forward<Args>(args)...
    );
    return lhs + rhs;
}

int main() {
    using namespace keywords;
    static_assert(add(lhs = 3, rhs = 4) == 7);
}
