#ifndef SAYA_IKA_AST_DETAIL_LOOKUP_QUERY_HPP
#define SAYA_IKA_AST_DETAIL_LOOKUP_QUERY_HPP

#include "saya/ika/ast_fwd.hpp"
#include "saya/ika/ast/id.hpp"

#include <boost/functional/hash.hpp>

#include <algorithm>
#include <deque>


namespace saya { namespace ika { namespace ast { namespace detail {

template<class T>
struct LookupQuery;

template<>
struct LookupQuery<Namespace>
{
    std::deque<NSID> qualifier;
    NSID target;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(std::deque<NSID> const& qualifier, NSID const& target)
        : qualifier(qualifier)
        , target(target)
    {}

    explicit LookupQuery(NSID const& target)
        : target(target)
    {}
};

template<>
struct LookupQuery<Var>
{
    std::deque<NSID> qualifier;
    VarID target;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(std::deque<NSID> const& qualifier, VarID const& target)
        : qualifier(qualifier)
        , target(target)
    {}

    explicit LookupQuery(VarID const& target)
        : target(target)
    {}
};

template<>
struct LookupQuery<Func>
{
    std::deque<NSID> qualifier;
    FuncID target;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(std::deque<NSID> const& qualifier, FuncID const& target)
        : qualifier(qualifier)
        , target(target)
    {}

    explicit LookupQuery(FuncID const& target)
        : target(target)
    {}
};

template<>
struct LookupQuery<Macro>
{
    std::deque<NSID> qualifier;
    MacroID target;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(std::deque<NSID> const& qualifier, MacroID const& target)
        : qualifier(qualifier)
        , target(target)
    {}

    explicit LookupQuery(MacroID const& target)
        : target(target)
    {}
};

template<>
struct LookupQuery<Group>
{
    bool dig{false};
    std::deque<GroupID> qualifier;
    GroupID target;
    boost::optional<AdditionalClass*> additional_class;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(bool dig, std::deque<GroupID> const& qualifier, GroupID const& target, boost::optional<AdditionalClass*> const& additional_class)
        : dig(dig)
        , qualifier(qualifier)
        , target(target)
        , additional_class(additional_class)
    {}

    explicit LookupQuery(bool dig, GroupID const& target, boost::optional<AdditionalClass*> const& additional_class)
        : dig(dig)
        , target(target)
        , additional_class(additional_class)
    {}

    explicit LookupQuery(GroupID const& target)
        : target(target)
    {}
};

template<>
struct LookupQuery<Endpoint>
{
    // std::deque<GroupID> qualifier;
    EndpointID target;
    boost::optional<AdditionalClass*> additional_class;

    std::size_t ctx_hash{static_cast<std::size_t>(-1)};

    explicit LookupQuery(EndpointID const& target, boost::optional<AdditionalClass*> const& additional_class)
        : target(target)
        , additional_class(additional_class)
    {}
};

template<class T, std::enable_if_t<!std::is_same<Endpoint, T>::value, int> = 0>
inline bool operator==(LookupQuery<T> const& lhs, LookupQuery<T> const& rhs)
{
    return
        lhs.target == rhs.target &&
        std::equal(lhs.qualifier.begin(), lhs.qualifier.end(), rhs.qualifier.begin(), rhs.qualifier.end())
    ;
}

template<class T, std::enable_if_t<std::is_same<Endpoint, T>::value, int> = 0>
inline bool operator==(LookupQuery<T> const& lhs, LookupQuery<T> const& rhs)
{
    return lhs.target == rhs.target;
}

template<class T>
inline std::size_t hash_value(LookupQuery<T> const& query)
{
    return query.ctx_hash;
}

}}}} // saya

#endif
