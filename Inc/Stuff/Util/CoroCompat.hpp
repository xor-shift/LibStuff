#pragma once

#ifdef __clang__
#    if !defined(__cpp_impl_coroutine) || !__cpp_impl_coroutine
#        define __cpp_impl_coroutine 1
#    endif

namespace std::experimental {
using namespace std;
}

#    define BOOST_ASIO_HAS_CO_AWAIT 1
#    define BOOST_ASIO_HAS_STD_COROUTINE
#endif