#ifndef GUARD_system_base_hpp
#define GUARD_system_base_hpp
//  ---------------------------------------------
//  Basic system facilities
//  ---------------------------------------------
#include <concepts> // std::convertible_to
#include <string>

#include "os-detect.hpp" // MS_WINDOWS, POSIX

#if defined(MS_WINDOWS)
  #include <Windows.h>
#elif defined(POSIX)
  #include <unistd.h> // unlink, exec*, fork, ...
#endif


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace sys //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
{


#if defined(MS_WINDOWS)

//---------------------------------------------------------------------------
// Format system error message
[[nodiscard]] std::string get_lasterr_msg(DWORD e =0) noexcept
{
    //#include <system_error>
    //std::string message = std::system_category().message(e);

    if(e==0) e = ::GetLastError(); // ::WSAGetLastError()
    const DWORD buf_siz = 1024;
    TCHAR buf[buf_siz];
    const DWORD siz =
        ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS|
                         FORMAT_MESSAGE_MAX_WIDTH_MASK,
                         nullptr,
                         e,
                         0, // MAKELANGID deprecated
                         buf,
                         buf_siz,
                         nullptr );

    return siz>0 ? std::string(buf, siz)
                 : std::string("Unknown error ") + std::to_string(i);
}

//---------------------------------------------------------------------------
void shell_execute(const char* const pth, const char* const args =nullptr) noexcept
{
    SHELLEXECUTEINFOA ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    ShExecInfo.fMask = 0;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "open";
    ShExecInfo.lpFile = pth;
    ShExecInfo.lpParameters = args;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ::ShellExecuteEx(&ShExecInfo);
}

#endif


//---------------------------------------------------------------------------
// Same effect as double click on a file
void launch_file(const std::string& pth) noexcept
{
  #if defined(MS_WINDOWS)
    shell_execute( pth.c_str() );
  #elif defined(POSIX)
    if( const auto pid=fork(); pid==0 ) // pid_t
       {
        execlp("xdg-open", "xdg-open", pth.c_str(), nullptr);
       }
  #endif
}


//---------------------------------------------------------------------------
template<std::convertible_to<std::string> ...Args>
void execute(const char* const exe, Args&&... args) noexcept
{
  #if defined(MS_WINDOWS)
    try{
        auto join_args = [... args = std::forward<Args>(args)]() -> std::string
           {
            std::string s;
            const std::size_t totsiz = sizeof...(args) + (std::size(args) + ...);
            s.reserve(totsiz);
            ((s+=' ', s+=args), ...);
            return s;
           };

        shell_execute( exe, join_args().c_str() );
       }
    catch(...){}
  #elif defined(POSIX)
    if( const auto pid=fork(); pid==0 ) // pid_t
       {
        struct loc final
           {// Extract char pointer for posix api exec*
            static const char* c_str(const char* const s) noexcept { return s; }
            static const char* c_str(const std::string& s) noexcept { return s.c_str(); }
           };
        execlp(exe, exe, loc::c_str(std::forward<Args>(args))..., nullptr);
       }
  #endif
}


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



//---- end unit -------------------------------------------------------------
#endif
