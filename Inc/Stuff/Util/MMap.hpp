#pragma once

#include <Stuff/Util/Hacks/Try.hpp>

#include <span>
#include <string>
#include <string_view>

namespace Stf {

struct MMapStringView {
    MMapStringView(std::string const& filename, bool readonly)
        : m_filename(filename)
        , m_readonly(readonly) {
        initialize();
    }

    ~MMapStringView() noexcept { deinitialize(); }

    char* data() noexcept { return reinterpret_cast<char*>(m_data); }

    const char* data() const noexcept { return reinterpret_cast<const char*>(m_data); }

    constexpr size_t size() const noexcept { return m_filesize; }

    operator std::string_view() const noexcept { return { data(), data() + size() }; }

    operator std::span<const char>() const noexcept { return { data(), data() + size() }; }

    operator std::span<char>() { return { data(), data() + size() }; }

private:
    std::string m_filename;
    bool m_readonly;

    size_t m_filesize = 0;
    void* m_data = nullptr;
    int m_fildes;

    void initialize() noexcept;

    void deinitialize() noexcept;
};

}
