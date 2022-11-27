#pragma once

#include <cstddef>
#include <functional>

#include <Stuff/Intro/HelperTypes.hpp>

namespace Stf::Detail {

template<typename T, typename Fn, typename N> static constexpr decltype(auto) explore_aggregate(T&&, Fn&&, N&&) {
    static_assert(!std::is_same_v<T, T>, "Unsupported aggregate size");
}

template<typename T, typename N>
inline auto aggregate_member_helper(N) {
    static_assert(!std::is_same_v<T, T>, "Unsupported aggregate size");
}

/* run the following in your favourite JS runtime:
(()=>{
    let l=n=>{if(typeof n === "undefined")return"";let r=[];do{let t=n%26;t=t?t:26;n=(n-t)/26;r.push(t)}while(n>0)return r.map(v=>String.fromCharCode('a'.charCodeAt()+v-1)).reverse().join('')}
    let ls=upto=>{let r=[];for(let i=1;i<=upto;i++)r.push(l(i));return r}
    let a=n=>`AGGREGATE_EXPLORER_FACTORY(${n}, ${ls(n).join(", ")});`
    let as=upto=>{let r=[];for(let i=1;i<=upto;i++)r.push(a(i));return r}
    console.log(as(64).join('\n'))
})();

(()=>{
 let l=n=>{if(typeof n === "undefined")return"";let r=[];do{let t=n%26;t=t?t:26;n=(n-t)/26;r.push(t)}while(n>0)return r.map(v=>String.fromCharCode('a'.charCodeAt()+v-1)).reverse().join('')}
 let ls=upto=>{let r=[];for(let i=1;i<=upto;i++)r.push(l(i));return r}
 let a=n=>`template<typename T> inline auto aggregate_member_helper(std::integral_constant<size_t, ${n}>) {\n\tstd::unreachable();\n\tT* _v;\n\tauto&& [${ls(n).join(", ")}] = std::move(*_v);\n\treturn ABunchOfTypes<${ls(n).map(v=>"decltype("+v+")").join(", ")}>{};\n}`
 let as=upto=>{let r=[];for(let i=1;i<=upto;i++)r.push(a(i));return r}
 console.log(as(64).join("\n\n"))
})();
 */

//pad comment (the block above gets treated as documentation, heh)

#define AGGREGATE_EXPLORER_FACTORY(_N, ...)                                                                                                            \
    template<typename T, typename Fn> static constexpr decltype(auto) explore_aggregate(T&& data, Fn&& callable, std::integral_constant<size_t, _N>) { \
        auto&& [__VA_ARGS__] = data;                                                                                                                   \
        return std::invoke(callable, std::forward_as_tuple(__VA_ARGS__));                                                                              \
    }

#include "./AggregateExplorers.ipp"

#undef AGGREGATE_EXPLORER_FACTORY

template<size_t N, typename T, typename Fn> static constexpr decltype(auto) explore_aggregate(T&& data, Fn&& callable) {
    return explore_aggregate(std::forward<T>(data), std::forward<Fn>(callable), std::integral_constant<size_t, N> {});
}

}