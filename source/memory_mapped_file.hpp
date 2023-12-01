#ifndef GUARD_memory_mapped_file_hpp
#define GUARD_memory_mapped_file_hpp
//  ---------------------------------------------
//  A file buffered in memory
//  ---------------------------------------------
#include <stdexcept> // std::runtime_error
#include <string>
#include <string_view>
#include <fmt/core.h> // fmt::format

#include "os-detect.hpp" // MS_WINDOWS, POSIX
#include "system_base.hpp" // sys::get_lasterr_msg()

#if defined(POSIX)
  #include <fcntl.h> // open
  #include <sys/stat.h> // fstat
  #include <sys/mman.h> // mmap, munmap
#endif


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace sys
{

/////////////////////////////////////////////////////////////////////////////
class memory_mapped_file final
{
 private:
    std::string m_path;
    const char* m_buf = nullptr;
    std::size_t m_bufsiz = 0;
  #if defined(MS_WINDOWS)
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = nullptr;
  #endif

 public:
    explicit memory_mapped_file( std::string&& pth )
      : m_path(pth)
       {
      #if defined(MS_WINDOWS)
        hFile = ::CreateFileA(m_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);
        if(hFile==INVALID_HANDLE_VALUE)
           {
            throw std::runtime_error( fmt::format("Couldn't open {} ({}))", m_path, get_lasterr_msg()));
           }
        m_bufsiz = ::GetFileSize(hFile, nullptr);

        hMapping = ::CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if(hMapping==nullptr)
           {
            ::CloseHandle(hFile);
            throw std::runtime_error( fmt::format("Couldn't map {} ({})", m_path, get_lasterr_msg()));
           }
        //
        m_buf = static_cast<const char*>( ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0) );
        if(m_buf==nullptr)
           {
            ::CloseHandle(hMapping);
            ::CloseHandle(hFile);
            throw std::runtime_error( fmt::format("Couldn't create view of {} ({})", m_path, get_lasterr_msg()) );
           }
      #elif defined(POSIX)
        const int fd = open(m_path.c_str(), O_RDONLY);
        if(fd==-1) throw std::runtime_error( fmt::format("Couldn't open {}", m_path) );

        // obtain file size
        struct stat sbuf {};
        if(fstat(fd, &sbuf)==-1) throw std::runtime_error("Cannot stat file size");
        m_bufsiz = static_cast<std::size_t>(sbuf.st_size);

        m_buf = static_cast<const char*>(mmap(nullptr, m_bufsiz, PROT_READ, MAP_PRIVATE, fd, 0U));
        if(m_buf==MAP_FAILED)
           {
            m_buf = nullptr;
            throw std::runtime_error("Cannot map file");
           }
      #endif
       }

    ~memory_mapped_file() noexcept
       {
        if(m_buf)
           {
          #if defined(MS_WINDOWS)
            ::UnmapViewOfFile(m_buf);
            if(hMapping) ::CloseHandle(hMapping);
            if(hFile!=INVALID_HANDLE_VALUE) ::CloseHandle(hFile);
          #elif defined(POSIX)
            /* const int ret = */ munmap(static_cast<void*>(const_cast<char*>(m_buf)), m_bufsiz);
            //if(ret==-1) std::cerr << "munmap() failed\n";
          #endif
           }
       }

    // Prevent copy
    memory_mapped_file(const memory_mapped_file& other) = delete;
    memory_mapped_file& operator=(const memory_mapped_file& other) = delete;

    // Move
    memory_mapped_file(memory_mapped_file&& other) noexcept
      : m_path(std::move(other.m_path))
      , m_buf(other.m_buf)
      , m_bufsiz(other.m_bufsiz)
    #if defined(MS_WINDOWS)
      , hFile(other.hFile)
      , hMapping(other.hMapping)
    #endif
       {
        other.m_bufsiz = 0;
        other.m_buf = nullptr;
      #if defined(MS_WINDOWS)
        other.hFile = INVALID_HANDLE_VALUE;
        other.hMapping = nullptr;
      #endif
       }
    // Prevent move assignment
    memory_mapped_file& operator=(memory_mapped_file&& other) = delete;

    [[nodiscard]] const std::string& path() const noexcept { return m_path; }
    [[nodiscard]] std::string_view as_string_view() const noexcept { return std::string_view{m_buf, m_bufsiz}; }
    //[[nodiscard]] std::size_t size() const noexcept { return m_bufsiz; }
    //[[nodiscard]] const char* begin() const noexcept { return m_buf; }
    //[[nodiscard]] const char* end() const noexcept { return m_buf + m_bufsiz; }
};

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



//---- end unit -------------------------------------------------------------
#endif
