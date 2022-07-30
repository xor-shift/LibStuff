#pragma once

namespace Stf {

template<typename... Funcs>
struct MultiVisitor : public Funcs...{
    using Funcs::operator()...;
};

template<typename... Funcs>
MultiVisitor(Funcs&&...) -> MultiVisitor<Funcs...>;

}
