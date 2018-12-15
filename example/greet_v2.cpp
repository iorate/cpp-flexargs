
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string_view>
#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct name_> name;
    inline constexpr keyword<struct out_> out;
}

template <class ...Args>                          // In Python:
void greet_v2(Args &&...args) {                   // def greet_v2(name: std.string_view, *, out: std.ostream = std.cout) -> None:
    auto [name, out] = match(
        parameter<std::string_view>(keywords::name),
                                                  // A parameter can have a type.
        keyword_parameter<std::ostream &>(keywords::out) = std::cout,
                                                  // Declare a keyword-only parameter.
                                                  // It can be specified only by a keyword.
        std::forward<Args>(args)...
    );
    out << "Hello, " << name << "!\n";
}

int main() {
    using namespace keywords;
    greet_v2("World");
    greet_v2(42);                                 // Error: 'int' is not convertible to 'std::string_view'.
    greet_v2("World", out = std::cerr);
    greet_v2("World", std::cerr);                 // Error: extra argument 'std::cerr'.
}
