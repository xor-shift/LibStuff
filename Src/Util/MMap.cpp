// TODO: add proper platform detection
#ifdef __unix__

#include <Stuff/Util/MMap.hpp>

#include <Stuff/Util/Scope.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

namespace Stf {

void MMapStringView::initialize() noexcept {
    m_fildes = open(m_filename.c_str(), m_readonly ? O_RDONLY : O_RDWR);
    if (m_fildes < 0) {
        return;
    }

    Stf::ScopeGuard open_guard(Stf::GuardType::ScopeExit, [this] {
        close(m_fildes);
        m_fildes = -1;
    });

    struct stat stats{};
    if (fstat(m_fildes, &stats) < 0) {
        return;
    }

    m_filesize = stats.st_size;
    m_data = mmap(nullptr, m_filesize, PROT_READ | (m_readonly ? 0 : PROT_WRITE), MAP_PRIVATE, m_fildes, 0);
    if (m_data == MAP_FAILED) {
        return;
    }

    open_guard.release();
}

void MMapStringView::deinitialize() noexcept {
    if (m_fildes == -1)
        return;

    munmap(m_data, m_filesize);
    close(m_fildes);
}

}

#endif
