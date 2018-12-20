# cpp-flexargs
Implements keyword arguments in C++17

## Description
This single-header library makes it possible to write functions that accept keyword arguments. It requires C++17 features.

## Example
draw_text.cpp
``` cpp
// Reference: http://www.kmonos.net/alang/boost/classes/parameter.html

#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include "../flexargs.hpp"
using namespace std::literals;
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct x_> x;
    inline constexpr keyword<struct y_> y;
    inline constexpr keyword<struct msg_> msg;
    inline constexpr keyword<struct width_> width;
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
```
```
$ g++ -std=c++17 draw_text.cpp -o draw_text

$ ./draw_text
(   1,   2) : Hello
(       3,       4) : World
( 5, 6) : GoodBye
```

## Tutorial
Begin with a simple greeting function.

greet_v0.cpp
```cpp
#include <iostream>
#include <string_view>

void greet_v0(std::string_view name, std::ostream &out = std::cout) {
    out << "Hello, " << name << "!\n";
}

int main() {
    greet_v0("World");
}
```
Make this function accept keyword arguments using cpp-flexargs.

greet_v1.cpp
```cpp
#include <iostream>
#include <utility>
#include "../flexargs.hpp"                        // Include the header file.
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct name_> name;  // Define a keyword.
    inline constexpr keyword<struct out_> out;    // A template argument should be a unique type.
}

template <class ...Args>                          // In Python:
void greet_v1(Args &&...args) {                   // def greet_v1(name, out = std.cout):
    auto [name, out] = match(
        parameter(keywords::name),                // Declare a parameter.
        parameter(keywords::out) = std::cout,     // A parameter can have a default value.
        std::forward<Args>(args)...
    );
    out << "Hello, " << name << "!\n";
}

int main() {
    using namespace keywords;
    greet_v1("World");
    greet_v1(out = std::cerr, name = "World");    // A parameter can also be specified by a keyword.
}
```
You can declare typed parameters and keyword-only parameters.

greet_v2.cpp
```cpp
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
```

## Error Messages
cpp-flexargs yields clear error messages.

compile_error.cpp
```cpp
#include <iostream>
#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct x_> x;
    inline constexpr keyword<struct y_> y;
}

template <class ...Args>                          // In Python:
void f(Args &&...args) {                          // def f(x, y):
    auto [x, y] = match(
        parameter(keywords::x),
        parameter(keywords::y),
        std::forward<Args>(args)...
    );
    std::cout << "x = " << x << "\n";
    std::cout << "y = " << y << "\n";
}

int main() {
    using namespace keywords;
    f(1, 2, x = 3);
}
```
The function call `f(1, 2, x = 3)` is invalid because `x` is specified twice.

GCC 8.2.0 and Clang 7.0.0 show the following error messages.
```
$ g++ -std=c++17 compile_error.cpp
compile_error.cpp: In instantiation of 'void f(Args&& ...) [with Args = {int, int, flexargs::detail::keyword_argument<keywords::x_, int>}]':
compile_error.cpp:32:18:   required from here
compile_error.cpp:21:10: error: cannot decompose class type 'flexargs::detail::syntax_error<flexargs::detail::duplicate_argument<keywords::x_> >' without non-static data members
     auto [x, y] = match(
          ^~~~~~

$ clang++ -std=c++17 compile_error.cpp
compile_error.cpp:21:10: error: type 'flexargs::detail::syntax_error<flexargs::detail::duplicate_argument<keywords::x_> >' decomposes into 0 elements, but 2 names were provided
    auto [x, y] = match(
         ^
compile_error.cpp:32:5: note: in instantiation of function template specialization 'f<int, int, flexargs::detail::keyword_argument<keywords::x_, int> >' requested here
    f(1, 2, x = 3);
    ^
1 error generated.
```
Pay attention to the type `flexargs::detail::syntax_error<flexargs::detail::duplicate_argument<keywords::x_> >`, which represents the error message.

## Performance
The implementation of cpp-flexargs consists of many function calls, but most of which can be omitted by optimization.

Here is an example that calls a normal function and a function accepting keyword arguments 100'000'000 times.

performance.cpp
```cpp
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
    inline constexpr keyword<struct op_> op;
    inline constexpr keyword<struct lhs_> lhs;
    inline constexpr keyword<struct rhs_> rhs;
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
    constexpr int N = 100'000'000;
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
```
```
$ g++ -std=c++17 -O2 performance.cpp -lboost_timer -o performance

$ ./performance
call calc_v0() 100000000 times:
 0.284747s wall, 0.296875s user + 0.000000s system = 0.296875s CPU (104.3%)
call calc_v1() 100000000 times:
 0.729618s wall, 0.718750s user + 0.000000s system = 0.718750s CPU (98.5%)
```
When I compiled it by GCC 8.2.0 with '-O2' flag and executed it in my environment, the extra execution time was approximately 0.45s. That means the overhead per call was 4.5ns, which is probably an acceptable value in most cases.

## Constant Expressions
cpp-flexargs is constexpr-friendly.

constexpr.cpp
```cpp
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
```
Unfortunately, the above example cannot be successfully compiled by MSVC 15.9.3.

## SFINAE
To raise substitution failure when invalid arguments are passed, use `match_()` in a function signature instead of `match()`.

sfinae.cpp
``` cpp
#include <tuple>
#include <type_traits>
#include <utility>
#include "../flexargs.hpp"
using namespace flexargs;

namespace keywords {
    inline constexpr keyword<struct lhs_> lhs;
    inline constexpr keyword<struct rhs_> rhs;
}

template <class ...Args>
constexpr auto multiply_v1(Args &&...args) {
    auto [lhs, rhs] = match(
        parameter<std::is_arithmetic>(keywords::lhs),  // A parameter can have a type constraint.
        parameter<std::is_arithmetic>(keywords::rhs),
        std::forward<Args>(args)...
    );
    return lhs * rhs;
}

template <
    class ...Args,
    class Params = decltype(match_(                    // A result type of 'match_()' is a tuple of parameter types.
        parameter<std::is_arithmetic>(keywords::lhs),  // You can use it for further validation of parameter types.
        parameter<std::is_arithmetic>(keywords::rhs),
        std::declval<Args>()...
    ))
>
constexpr auto multiply_v2(Args &&...args) {
    auto [lhs, rhs] = match(
        parameter<std::is_arithmetic>(keywords::lhs),
        parameter<std::is_arithmetic>(keywords::rhs),
        std::forward<Args>(args)...
    );
    return lhs * rhs;
}

int main() {
    using namespace keywords;
    static_assert(multiply_v1(lhs = 3, rhs = 4) == 12);
    static_assert(multiply_v1(lhs = 3, rhs = "4") == 12);
    static_assert(multiply_v2(lhs = 3, rhs = 4) == 12);
    static_assert(multiply_v2(lhs = 3, rhs = "4") == 12);
}
```
```
$ g++ sfinae.cpp
sfinae.cpp: In instantiation of 'constexpr auto multiply_v1(Args&& ...) [with Args = {flexargs::detail::keyword_argument<keywords::lhs_, int>, flexargs::detail::keyword_argument<keywords::rhs_, const char (&)[2]>}]':
sfinae.cpp:52:49:   required from here
sfinae.cpp:22:10: error: cannot decompose class type 'flexargs::detail::type_error<flexargs::detail::not_satisfying<const char (&)[2], std::is_arithmetic> >' without non-static data members
     auto [lhs, rhs] = match(
          ^~~~~~~~~~
...
...
...
sfinae.cpp:54:49: error: no matching function for call to 'multiply_v2(flexargs::detail::keyword_argument<keywords::lhs_, int>, flexargs::detail::keyword_argument<keywords::rhs_, const char (&)[2]>)'
     static_assert(multiply_v2(lhs = 3, rhs = "4") == 12);
                                                 ^
sfinae.cpp:40:16: note: candidate: 'template<class ... Args, class Params> constexpr auto multiply_v2(Args&& ...)'
 constexpr auto multiply_v2(Args &&...args) {
                ^~~~~~~~~~~
sfinae.cpp:40:16: note:   template argument deduction/substitution failed:
...
...
...
```

## Author
[iorate](https://github.com/iorate) ([Twitter](https://twitter.com/iorate))

## License
cpp-flexargs is licensed under [Boost Software License](LICENSE_1_0.txt).
