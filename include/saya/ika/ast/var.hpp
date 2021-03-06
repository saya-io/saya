#ifndef SAYA_IKA_AST_VAR_HPP
#define SAYA_IKA_AST_VAR_HPP

#include "saya/ika/ast_fwd.hpp"
#include "saya/ika/ast/ast_entity.hpp"

namespace saya { namespace ika { namespace ast {

struct Var : ASTEntity
{
    VarID id;
    vm::Value vv;

    Var() = default;
    explicit Var(VarID const& id, vm::TypeID const& type_id)
        : id(id), vv(type_id)
    {}

    explicit Var(VarID const& id)
        : id(id), vv()
    {}
};

}}} // saya

#endif
