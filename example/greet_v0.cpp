
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string_view>

void greet_v0(std::string_view name, std::ostream &out = std::cout) {
    out << "Hello, " << name << "!\n";
}

int main() {
    greet_v0("World");
}
