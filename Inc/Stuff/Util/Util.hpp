#pragma once

#include <cstddef>
#include <functional>

#include "./Conv.hpp"
#include "./CoroCompat.hpp"
#include "./DummyIterator.hpp"
#include "./Error.hpp"
#include "./Scope.hpp"
#include "./SpinLock.hpp"

namespace Stf {

template<typename... Funcs>
struct MultiVisitor : public Funcs...{
    using Funcs::operator()...;
};

template<typename... Funcs>
MultiVisitor(Funcs&&...) -> MultiVisitor<Funcs...>;

}
