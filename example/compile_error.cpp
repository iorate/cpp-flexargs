
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    constexpr keyword<struct x_> x;
    constexpr keyword<struct y_> y;
}

template <class ...Args>                          // In Python:
void f(Args &&...args) {                          // def f(x, y):
    auto [x, y] = match(
        parameter(keywords::x),
        parameter(keywords::y),
        std::forward<Args>(args)...);
    std::cout << "x = " << x << "\n";
    std::cout << "y = " << y << "\n";
}

int main() {
    using namespace keywords;
    f(1, 2, x = 3);
}

/*
$ g++ -std=c++17 compile_error.cpp
compile_error.cpp: In instantiation of 'void f(Args&& ...) [with Args = {int, int, flexargs::detail::keyword_argument<keywords::x_, int>}]':
compile_error.cpp:31:18:   required from here
compile_error.cpp:21:10: error: cannot decompose class type 'flexargs::detail::syntax_error<flexargs::detail::duplicate_argument<keywords::x_> >' without non-static data members
     auto [x, y] = match(
          ^~~~~~

$ clang++ -std=c++17 compile_error.cpp
compile_error.cpp:21:10: error: type 'flexargs::detail::syntax_error<flexargs::detail::duplicate_argument<keywords::x_> >' decomposes into 0 elements, but 2 names were provided
    auto [x, y] = match(
         ^
compile_error.cpp:31:5: note: in instantiation of function template specialization 'f<int, int, flexargs::detail::keyword_argument<keywords::x_, int> >' requested here
    f(1, 2, x = 3);
    ^
1 error generated.
*/
