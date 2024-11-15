/*
 * Copyright (c) 2021-2022, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021-2022, Kenneth Myhra <kennethmyhra@serenityos.org>
 * Copyright (c) 2021-2022, Sam Atkins <atkinssj@serenityos.org>
 * Copyright (c) 2022, Matthias Zimmerman <matthias291999@gmail.com>
 * Copyright (c) 2023, Cameron Youell <cameronyouell@gmail.com>
 * Copyright (c) 2024, stasoid <stasoid@yahoo.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/ByteString.h>
#include <AK/ScopeGuard.h>
#include <LibCore/System.h>
#include <WinSock2.h>
#include <io.h>

namespace Core::System {

ErrorOr<int> open(StringView path, int options, mode_t mode)
{
    ByteString string_path = path;
    int rc = _open(string_path.characters(), options, mode);
    if (rc < 0)
        return Error::from_syscall("open"sv, -errno);
    return rc;
}

ErrorOr<void> close(int fd)
{
    if (_close(fd) < 0)
        return Error::from_syscall("close"sv, -errno);
    return {};
}

ErrorOr<ssize_t> read(int fd, Bytes buffer)
{
    int rc = _read(fd, buffer.data(), buffer.size());
    if (rc < 0)
        return Error::from_syscall("read"sv, -errno);
    return rc;
}

ErrorOr<ssize_t> write(int fd, ReadonlyBytes buffer)
{
    int rc = _write(fd, buffer.data(), buffer.size());
    if (rc < 0)
        return Error::from_syscall("write"sv, -errno);
    return rc;
}

ErrorOr<off_t> lseek(int fd, off_t offset, int whence)
{
    long rc = _lseek(fd, offset, whence);
    if (rc < 0)
        return Error::from_syscall("lseek"sv, -errno);
    return rc;
}

ErrorOr<void> ftruncate(int fd, off_t length)
{
    long position = _tell(fd);
    if (position == -1)
        return Error::from_errno(errno);

    ScopeGuard restore_position { [&] { _lseek(fd, position, SEEK_SET); } };

    auto result = lseek(fd, length, SEEK_SET);
    if (result.is_error())
        return result.release_error();

    if (SetEndOfFile((HANDLE)_get_osfhandle(fd)) == 0)
        return Error::from_windows_error(GetLastError());
    return {};
}

ErrorOr<struct stat> fstat(int fd)
{
    struct stat st = {};
    if (::fstat(fd, &st) < 0)
        return Error::from_syscall("fstat"sv, -errno);
    return st;
}

ErrorOr<void> ioctl(int, unsigned, ...)
{
    dbgln("Core::System::ioctl() is not implemented");
    VERIFY_NOT_REACHED();
}

ErrorOr<void*> mmap(void*, size_t size, int protection, int flags, int fd, off_t offset, size_t alignment, StringView)
{
    // custom alignment is not supported
    VERIFY(!alignment);
    // address hint is not supported
    VERIFY(!(flags & MAP_FIXED));
    if (flags & ~(MAP_SHARED | MAP_PRIVATE | MAP_ANONYMOUS))
        dbgln("mmap: unrecognized flags {}", flags);

    int map_protection = 0, view_protection = 0;
    switch (protection) {
    case PROT_READ:
        map_protection = PAGE_READONLY;
        view_protection = FILE_MAP_READ;
        break;
    case PROT_READ | PROT_WRITE:
        map_protection = PAGE_READWRITE;
        view_protection = FILE_MAP_WRITE;
        break;
    default:
        VERIFY_NOT_REACHED();
    }

    HANDLE file_handle = INVALID_HANDLE_VALUE;
    // Anonymous mapping returned by this function is effectively always private because
    // the only way to share it is via the handle, which is immediately closed after MapViewOfFile.
    // Hence:
    // * shared anonymous is not supported
    // * private anonymous does not need to use copy-on-write
    if (flags & MAP_ANONYMOUS) {
        VERIFY(fd == -1 && offset == 0);
        VERIFY(!(flags & MAP_SHARED));
    } else {
        if (flags & MAP_PRIVATE)
            view_protection = FILE_MAP_COPY; // copy-on-write
        file_handle = (HANDLE)_get_osfhandle(fd);
        if (file_handle == INVALID_HANDLE_VALUE)
            return Error::from_errno(errno);
    }

    // double shift is to support 32-bit size
    HANDLE map_handle = CreateFileMapping(file_handle, NULL, map_protection, size >> 31 >> 1, size & 0xFFFFFFFF, NULL);
    if (!map_handle)
        return Error::from_windows_error(GetLastError());

    // NOTE: offset must be a multiple of the allocation granularity
    VERIFY(offset >= 0);
    void* ptr = MapViewOfFile(map_handle, view_protection, offset >> 31 >> 1, offset & 0xFFFFFFFF, 0);
    auto error = GetLastError();

    // It is OK to close map_handle here because
    // "Mapped views of a file mapping object maintain internal references to the object,
    // and a file mapping object does not close until all references to it are released."
    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfilemappinga
    CloseHandle(map_handle);

    if (!ptr)
        return Error::from_windows_error(error);

    return ptr;
}

ErrorOr<void> munmap(void* address, size_t)
{
    if (!UnmapViewOfFile(address))
        return Error::from_windows_error(GetLastError());
    return {};
}

}
