#pragma once

#include <exception>
#include <functional>
#include <optional>

namespace Stf {

template<typename EF> struct ScopeExit;
template<typename EF> struct ScopeFail;
template<typename EF> struct ScopeSuccess;

namespace Detail {

template<typename Fn, typename ScopeType>
concept ScopeIsConstructible = (!std::is_same_v<std::remove_cvref_t<Fn>, ScopeType>)
                            && (std::is_constructible_v<typename ScopeType::function_type, Fn>);

template<typename Fn, typename ScopeType>
concept ScopeIsConstructibleWithForward
  = (!std::is_lvalue_reference_v<Fn>) && std::is_nothrow_constructible_v<typename ScopeType::function_type, Fn>;

template<typename Fn, typename ScopeType>
concept ScopeConstructorNoexcept = std::is_nothrow_constructible_v<typename ScopeType::function_type, Fn>
                                || std::is_nothrow_constructible_v<typename ScopeType::function_type, Fn&>;

template<typename ScopeType>
concept ScopeIsMoveConstructible = std::is_nothrow_move_constructible_v<typename ScopeType::function_type>
                                || std::is_copy_constructible_v<typename ScopeType::function_type>;

template<typename ScopeType>
concept ScopeIsMoveConstructibleWithForward = std::is_nothrow_move_constructible_v<typename ScopeType::function_type>;

template<typename ScopeType>
concept ScopeMoveConstructorNoexcept = std::is_nothrow_move_constructible_v<typename ScopeType::function_type>
                                    || std::is_nothrow_copy_constructible_v<typename ScopeType::function_type>;

template<typename EF> struct ScopeBase {
    using function_type = EF;

    friend struct ScopeExit<EF>;
    friend struct ScopeFail<EF>;
    friend struct ScopeSuccess<EF>;

    template<typename Fn>
        requires ScopeIsConstructible<Fn, ScopeBase> && ScopeIsConstructibleWithForward<Fn, ScopeBase>
    explicit constexpr ScopeBase(Fn&& callback) noexcept(ScopeConstructorNoexcept<Fn, ScopeBase>)
        : m_callback(std::forward<Fn>(callback))
        , m_armed(true) { }

    template<typename Fn>
        requires ScopeIsConstructible<Fn, ScopeBase> && (!ScopeIsConstructibleWithForward<Fn, ScopeBase>)
    explicit constexpr ScopeBase(Fn&& callback) noexcept(ScopeConstructorNoexcept<Fn, ScopeBase>)
        : m_callback(std::forward<Fn>(callback))
        , m_armed(true) { }

    constexpr ScopeBase(ScopeBase&& other) noexcept(ScopeMoveConstructorNoexcept<ScopeBase>)
        requires ScopeIsMoveConstructible<ScopeBase> && ScopeIsMoveConstructibleWithForward<ScopeBase>
    : m_callback(std::forward<EF>(other.m_callback)) {
        other.release();
    }

    constexpr ScopeBase(ScopeBase&& other) noexcept(ScopeMoveConstructorNoexcept<ScopeBase>)
        requires ScopeIsMoveConstructible<ScopeBase> && (!ScopeIsMoveConstructibleWithForward<ScopeBase>)
    : m_callback(other.m_callback) {
        other.release();
    }

    ScopeBase(ScopeBase const&) = delete;

    constexpr void release() noexcept { m_armed = false; }

private:
    EF m_callback;
    bool m_armed = false;

    constexpr void execute() {
        if (m_armed)
            std::invoke(m_callback);
    }
};

template<typename EF> ScopeBase(EF) -> ScopeBase<EF>;

}

template<typename EF> struct ScopeExit {
    template<typename T>
    constexpr ScopeExit(T&& v) noexcept(noexcept(Detail::ScopeBase<EF> { std::forward<T>(v) }))
        : m_impl(std::forward<T>(v)) { }

    constexpr ~ScopeExit() noexcept { m_impl.execute(); }

    constexpr void release() noexcept { m_impl.release(); }

private:
    Detail::ScopeBase<EF> m_impl {};
};

template<typename EF> ScopeExit(EF) -> ScopeExit<EF>;

template<typename EF> struct ScopeFail {
    template<typename T>
    constexpr ScopeFail(T&& v) noexcept(noexcept(Detail::ScopeBase<EF> { std::forward<T>(v) }))
        : m_impl(std::forward<T>(v)) { }

    constexpr ~ScopeFail() noexcept {
        if (std::uncaught_exceptions() != 0)
            m_impl.execute();
    }

    constexpr void release() noexcept { m_impl.release(); }

private:
    Detail::ScopeBase<EF> m_impl {};
};

template<typename EF> ScopeFail(EF) -> ScopeFail<EF>;

template<typename EF> struct ScopeSuccess {
    template<typename T>
    constexpr ScopeSuccess(T&& v) noexcept(noexcept(Detail::ScopeBase<EF> { std::forward<T>(v) }))
        : m_impl(std::forward<T>(v)) { }

    constexpr ~ScopeSuccess() noexcept {
        if (std::uncaught_exceptions() == 0)
            m_impl.execute();
    }

    constexpr void release() noexcept { m_impl.release(); }

private:
    Detail::ScopeBase<EF> m_impl {};
};

template<typename EF> ScopeSuccess(EF) -> ScopeSuccess<EF>;

}
