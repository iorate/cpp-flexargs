
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <utility>
#include "../flexargs.hpp"                        // Include the header file.
using namespace flexargs;

namespace keywords {
    constexpr keyword<struct name_> name;         // Define a keyword.
    constexpr keyword<struct out_> out;           // A template argument should be a unique type.
}

template <class ...Args>                          // In Python:
void greet_v1(Args &&...args) {                   // def greet_v1(name, out = std.cout):
    auto [name, out] = match(
        parameter(keywords::name),                // Declare a parameter.
        parameter(keywords::out) = std::cout,     // A parameter can have a default value.
        std::forward<Args>(args)...);
    out << "Hello, " << name << "!\n";
}

int main() {
    using namespace keywords;
    greet_v1("World");
    greet_v1(out = std::cerr, name = "World");    // A parameter can also be specified by a keyword.
}
