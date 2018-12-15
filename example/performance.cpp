
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Reference: http://www.kmonos.net/alang/boost/classes/program_options.html

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <boost/timer/timer.hpp>
#include "../flexargs.hpp"
using namespace flexargs;

int calc_v0(std::string_view op, int lhs = 100, int rhs = 200) {
    if (op == "add") {
        return lhs + rhs;
    } else if (op == "sub") {
        return lhs - rhs;
    } else if (op == "mul") {
        return lhs * rhs;
    } else if (op == "div") {
        if (rhs == 0) {
            throw std::invalid_argument("division by zero");
        } else {
            return lhs / rhs;
        }
    } else {
        throw std::invalid_argument("bad operator");
    }
}

namespace keywords {
    constexpr keyword<struct op_> op;
    constexpr keyword<struct lhs_> lhs;
    constexpr keyword<struct rhs_> rhs;
}

template <class ...Args>
int calc_v1(Args &&...args) {
    auto [op, lhs, rhs] = match(
        parameter<std::string_view>(keywords::op),
        parameter<int>(keywords::lhs) = 100,
        parameter<int>(keywords::rhs) = 200,
        std::forward<Args>(args)...
    );
    if (op == "add") {
        return lhs + rhs;
    } else if (op == "sub") {
        return lhs - rhs;
    } else if (op == "mul") {
        return lhs * rhs;
    } else if (op == "div") {
        if (rhs == 0) {
            throw std::invalid_argument("division by zero");
        } else {
            return lhs / rhs;
        }
    } else {
        throw std::invalid_argument("bad operator");
    }
}

int main() {
    using namespace keywords;
    constexpr int N = 100000000;
    {
        std::cout << "call calc_v0() " << N << " times:\n";
        boost::timer::auto_cpu_timer timer;
        for (int i = 0; i < N; ++i) {
            calc_v0("sub", 999);
        }
    }
    {
        std::cout << "call calc_v1() " << N << " times:\n";
        boost::timer::auto_cpu_timer timer;
        for (int i = 0; i < N; ++i) {
            calc_v1(op = "sub", lhs = 999);
        }
    }
}

/*
$ g++ -std=c++17 -O2 performance.cpp -lboost_timer -o performance

$ ./performance
call calc_v0() 100000000 times:
 0.284747s wall, 0.296875s user + 0.000000s system = 0.296875s CPU (104.3%)
call calc_v1() 100000000 times:
 0.729618s wall, 0.718750s user + 0.000000s system = 0.718750s CPU (98.5%)
*/
