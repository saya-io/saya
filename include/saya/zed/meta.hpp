#ifndef SAYA_ZED_META_HPP
#define SAYA_ZED_META_HPP

#include "saya/zed/seq.hpp"
#include "saya/zed/fold.hpp"


namespace saya { namespace zed {

struct meta_arg {};

template<class... Args>
class template_
{
public:
    static constexpr std::size_t args_count = sizeof...(Args);
    static constexpr std::size_t meta_arg_count =
        std::integral_constant<std::size_t, (0 + std::is_same_v<Args, meta_arg>)...>::value
    ;
    static constexpr bool has_meta_arg = meta_arg_count > 0;

    static_assert(has_meta_arg, "variadic meta template must have at least one meta_arg");
};

template<class MT1>
class template_<MT1>
{
public:
    static constexpr std::size_t args_count = 1;
    static constexpr std::size_t meta_arg_count = std::is_same_v<MT1, meta_arg>;
    static constexpr bool has_meta_arg = meta_arg_count > 0;

    static_assert(has_meta_arg, "unary meta template must have exact one meta_arg");
};


template<class... Args>
struct auto_integral_sequence;

template<class T, T... Vs>
struct auto_integral_sequence<
    std::integral_constant<T, Vs>...
>
{
    using type = std::integer_sequence<T, Vs...>;
};

template<class... Args>
using auto_integral_sequence_t = typename auto_integral_sequence<Args...>::type;



template<class T1, class T2>
struct eq : std::conditional_t<(T1::value == T2::value), std::true_type, std::false_type> {};

template<class T1, class T2>
struct not_eq : std::conditional_t<(T1::value != T2::value), std::true_type, std::false_type> {};


template<class T>
struct is_true : std::conditional_t<T::value, std::true_type, std::false_type> {};

template<class T>
struct is_false : std::conditional_t<!T::value, std::true_type, std::false_type> {};


template<class T1, class T2>
struct less : std::conditional_t<(T1::value < T2::value), std::true_type, std::false_type> {};

template<class T1, class T2>
struct less_eq : std::conditional_t<(T1::value <= T2::value), std::true_type, std::false_type> {};

template<class T1, class T2>
struct greater : std::conditional_t<(T1::value > T2::value), std::true_type, std::false_type> {};

template<class T1, class T2>
struct greater_eq : std::conditional_t<(T1::value >= T2::value), std::true_type, std::false_type> {};


template<
    template<class...>
    class Tuple,
    class... Args
>
struct lazy_unwrap
{
    using type = lazy_unwrap<Tuple, Args...>;
};

template<
    template<class...>
    class Tuple,
    class... As
>
struct lazy_unwrap<
    Tuple, Tuple<As...>
>
{
    using type = lazy_unwrap<Tuple, As...>;
};

template<
    template<class...>
    class Tuple,
    class... Args
>
using lazy_unwrap_t = typename lazy_unwrap<Tuple, Args...>::type;


template<template<class...> class Tuple, std::size_t I, class T>
struct element;

template<template<class...> class Tuple, std::size_t I, class Head, class... Tail>
struct element<Tuple, I, Tuple<Head, Tail...>> : element<Tuple, I - 1, Tuple<Tail...>> {};

template<template<class...> class Tuple, class Head, class... Tail>
struct element<Tuple, 0, Tuple<Head, Tail...>> { using type = Head; };

template<template<class...> class Tuple, std::size_t I, class T>
using element_t = typename element<Tuple, I, T>::type;


namespace detail {

template<
    template<class...>
    class Tuple,
    class A,
    class T,
    class HeadSeq,
    class TailSeq
>
struct element_assign_impl;

template<
    template<class...>
    class Tuple,
    class A,
    class T,
    std::size_t... HeadIs,
    std::size_t... TailIs
>
struct element_assign_impl<
    Tuple, A, T,
    std::index_sequence<HeadIs...>, std::index_sequence<TailIs...>
>
{
    using type = Tuple<
        element_t<Tuple, HeadIs, A>...,
        T,
        element_t<Tuple, TailIs, A>...
    >;
};

template<template<class...> class Tuple, class A, std::size_t MaxI, std::size_t I, class T>
struct element_assign_dispatch
{
    // MaxI = 6, I = 4
    // [0, 1, 2, 3, 4, 5, 6] -> [0, 1, 2, 3], T, [5, 6]
    using type = typename detail::element_assign_impl<
        Tuple, A, T,
        std::make_index_sequence<I>,
        i_seq_offset_t<std::size_t, I + 1, std::make_index_sequence<MaxI - I>>
    >::type;
};

template<
    template<class...>
    class Tuple,
    std::size_t MaxI,
    class T,
    class Head,
    class... Tail
>
struct element_assign_dispatch<
    Tuple, Tuple<Head, Tail...>, MaxI, 0, T
>
{
    using type = Tuple<T, Tail...>;
};

template<
    template<class...>
    class Tuple,
    std::size_t MaxI,
    class T,
    class Tail,
    class... Head
>
struct element_assign_dispatch<
    Tuple, Tuple<Head..., Tail>, MaxI, MaxI, T
>
{
    using type = Tuple<Head..., T>;
};

} // detail


template<template<class...> class Tuple, class A, std::size_t I, class T>
struct element_assign;

template<
    template<class...>
    class Tuple,
    std::size_t I, class T, class... Args
>
struct element_assign<
    Tuple, Tuple<Args...>, I, T
>
{
    using type = typename detail::element_assign_dispatch<Tuple, Tuple<Args...>, sizeof...(Args) - 1, I, T>::type;
};

template<template<class...> class Tuple, class A, std::size_t I, class T>
using element_assign_t = typename element_assign<Tuple, A, I, T>::type;


namespace detail {

namespace aux_ {

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_apply;

template<
    template<class>
    class Template, // unary meta
    class Arg,
    class... MTs
>
struct meta_apply<
    Template,
    template_<MTs...>, // auto-assign
    Arg
>
{
    using type = Template<Arg>;
};

template<
    class MT1, class MT2,
    template<class, class>
    class Template, // binary meta
    class Arg
>
struct meta_apply<
    Template,
    template_<MT1, MT2>,
    Arg
>
{
    static_assert(std::is_same_v<MT1, meta_arg> || std::is_same_v<MT2, meta_arg>, "binary meta type: either MT1 or MT2 must be meta_arg");

    using type = std::conditional_t<
        std::is_same_v<MT1, meta_arg>,
        Template<Arg, MT2>,
        Template<MT1, Arg>
    >;
};


template<
    template<class, class>
    class Template, // binary meta
    class Arg1, class Arg2,
    class... MTs
>
struct meta_apply<
    Template,
    template_<MTs...>, // auto-assign
    Arg1, Arg2
>
{
    using type = Template<Arg1, Arg2>;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using meta_apply_t = typename meta_apply<Template, MetaTemplate, Args...>::type;

} // aux_


template<
    class Result,
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_apply_impl;

template<
    class Result,
    template<class...>
    class Template,
    class MetaTemplate
>
struct meta_apply_impl<
    Result, Template, MetaTemplate
>
{
    using type = Result;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class Arg,
    class... ResultArgs,
    class... Rest
>
struct meta_apply_impl<
    std::tuple<ResultArgs...>,
    Template,
    MetaTemplate,
    Arg, Rest...
>
{
    using type = typename meta_apply_impl<
        std::tuple<
            ResultArgs...,
            aux_::meta_apply_t<Template, MetaTemplate, Arg>
        >,
        Template,
        MetaTemplate,
        Rest...
    >::type;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_apply_dispatcher
{
    using type = typename meta_apply_impl<
        std::tuple<>, Template, MetaTemplate, Args...
    >::type;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class Arg
>
struct meta_apply_dispatcher<
    Template, MetaTemplate, Arg
>
{
    using type = aux_::meta_apply_t<Template, MetaTemplate, Arg>;
};

} // detail


template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using meta_apply = typename detail::meta_apply_dispatcher<Template, MetaTemplate, Args...>::type;


namespace detail {

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_t_dispatcher
{
    using type = std::tuple<
        typename meta_apply<Template, MetaTemplate, Args>::type...
    >;
};

template<
    template<class...>
    class LazyUnwrapTuple,
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_t_dispatcher<
    Template,
    MetaTemplate,
    lazy_unwrap<LazyUnwrapTuple, Args...>
>
{
    using type = typename meta_t_dispatcher<Template, MetaTemplate, Args...>::type;
};

} // detail


template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using meta_t = typename detail::meta_t_dispatcher<Template, MetaTemplate, Args...>::type;


namespace detail {

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_v_dispatcher
{
    using type = auto_integral_sequence_t<
        typename aux_::meta_apply_t<Template, MetaTemplate, Args>::type...
    >;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct meta_v_dispatcher<
    Template,
    MetaTemplate,
    std::tuple<Args...>
>
{
    using type = auto_integral_sequence_t<
        typename aux_::meta_apply_t<Template, MetaTemplate, Args>::type...
    >;
};

} // detail

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using meta_v = typename detail::meta_v_dispatcher<Template, MetaTemplate, Args...>::type;


namespace detail {

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct all_of_dispatcher
{
    using type = typename bool_dispatch_t<fold_logical_and, meta_v<Template, MetaTemplate, Args...>>::type;
};

} // detail

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using all_of = typename detail::all_of_dispatcher<Template, MetaTemplate, Args...>::type;


template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
constexpr bool all_of_v = all_of<Template, MetaTemplate, Args...>::value;


namespace detail {

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
struct any_of_impl
{
    using type = fold_logical_or_t<meta_apply<Template, MetaTemplate, Args>::value...>;
};

template<
    template<class...>
    class Template,
    class MetaTemplate,
    template<class...>
    class Tuple,
    class... Args
>
struct any_of_impl<
    Template, MetaTemplate, lazy_unwrap<Tuple, Args...>
>
{
    using type = fold_logical_or_t<meta_apply<Template, MetaTemplate, Args>::value...>;
};
} // detail

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
using any_of = typename detail::any_of_impl<Template, MetaTemplate, Args...>::type;

template<
    template<class...>
    class Template,
    class MetaTemplate,
    class... Args
>
constexpr bool any_of_v = any_of<Template, MetaTemplate, Args...>::value;


template<
    template<class...>
    class Tuple,
    template<class...>
    class CondTemplate,
    class MetaTemplate,
    class TrueType, class FalseType,
    class A
>
struct meta_conditional;

template<
    template<class...>
    class Tuple,
    template<class...>
    class CondTemplate,
    class MetaTemplate,
    class TrueType, class FalseType,
    class... Args
>
struct meta_conditional<
    Tuple, CondTemplate, MetaTemplate, TrueType, FalseType,
    Tuple<Args...>
>
{
    using type = std::tuple<
        typename std::conditional<
            meta_apply<CondTemplate, MetaTemplate, Args>::value,
            TrueType, FalseType
        >::type...
    >;
};


template<
    template<class...>
    class Tuple,
    template<class...>
    class CondTemplate,
    class MetaTemplate,
    class TrueType, class FalseType,
    class A
>
using meta_conditional_t = typename meta_conditional<Tuple, CondTemplate, MetaTemplate, TrueType, FalseType, A>::type;


namespace detail {

template<
    class Result,
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class... Args
>
struct reduced_impl;

template<
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class Prev,
    class Current,
    class... Result,
    class... Rest
>
struct reduced_impl<
    std::tuple<Result...>,
    BinaryTemplate,
    MetaTemplate,
    Prev,
    Current,
    Rest...
>
{
    using type = typename reduced_impl<
        std::tuple<
            Result...,
            BinaryTemplate<Prev, Current>
        >,
        BinaryTemplate,
        MetaTemplate,
        Current,
        Rest...
    >::type;
};

template<
    class Result,
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class Prev
>
struct reduced_impl<
    Result,
    BinaryTemplate,
    MetaTemplate,
    Prev
>
{
    using type = Result;
};

} // detail

template<
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class... Args
>
struct reduced
{
    using type = typename detail::reduced_impl<
        std::tuple<>,
        BinaryTemplate,
        MetaTemplate,
        Args...
    >::type;
};


template<
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class... Args
>
struct reduced<
    BinaryTemplate,
    MetaTemplate,
    std::tuple<Args...>
>
{
    using type = typename reduced<BinaryTemplate, MetaTemplate, Args...>::type;
};

template<
    template<class, class>
    class BinaryTemplate,
    class MetaTemplate,
    class... Args
>
using reduced_t = typename reduced<BinaryTemplate, MetaTemplate, Args...>::type;


template<class... Args>
struct paired
{
    using type = reduced_t<std::pair, template_<>, Args...>;
};

template<class... Args>
using paired_t = typename paired<Args...>::type;


namespace detail {

template<
    class Result,
    template<class...>
    class Tuple,
    class... Args
>
struct compact_impl
{
    using type = typename compact_impl<Result, Tuple, Args...>::type;
};

template<
    class Result,
    template<class...>
    class Tuple
>
struct compact_impl<
    Result,
    Tuple
>
{
    using type = Result;
};

template<
    template<class...>
    class Tuple,
    class T1, class... Rest,
    class... Result
>
struct compact_impl<
    Tuple<Result...>,
    Tuple,
    T1, Rest...
>
{
    using type = std::conditional_t<
        is_void<T1>::value,
        typename compact_impl<
            Tuple<Result...>,
            Tuple,
            Rest...
        >::type,
        typename compact_impl<
            Tuple<Result..., T1>,
            Tuple,
            Rest...
        >::type
    >;
};

} // detail

template<
    template<class...>
    class Tuple,
    class... Args
>
struct compact
{
    using type = typename detail::compact_impl<Tuple<>, Tuple, Args...>::type;
};

template<
    template<class...>
    class Tuple,
    class... Args
>
using compact_t = typename compact<Tuple, Args...>::type;


namespace detail {

template<
    class Result,
    template<class...>
    class Tuple,
    class... Args
>
struct reversed_impl;

template<
    class Result,
    template<class...>
    class Tuple
>
struct reversed_impl<
    Result,
    Tuple
>
{
    using type = Result;
};

template<
    template<class...>
    class Tuple,
    class T1, class... Result, class... Rest
>
struct reversed_impl<
    Tuple<Result...>,
    Tuple,
    T1, Rest...
>
{
    using type = typename reversed_impl<
        Tuple<T1, Result...>,
        Tuple,
        Rest...
    >::type;
};

} // detail

template<
    template<class...>
    class Tuple,
    class... Args
>
struct reversed
{
    using type = typename detail::reversed_impl<Tuple<>, Tuple, Args...>::type;
};

template<
    template<class...>
    class Tuple,
    class... Args
>
using reversed_t = typename reversed<Tuple, Args...>::type;


namespace detail {

template<
    template<class...>
    class Tuple,
    class... Ts
>
struct concat_impl
{
    using type = typename concat_impl<Tuple, Ts...>::type;
};

template<
    template<class...>
    class Tuple,
    class Result
>
struct concat_impl<
    Tuple, Result
>
{
    using type = Result;
};

template<
    template<class...>
    class Tuple,
    class... T1s, class... T2s, class... Rest
>
struct concat_impl<
    Tuple,
    Tuple<T1s...>, Tuple<T2s...>, Rest...
>
{
    using type = typename concat_impl<
        Tuple,
        Tuple<T1s..., T2s...>,
        Rest...
    >::type;
};

} // detail

template<
    template<class...>
    class Tuple,
    class... Args
>
struct concat
{
    using type = typename detail::concat_impl<Tuple, Args...>::type;
};

template<
    template<class...>
    class Tuple,
    class... Args
>
using concat_t = typename concat<Tuple, Args...>::type;


template<
    template<class...>
    class Tuple,
    template<
        template<class...>
        class Tuple,
        class... Args
    >
    class Template,
    class T
>
struct unwrap_apply;

template<
    template<class...>
    class Tuple,
    template<
        template<class...>
        class Tuple,
        class... Args
    >
    class Template,
    class... Args
>
struct unwrap_apply<
    Tuple,
    Template,
    Tuple<Args...>
>
{
    using type = typename Template<Tuple, Args...>::type;
};

template<
    template<class...>
    class Tuple,
    template<
        template<class...>
        class Tuple,
        class... Args
    >
    class Template,
    class T
>
using unwrap_apply_t = typename unwrap_apply<Tuple, Template, T>::type;


namespace detail {

template<
    class Result,
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A, class T
>
struct zipped_single_impl
{
    using type = typename zipped_single_impl<
        Result, Tuple, PairTuple, A, T
    >::type;
};

template<
    class Result,
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class T
>
struct zipped_single_impl<
    Result,
    Tuple,
    PairTuple,
    Tuple<>,
    T
>
{
    using type = Result;
};

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class T,
    class A1, class... Rest,
    class... Result
>
struct zipped_single_impl<
    Tuple<Result...>,
    Tuple,
    PairTuple,
    Tuple<A1, Rest...>,
    T
>
{
    using type = typename zipped_single_impl<
        Tuple<Result..., PairTuple<A1, T>>,
        Tuple,
        PairTuple,
        Tuple<Rest...>,
        T
    >::type;
};

} // detail

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A, class T
>
struct zipped_single
{
    using type = typename detail::zipped_single_impl<
        Tuple<>, Tuple, PairTuple, A, T
    >::type;
};

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A, class T
>
using zipped_single_t = typename zipped_single<Tuple, PairTuple, A, T>::type;


namespace detail {

template<
    class Result,
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A1, class A2,
    class Hold1, class Hold2
>
struct zipped_impl; // { using type = typename zipped_impl<Result, Tuple, PairTuple, A1, A2, Hold1, Hold2>::type; };

template<
    class Result,
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A1, class A2,
    class... Hold2Unused
>
struct zipped_impl<
    Result,
    Tuple,
    PairTuple,
    A1, A2,
    any_holder<>,
    any_holder<Hold2Unused...>
>
{
    using type = Result;
};

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class Hold1,
    class A1, class... A2s,
    class... Hold1Rest,
    class... Results
>
struct zipped_impl<
    any_holder<Results...>,
    Tuple,
    PairTuple,
    A1, Tuple<A2s...>,
    any_holder<Hold1, Hold1Rest...>,
    any_holder<>
>
{
    using type = typename zipped_impl<
        any_holder<Results...>,
        Tuple, PairTuple, A1, Tuple<A2s...>,
        any_holder<Hold1Rest...>,
        any_holder<A2s...>
    >::type;
};

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class Hold1,
    class Hold2,
    class A1, class A2,
    class... Hold1Rest, class... Hold2Rest,
    class... Results
>
struct zipped_impl<
    any_holder<Results...>,
    Tuple,
    PairTuple,
    A1, A2,
    any_holder<Hold1, Hold1Rest...>,
    any_holder<Hold2, Hold2Rest...>
>
{
    using type = typename zipped_impl<
        any_holder<Results..., PairTuple<Hold1, Hold2>>,
        Tuple, PairTuple, A1, A2,
        any_holder<Hold1, Hold1Rest...>,
        any_holder<Hold2Rest...>
    >::type;
};

} // detail


template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A1, class A2
>
struct zipped;

template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class... A1s, class... A2s
>
struct zipped<
    Tuple,
    PairTuple,
    Tuple<A1s...>, Tuple<A2s...>
>
{
    using type = rewrap_t<
        any_holder,
        Tuple,
        typename detail::zipped_impl<
            any_holder<>,
            Tuple,
            PairTuple,
            Tuple<A1s...>,
            Tuple<A2s...>,
            any_holder<A1s...>,
            any_holder<A2s...>
        >::type
    >;
};
template<
    template<class...>
    class Tuple,
    template<class T1, class T2>
    class PairTuple,
    class A1, class A2
>
using zipped_t = typename zipped<Tuple, PairTuple, A1, A2>::type;

}} // saya

#endif
