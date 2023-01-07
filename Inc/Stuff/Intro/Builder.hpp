#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <numeric>
#include <span>
#include <string_view>
#include <tuple>

#include <Stuff/Intro/HelperTypes.hpp>

namespace Stf::Intro {

namespace Detail {

template<ArrayString MemberName, typename Accessor> struct MemberDetails {
    using struct_type = typename Accessor::struct_type;

    static constexpr ArrayString member_name = MemberName;

    Accessor accessor;

    static constexpr std::string_view name() { return { MemberName.str }; }
};

template<ArrayString TargetName, size_t OriginalSize, MemberDetails V, MemberDetails... Vs>
constexpr size_t find_named_member_index() {
    constexpr size_t left = 1 + sizeof...(Vs);
    constexpr size_t index = OriginalSize - left;

    if constexpr ((V.member_name <=> TargetName) == std::strong_ordering::equal) {
        return index;
    } else if constexpr (left == 1) {
        return OriginalSize;
    } else {
        return find_named_member_index<TargetName, OriginalSize, Vs...>();
    }
}

template<typename T> struct ArrayMemberHelper;

template<typename T> struct ArrayMemberHelper<T[]> {
    using value_type = T;
    static constexpr size_t extent = std::dynamic_extent;

    using span_type = std::span<value_type, extent>;
    using const_span_type = std::span<const value_type, extent>;
};

template<typename T, size_t N> struct ArrayMemberHelper<T[N]> {
    using value_type = T;
    static constexpr size_t extent = N;

    using span_type = std::span<value_type, extent>;
    using const_span_type = std::span<const value_type, extent>;
};

template<typename For, auto MemberPtr>
using member_type = std::remove_reference_t<decltype(std::declval<For>().*MemberPtr)>;

template<typename For, auto MemberPtr> struct MemberAccessor;

template<typename For, auto MemberPtr>
    requires std::is_array_v<member_type<For, MemberPtr>>
struct MemberAccessor<For, MemberPtr> {
    using struct_type = For;

    static constexpr auto(For::*member_ptr) = MemberPtr;

    constexpr auto operator()(struct_type& obj) const {
        return typename ArrayMemberHelper<member_type<For, MemberPtr>>::span_type { obj.*member_ptr };
    }

    constexpr auto operator()(struct_type const& obj) const {
        return typename ArrayMemberHelper<member_type<For, MemberPtr>>::const_span_type { obj.*member_ptr };
    }
};

template<typename For, auto MemberPtr>
    requires(!std::is_array_v<member_type<For, MemberPtr>>)
struct MemberAccessor<For, MemberPtr> {
    using struct_type = For;

    static constexpr auto(For::*member_ptr) = MemberPtr;

    constexpr auto& operator()(struct_type& obj) const { return obj.*member_ptr; }
    constexpr decltype(auto) operator()(struct_type&& obj) const { return std::move(obj.*member_ptr); }
    constexpr auto const& operator()(struct_type const& obj) const { return obj.*member_ptr; }
    constexpr decltype(auto) operator()(struct_type const&& obj) const { return std::move(obj.*member_ptr); }
};

template<typename For, auto MemberPtr, typename Transform> struct TransformMemberAccessor {
    using struct_type = For;

    static constexpr auto(For::*member_ptr) = MemberPtr;
    static constexpr Transform transform {};

    constexpr decltype(auto) operator()(struct_type& obj) const { return transform(obj.*member_ptr); }
    constexpr decltype(auto) operator()(struct_type&& obj) const { return transform(std::move(obj.*member_ptr)); }
    constexpr decltype(auto) operator()(struct_type const& obj) const { return transform(obj.*member_ptr); }
    constexpr decltype(auto) operator()(struct_type const&& obj) const { return transform(std::move(obj.*member_ptr)); }
};

}

template<typename For, Detail::MemberDetails... MemberDetailers> struct StructBuilder {
    using type = For;
    using keys = ABunchOfValues<decltype(MemberDetailers)::member_name...>;

    static constexpr size_t size() { return sizeof...(MemberDetailers); }
    static constexpr size_t size(For const&) { return size(); }

    template<size_t N> static constexpr auto get_details() {
        auto tup = std::make_tuple(MemberDetailers...);
        return std::get<N>(tup);
    }

    template<size_t I, typename Struct> static constexpr decltype(auto) get(Struct&& v) {
        return StructBuilder::get_details<I>().accessor(std::forward<Struct>(v));
    }

    template<size_t I>
    static constexpr auto key_name() {
        return keys::template get<I>();
    }

    template<ArrayString MemberName, typename Struct> static constexpr decltype(auto) get(Struct&& v) {
        constexpr size_t index = Detail::find_named_member_index<MemberName, size(), MemberDetailers...>();
        return get<index, Struct>(std::forward<Struct>(v));
    }

    template<auto ptr, ArrayString MemberName> constexpr auto add_simple() {
        using Accessor = Detail::MemberAccessor<For, ptr>;
        constexpr Accessor accessor {};

        using detail_type = Detail::MemberDetails<MemberName, Accessor>;

        constexpr detail_type details {
            .accessor = accessor,
        };

        using ret_type = StructBuilder<For, MemberDetailers..., details>;

        return ret_type {};
    }

    template<auto ptr, ArrayString MemberName, typename Transform> constexpr auto add_with_transform(Transform&&) {
        using Accessor = Detail::TransformMemberAccessor<For, ptr, std::remove_cvref_t<Transform>>;
        constexpr Accessor accessor {};

        using detail_type = Detail::MemberDetails<MemberName, Accessor>;

        constexpr detail_type details {
            .accessor = accessor,
        };

        using ret_type = StructBuilder<For, MemberDetailers..., details>;

        return ret_type {};
    }
};

}
