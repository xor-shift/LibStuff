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
    using member_type = typename Accessor::member_type;
    using access_type = typename Accessor::access_type;

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

template<typename T> struct AccessTypeHelper {
    using type = T;
};

template<typename T> struct AccessTypeHelper<T[]> {
    using type = std::span<T>;
};

template<typename T> struct AccessTypeHelper<const T[]> {
    using type = std::span<const T>;
};

template<typename T, size_t N> struct AccessTypeHelper<T[N]> {
    using type = std::span<T, N>;
};

template<typename T, size_t N> struct AccessTypeHelper<const T[N]> {
    using type = std::span<const T, N>;
};

template<typename For, auto MemberPtr> struct MemberAccessor {
    using struct_type = For;
    using member_type = std::remove_reference_t<decltype(std::declval<For>().*MemberPtr)>;
    using const_member_type = std::remove_reference_t<decltype(std::declval<const For>().*MemberPtr)>;
    using access_type = typename Detail::AccessTypeHelper<member_type>::type;
    using const_access_type = typename Detail::AccessTypeHelper<const_member_type>::type;

    static constexpr bool span_accessor = std::is_array_v<member_type>;

    static constexpr member_type(For::*member_ptr) = MemberPtr;

    constexpr access_type& operator()(struct_type& obj) const
        requires(!span_accessor)
    {
        return obj.*member_ptr;
    }

    constexpr access_type&& operator()(struct_type&& obj) const
        requires(!span_accessor)
    {
        return std::move(obj.*member_ptr);
    }

    constexpr access_type const& operator()(struct_type const& obj) const
        requires(!span_accessor)
    {
        return obj.*member_ptr;
    }

    constexpr access_type const&& operator()(struct_type const&& obj) const
        requires(!span_accessor)
    {
        return std::move(obj.*member_ptr);
    }

    constexpr access_type operator()(struct_type& obj) const
        requires(span_accessor)
    {
        return obj.*member_ptr;
    }

    constexpr const_access_type operator()(struct_type const& obj) const
        requires(span_accessor)
    {
        return obj.*member_ptr;
    }
};

template<typename For, auto MemberPtr, typename Transform> struct TransformMemberAccessor {
    using struct_type = For;
    using member_type = std::remove_reference_t<decltype(std::declval<For>().*MemberPtr)>;
    using const_member_type = std::remove_reference_t<decltype(std::declval<const For>().*MemberPtr)>;
    using access_type = typename Detail::AccessTypeHelper<member_type>::type;
    using const_access_type = typename Detail::AccessTypeHelper<const_member_type>::type;

    static constexpr member_type(For::*member_ptr) = MemberPtr;
    static constexpr Transform transform {};

    constexpr decltype(auto) operator()(struct_type& obj) const { return transform(obj.*member_ptr); }
    constexpr decltype(auto) operator()(struct_type&& obj) const { return transform(std::move(obj.*member_ptr)); }
    constexpr decltype(auto) operator()(struct_type const& obj) const { return transform(obj.*member_ptr); }
    constexpr decltype(auto) operator()(struct_type const&& obj) const { return transform(std::move(obj.*member_ptr)); }
};

}

template<typename For, Detail::MemberDetails... MemberDetailers> struct StructBuilder {
    using type = For;
    using types = ABunchOfTypes<typename decltype(MemberDetailers)::access_type...>;
    using keys = ABunchOfValues<decltype(MemberDetailers)::member_name...>;

    template<size_t I> using nth_type = std::tuple_element_t<I, types>;

    static constexpr size_t size() { return sizeof...(MemberDetailers); }

    template<size_t N> static constexpr auto get_details() {
        auto tup = std::make_tuple(MemberDetailers...);
        return std::get<N>(tup);
    }

    template<size_t I, typename Struct> static constexpr decltype(auto) get(Struct&& v) {
        return StructBuilder::get_details<I>().accessor(std::forward<Struct>(v));
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
