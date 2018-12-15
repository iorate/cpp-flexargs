
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Reference: http://www.kmonos.net/alang/boost/classes/parameter.html

#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include "../flexargs.hpp"
using namespace std::literals;
using namespace flexargs;

namespace keywords {
    constexpr keyword<struct x_> x;
    constexpr keyword<struct y_> y;
    constexpr keyword<struct msg_> msg;
    constexpr keyword<struct width_> width;
}

template <class ...Args>                          // In Python:
void draw_text(Args &&...args) {                  // def draw_text(x: int, y: int, msg: Any, *, width: int = 4) -> None:
    auto [x, y, msg, width] = match(
        parameter<int>(keywords::x),
        parameter<int>(keywords::y),
        parameter(keywords::msg),
        keyword_parameter<int>(keywords::width) = 4,
        std::forward<Args>(args)...
    );
    std::cout << "(" << std::setw(width) << x << "," << std::setw(width) << y << ") : " << msg << std::endl;
}

int main() {
    using namespace keywords;
    auto www = "World"s;
    draw_text(x = 1, y = 2, msg = "Hello");       // Specify parameters by keywords.
    draw_text(msg = www, x = 3, y = 4, width = 8);
    draw_text(5, 6, "GoodBye", width = 2);        // You can also specify parameters by positions.
}
