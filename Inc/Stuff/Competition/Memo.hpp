#pragma once

#include <list>

#include <Stuff/Util/Tuple.hpp>

namespace Stf::Comp {

template<typename Ret, typename... Args> struct MemoizerBase {
    MemoizerBase(size_t lru_suze = 512uz * 1024uz)
        : m_lru_size(lru_suze) {
        m_lru_map.reserve(m_lru_size);
        m_memo.reserve(m_lru_size);
    }

    virtual ~MemoizerBase() = default;

    Ret operator()(Args... args) {
        ++m_all_calls;

        auto arg_tuple = std::make_tuple(args...);
        if (auto it = m_memo.find(arg_tuple); it != m_memo.end()) {
            ++m_memo_calls;
            return it->second;
        }

        auto ret = impl(std::move(args)...);

        remember(ret, std::move(arg_tuple));
        return ret;
    }

    Ret call(Args... args) { return (*this)(std::move(args)...); }

    size_t all_calls() { return m_all_calls; }
    size_t memo_calls() { return m_memo_calls; }

protected:
    virtual Ret impl(Args...) = 0;

//private:
    using tuple_type = std::tuple<Args...>;
    using list_type = std::list<tuple_type>;

    size_t m_all_calls = 0;
    size_t m_memo_calls = 0;

    size_t m_lru_size;
    std::unordered_map<tuple_type, typename list_type::iterator> m_lru_map {};
    list_type m_lru_list {};

    std::unordered_map<tuple_type, Ret> m_memo {};

    void forget_least_recent() {
        auto front = m_lru_list.begin();
        m_memo.erase(*front);
        m_lru_map.erase(*front);
        m_lru_list.erase(front);
    }

    bool make_most_recent(tuple_type const& args) {
        auto it = m_lru_map.find(args);
        if (it == m_lru_map.end()) {
            return false;
        }

        m_lru_list.splice(begin(m_lru_list), m_lru_list, it->second, next(it->second));

        return true;
    }

    void remember(Ret ret, tuple_type args) {
        if (make_most_recent(args))
            return;

        while (m_lru_list.size() >= m_lru_size)
            forget_least_recent();

        m_memo.insert({args, ret});
        m_lru_list.push_back(args);
        m_lru_map.insert({args, --m_lru_list.end()});
    }
};

}
