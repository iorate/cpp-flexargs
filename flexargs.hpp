
// cpp-flexargs
//
// Copyright iorate 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLEXARGS_HPP_INCLUDED
#define FLEXARGS_HPP_INCLUDED

#if _MSVC_LANG < 201703L && __cplusplus < 201703L
#include "cpp-flexargs: the compiler does not support C++17"
#endif

#include <cstddef>
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>

namespace flexargs {

namespace detail {

/* Errors */

template <class Msg>
struct syntax_error {
#ifdef _MSC_VER
private:
    int _unused;
#endif
};

template <class Id>
struct non_default_parameter_after_default_parameter {};

template <class Id>
struct non_keyword_parameter_after_keyword_parameter {};

template <class Id>
struct duplicate_parameter {};

struct non_keyword_argument_after_keyword_argument {};

template <class Id>
struct duplicate_argument {};

template <class Id>
struct missing_argument {};

template <class Id>
struct extra_keyword_argument {};

struct extra_non_keyword_argument {};

template <class Msg>
struct type_error {
#ifdef _MSC_VER
private:
    int _unused;
#endif
};

template <class T>
struct not_movable {};

template <class T, class Type>
struct not_convertible {};

template <class T, template <class> class Constraint>
struct not_satisfying {};

template <class T>
struct is_error : std::false_type {};

template <class Msg>
struct is_error<syntax_error<Msg>> : std::true_type {};

template <class Msg>
struct is_error<type_error<Msg>> : std::true_type {};

template <class T>
inline constexpr bool is_error_v = is_error<T>::value;

/* Parameters */

template <class Id, class Pass, bool IsKeyword, class Def>
struct default_parameter {
    using id = Id;

    template <class T>
    using pass = typename Pass::template apply<T>;

    using pass_default = typename Pass::template apply_default<Def>;

    static constexpr bool is_keyword = IsKeyword;
    static constexpr bool is_default = true;

    Def &&_def;

    Def &&def() const && {
        return std::forward<Def>(_def);
    }
};

template <class Id, class Pass, bool IsKeyword>
struct parameter {
    using id = Id;

    template <class T>
    using pass = typename Pass::template apply<T>;

    static constexpr bool is_keyword = IsKeyword;
    static constexpr bool is_default = false;

    template <class Def>
    constexpr default_parameter<Id, Pass, IsKeyword, Def> operator=(Def &&def) const {
        return {std::forward<Def>(def)};
    }
};

template <class>
struct is_parameter : std::false_type {};

template <class Id, class Pass, bool IsKeyword>
struct is_parameter<parameter<Id, Pass, IsKeyword>> : std::true_type {};

template <class Id, class Pass, bool IsKeyword, class Def>
struct is_parameter<default_parameter<Id, Pass, IsKeyword, Def>> : std::true_type {};

template <class T>
inline constexpr bool is_parameter_v = is_parameter<T>::value;

struct forward_pass {
    template <class T>
    using apply = T &&;

    template <class T>
    using apply_default = std::conditional_t<
        !std::is_move_constructible_v<T>,
        type_error<not_movable<T>>,
        T
    >;
};

template <class Type>
struct convert_pass {
    template <class T>
    using apply = std::conditional_t<
        !std::is_convertible_v<T, Type>,
        type_error<not_convertible<T, Type>>,
        Type
    >;

    template <class T>
    using apply_default = apply<T>;
};

template <template <class> class Constraint>
struct validate_pass {
    template <class T>
    using apply = std::conditional_t<
        !Constraint<T>::value,
        type_error<not_satisfying<T, Constraint>>,
        T &&
    >;

    template <class T>
    using apply_default = std::conditional_t<
        !Constraint<T>::value,
        type_error<not_satisfying<T, Constraint>>,
        std::conditional_t<
            !std::is_move_constructible_v<T>,
            type_error<not_movable<T>>,
            T
        >
    >;
};

/* Arguments */

template <class Val>
struct argument {
    using value_type = Val;

    static constexpr bool is_keyword = false;

    Val &&_val;

    constexpr Val &&val() const && {
        return std::forward<Val>(_val);
    }
};

template <class Id, class Val>
struct keyword_argument {
    using id = Id;
    using value_type = Val;

    static constexpr bool is_keyword = true;

    Val &&_val;

    constexpr Val &&val() const && {
        return std::forward<Val>(_val);
    }
};

/* Splitters */

template <std::size_t N>
inline constexpr std::integral_constant<std::size_t, N> size_c = {};

template <class ParamsAndArgs, class Index>
constexpr auto find_non_param_i([[maybe_unused]] ParamsAndArgs &params_and_args, [[maybe_unused]] Index index) {
    if constexpr (Index::value >= std::tuple_size_v<ParamsAndArgs>) {
        return index;
    } else if constexpr (!is_parameter_v<std::remove_reference_t<std::tuple_element_t<Index::value, ParamsAndArgs>>>) {
        return index;
    } else {
        return find_non_param_i(params_and_args, size_c<Index::value + 1>);
    }
}

template <class ParamsAndArgs>
constexpr auto find_non_param(ParamsAndArgs &&params_and_args) {
    return find_non_param_i(params_and_args, size_c<0>);
}

template <class Val>
constexpr argument<Val> to_arg(Val &&val) {
    return {std::forward<Val>(val)};
}

template <class Id, class Val>
constexpr keyword_argument<Id, Val> to_arg(keyword_argument<Id, Val> &&arg) {
    return std::move(arg);
}

template <class ParamsAndArgs, std::size_t ...ParamIndices, std::size_t ...ArgIndices>
constexpr auto split_params_and_args_i(ParamsAndArgs &&params_and_args, std::index_sequence<ParamIndices...>, std::index_sequence<ArgIndices...>) {
    return std::make_tuple(
        std::make_tuple(std::get<ParamIndices>(std::move(params_and_args))...),
        std::make_tuple(to_arg(std::get<sizeof...(ParamIndices) + ArgIndices>(std::move(params_and_args)))...)
    );
}

template <class ParamsAndArgs>
constexpr auto split_params_and_args(ParamsAndArgs &&params_and_args) {
    auto index = find_non_param(params_and_args);
    return split_params_and_args_i(
        std::move(params_and_args),
        std::make_index_sequence<decltype(index)::value>(),
        std::make_index_sequence<std::tuple_size_v<ParamsAndArgs> - decltype(index)::value>()
    );
}

/* Validators */

template <class Params, class Pred, class Index>
constexpr auto validate_each_i([[maybe_unused]] Params &params, [[maybe_unused]] Pred &pred, Index) {
    if constexpr (Index::value >= std::tuple_size_v<Params>) {
        return nullptr;
    } else {
        auto np = pred(std::get<Index::value>(params));
        if constexpr (is_error_v<decltype(np)>) {
            return np;
        } else {
            return validate_each_i(params, pred, size_c<Index::value + 1>);
        }
    }
}

template <class Params, class Pred>
constexpr auto validate_each(Params &&params, Pred &&pred) {
    return validate_each_i(params, pred, size_c<0>);
}

template <class Params, class Pred, class Index>
constexpr auto validate_each_adjacent_i([[maybe_unused]] Params &params, [[maybe_unused]] Pred &pred, Index) {
    if constexpr (Index::value + 1 >= std::tuple_size_v<Params>) {
        return nullptr;
    } else {
        auto np = pred(std::get<Index::value>(params), std::get<Index::value + 1>(params));
        if constexpr (is_error_v<decltype(np)>) {
            return np;
        } else {
            return validate_each_adjacent_i(params, pred, size_c<Index::value + 1>);
        }
    }
}

template <class Params, class Pred>
constexpr auto validate_each_adjacent(Params &&params, Pred &&pred) {
    return validate_each_adjacent_i(params, pred, size_c<0>);
}

template <class Params, class Pred, class Index1, class Index2>
constexpr auto validate_each_combination_i([[maybe_unused]] Params &params, [[maybe_unused]] Pred &pred, [[maybe_unused]] Index1 index1, Index2) {
    if constexpr (Index1::value >= std::tuple_size_v<Params>) {
        return nullptr;
    } else if constexpr (Index2::value >= std::tuple_size_v<Params>) {
        return validate_each_combination_i(params, pred, size_c<Index1::value + 1>, size_c<Index1::value + 2>);
    } else {
        auto np = pred(std::get<Index1::value>(params), std::get<Index2::value>(params));
        if constexpr (is_error_v<decltype(np)>) {
            return np;
        } else {
            return validate_each_combination_i(params, pred, index1, size_c<Index2::value + 1>);
        }
    }
}

template <class Params, class Pred>
constexpr auto validate_each_combination(Params &&params, Pred &&pred) {
    return validate_each_combination_i(params, pred, size_c<0>, size_c<1>);
}

template <class Params>
constexpr auto validate_params(Params &&params) {
    auto np1 = validate_each(params, [](auto &param) {
        using param_t = std::remove_reference_t<decltype(param)>;
        if constexpr (param_t::is_default) {
            using param_val_t = typename param_t::pass_default;
            if constexpr (is_error_v<param_val_t>) {
                return param_val_t();
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    });
    if constexpr (is_error_v<decltype(np1)>) {
        return np1;
    } else {
        auto np2 = validate_each_adjacent(params, [](auto &param1, auto &param2) {
            using param1_t = std::remove_reference_t<decltype(param1)>;
            using param2_t = std::remove_reference_t<decltype(param2)>;
            if constexpr (!param1_t::is_keyword) {
                if constexpr (!param1_t::is_default) {
                    return nullptr;
                } else {
                    if constexpr (!param2_t::is_keyword && !param2_t::is_default) {
                        return syntax_error<non_default_parameter_after_default_parameter<typename param2_t::id>>();
                    } else {
                        return nullptr;
                    }
                }
            } else {
                if constexpr (!param2_t::is_default) {
                    return syntax_error<non_keyword_parameter_after_keyword_parameter<typename param2_t::id>>();
                } else {
                    return nullptr;
                }
            }
        });
        if constexpr (is_error_v<decltype(np2)>) {
            return np2;
        } else {
            return validate_each_combination(params, [](auto &param1, auto &param2) {
                using param1_t = std::remove_reference_t<decltype(param1)>;
                using param2_t = std::remove_reference_t<decltype(param2)>;
                if constexpr (std::is_same_v<typename param1_t::id, typename param2_t::id>) {
                    return syntax_error<duplicate_parameter<typename param2_t::id>>();
                } else {
                    return nullptr;
                }
            });
        }
    }
}

template <class Args>
constexpr auto validate_args(Args &&args) {
    auto np = validate_each_adjacent(args, [](auto &arg1, auto &arg2) {
        using arg1_t = std::remove_reference_t<decltype(arg1)>;
        using arg2_t = std::remove_reference_t<decltype(arg2)>;
        if constexpr (arg1_t::is_keyword && !arg2_t::is_keyword) {
            return syntax_error<non_keyword_argument_after_keyword_argument>();
        } else {
            return nullptr;
        }
    });
    if constexpr (is_error_v<decltype(np)>) {
        return np;
    } else {
        return validate_each_combination(args, [](auto &arg1, auto &arg2) {
            using arg1_t = std::remove_reference_t<decltype(arg1)>;
            using arg2_t = std::remove_reference_t<decltype(arg2)>;
            if constexpr (arg1_t::is_keyword && arg2_t::is_keyword) {
                if constexpr (std::is_same_v<typename arg1_t::id, typename arg2_t::id>) {
                    return syntax_error<duplicate_argument<typename arg2_t::id>>();
                } else {
                    return nullptr;
                }
            } else {
                return nullptr;
            }
        });
    }
}

/* Matchers */

inline constexpr std::size_t npos = static_cast<std::size_t>(-1);

template <std::size_t>
constexpr auto initialize_param_ctors_ii() {
    return size_c<npos>;
}

template <std::size_t ...ParamIndices>
constexpr auto initialize_param_ctors_i(std::index_sequence<ParamIndices...>) {
    return std::make_tuple(initialize_param_ctors_ii<ParamIndices>()...);
}

template <class Params>
constexpr auto initialize_param_ctors(Params &&) {
    return initialize_param_ctors_i(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Params>>>());
}

template <class ParamCtors, class Params, class Index>
constexpr auto finalize_param_ctors_i([[maybe_unused]] ParamCtors param_ctors, [[maybe_unused]] Params &params, Index) {
    if constexpr (Index::value >= std::tuple_size_v<ParamCtors>) {
        return param_ctors;
    } else {
        using param_t = std::tuple_element_t<Index::value, Params>;
        using param_ctor = std::tuple_element_t<Index::value, ParamCtors>;
        if constexpr (!param_t::is_default && param_ctor::value == npos) {
            return syntax_error<missing_argument<typename param_t::id>>();
        } else {
            return finalize_param_ctors_i(param_ctors, params, size_c<Index::value + 1>);
        }
    }
}

template <class ParamCtors, class Params>
constexpr auto finalize_param_ctors(ParamCtors param_ctors, Params &&params) {
    return finalize_param_ctors_i(param_ctors, params, size_c<0>);
}

template <class ParamCtors, class ParamIndex, class ArgIndex, class CurParamIndex>
constexpr auto update_param_ctors_ii([[maybe_unused]] ParamCtors param_ctors, ParamIndex, [[maybe_unused]] ArgIndex arg_index, CurParamIndex) {
    if constexpr (CurParamIndex::value == ParamIndex::value) {
        return arg_index;
    } else {
        return std::get<CurParamIndex::value>(param_ctors);
    }
}

template <class ParamCtors, class ParamIndex, class ArgIndex, std::size_t ...ParamIndices>
constexpr auto update_param_ctors_i(ParamCtors param_ctors, ParamIndex param_index, ArgIndex arg_index, std::index_sequence<ParamIndices...>) {
    return std::make_tuple(update_param_ctors_ii(param_ctors, param_index, arg_index, size_c<ParamIndices>)...);
}

template <class ParamCtors, class ParamIndex, class ArgIndex>
constexpr auto update_param_ctors(ParamCtors param_ctors, ParamIndex param_index, ArgIndex arg_index) {
    return update_param_ctors_i(param_ctors, param_index, arg_index, std::make_index_sequence<std::tuple_size_v<ParamCtors>>());
}

template <class Params, class Arg, class Index>
constexpr auto find_keyword_param_i([[maybe_unused]] Params &params, [[maybe_unused]] Arg &arg, [[maybe_unused]] Index index) {
    if constexpr (Index::value >= std::tuple_size_v<Params>) {
        return syntax_error<extra_keyword_argument<typename Arg::id>>();
    } else {
        using param_t = std::tuple_element_t<Index::value, Params>;
        if constexpr (std::is_same_v<typename Arg::id, typename param_t::id>) {
            using param_val_t = typename param_t::template pass<typename Arg::value_type>;
            if constexpr (is_error_v<param_val_t>) {
                return param_val_t();
            } else {
                return index;
            }
        } else {
            return find_keyword_param_i(params, arg, size_c<Index::value + 1>);
        }
    }
}

template <class Params, class Arg>
constexpr auto find_keyword_param(Params &&params, Arg &&arg) {
    return find_keyword_param_i(params, arg, size_c<0>);
}

template <class Params, class ParamIndex, class Args, class ArgIndex, class ParamCtors>
constexpr auto match_params_and_args_i([[maybe_unused]] Params &params, [[maybe_unused]] ParamIndex param_index, [[maybe_unused]] Args &args, [[maybe_unused]] ArgIndex arg_index, [[maybe_unused]] ParamCtors param_ctors) {
    if constexpr (ArgIndex::value >= std::tuple_size_v<Args>) {
        return finalize_param_ctors(param_ctors, params);
    } else {
        using arg_t = std::tuple_element_t<ArgIndex::value, Args>;
        if constexpr (!arg_t::is_keyword) {
            if constexpr (ParamIndex::value >= std::tuple_size_v<Params>) {
                return syntax_error<extra_non_keyword_argument>();
            } else {
                using param_t = std::tuple_element_t<ParamIndex::value, Params>;
                if constexpr (!param_t::is_keyword) {
                    using param_val_t = typename param_t::template pass<typename arg_t::value_type>;
                    if constexpr (is_error_v<param_val_t>) {
                        return param_val_t();
                    } else {
                        return match_params_and_args_i(params, size_c<ParamIndex::value + 1>, args, size_c<ArgIndex::value + 1>, update_param_ctors(param_ctors, param_index, arg_index));
                    }
                } else {
                    return syntax_error<extra_non_keyword_argument>();
                }
            }
        } else {
            auto keyword_param_index = find_keyword_param(params, std::get<ArgIndex::value>(args));
            if constexpr (is_error_v<decltype(keyword_param_index)>) {
                return keyword_param_index;
            } else {
                using keyword_param_ctor = std::tuple_element_t<decltype(keyword_param_index)::value, ParamCtors>;
                if constexpr (keyword_param_ctor::value != npos) {
                    return syntax_error<duplicate_argument<typename arg_t::id>>();
                } else {
                    return match_params_and_args_i(params, param_index, args, size_c<ArgIndex::value + 1>, update_param_ctors(param_ctors, keyword_param_index, arg_index));
                }
            }
        }
    }
}

template <class Params, class Args>
constexpr auto match_params_and_args(Params &&params, Args &&args) {
    return match_params_and_args_i(params, size_c<0>, args, size_c<0>, initialize_param_ctors(params));
}

template <class Param, class Args, class ArgIndex>
constexpr decltype(auto) construct_params_ii([[maybe_unused]] Param &&param, [[maybe_unused]] Args &&args, ArgIndex) {
    if constexpr (!Param::is_default) {
        using param_val_t = typename Param::template pass<typename std::tuple_element_t<ArgIndex::value, Args>::value_type>;
        return static_cast<param_val_t>(std::get<ArgIndex::value>(std::move(args)).val());
    } else {
        if constexpr (ArgIndex::value == npos) {
            using param_val_t = typename Param::pass_default;
            return static_cast<param_val_t>(std::move(param).def());
        } else {
            using param_val_t = typename Param::template pass<typename std::tuple_element_t<ArgIndex::value, Args>::value_type>;
            return static_cast<param_val_t>(std::get<ArgIndex::value>(std::move(args)).val());
        }
    }
}

template <class Params, class Args, class ParamCtors, std::size_t ...ParamIndices>
constexpr auto construct_params_i(Params &&params, Args &&args, ParamCtors param_ctors, std::index_sequence<ParamIndices...>) {
    return std::tuple<decltype(construct_params_ii(std::get<ParamIndices>(std::move(params)), std::move(args), std::get<ParamIndices>(param_ctors)))...>(
        construct_params_ii(std::get<ParamIndices>(std::move(params)), std::move(args), std::get<ParamIndices>(param_ctors))...
    );
}

template <class Params, class Args, class ParamCtors>
constexpr auto construct_params(Params &&params, Args &&args, ParamCtors param_ctors) {
    return construct_params_i(std::move(params), std::move(args), param_ctors, std::make_index_sequence<std::tuple_size_v<ParamCtors>>());
}

template <class Params, class Args>
constexpr auto match_i(Params &&params, Args &&args) {
    auto param_ctors = match_params_and_args(params, args);
    if constexpr (is_error_v<decltype(param_ctors)>) {
        return param_ctors;
    } else {
        return construct_params(std::move(params), std::move(args), param_ctors);
    }
}

} // namespace detail

/* Keywords */

template <class Id>
struct keyword {
    using id = Id;

    template <class Val>
    constexpr detail::keyword_argument<Id, Val> operator=(Val &&val) const {
        return {std::forward<Val>(val)};
    }

    template <class T>
    constexpr detail::keyword_argument<Id, std::initializer_list<T>> operator=(std::initializer_list<T> &&il) const {
        return {std::move(il)};
    }
};

/* Parameters */

template <class Id>
constexpr detail::parameter<Id, detail::forward_pass, false> parameter(keyword<Id>) {
    return {};
}

template <class Type, class Id>
constexpr detail::parameter<Id, detail::convert_pass<Type>, false> parameter(keyword<Id>) {
    return {};
}

template <template <class> class Constraint, class Id>
constexpr detail::parameter<Id, detail::validate_pass<Constraint>, false> parameter(keyword<Id>) {
    return {};
}

template <class Id>
constexpr detail::parameter<Id, detail::forward_pass, true> keyword_parameter(keyword<Id>) {
    return {};
}

template <class Type, class Id>
constexpr detail::parameter<Id, detail::convert_pass<Type>, true> keyword_parameter(keyword<Id>) {
    return {};
}

template <template <class> class Constraint, class Id>
constexpr detail::parameter<Id, detail::validate_pass<Constraint>, true> keyword_parameter(keyword<Id>) {
    return {};
}

/* Matchers */

template <class ...ParamsAndArgs>
constexpr auto match(ParamsAndArgs &&...params_and_args) {
    auto [params, args] = detail::split_params_and_args(std::forward_as_tuple(std::forward<ParamsAndArgs>(params_and_args)...));
    auto np1 = detail::validate_params(params);
    if constexpr (detail::is_error_v<decltype(np1)>) {
        return np1;
    } else {
        auto np2 = detail::validate_args(args);
        if constexpr (detail::is_error_v<decltype(np2)>) {
            return np2;
        } else {
            return detail::match_i(std::move(params), std::move(args));
        }
    }
}

template <class ...ParamsAndArgs>
constexpr auto match_(ParamsAndArgs &&...params_and_args) -> std::enable_if_t<!detail::is_error_v<decltype(match(std::forward<ParamsAndArgs>(params_and_args)...))>, decltype(match(std::forward<ParamsAndArgs>(params_and_args)...))> {
    return match(std::forward<ParamsAndArgs>(params_and_args)...);
}

} // namespace flexargs

#endif
