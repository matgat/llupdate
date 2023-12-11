#pragma once
//  ---------------------------------------------
//  Common encoding agnostic parsing facilities
//  ---------------------------------------------
#include <cassert>
#include <concepts>
#include <stdexcept> // std::exception, std::runtime_error, ...
#include <string_view>
#include <fmt/core.h> // fmt::format

#include "text.hpp" // text::*


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace MG
{

/////////////////////////////////////////////////////////////////////////////
class parse_error final : public std::exception
{
 private:
    std::string m_msg;
    std::size_t m_line;

 public:
    explicit parse_error(std::string&& msg,
                         const std::size_t lin) noexcept
       : m_msg(msg)
       , m_line(lin)
        {}

    std::size_t line() const noexcept { return m_line; }

    const char* what() const noexcept override { return m_msg.c_str(); } // Could rise a '-Wweak-vtables'
};


//---------------------------------------------------------------------------
void default_notify(const std::string_view) noexcept
   {
   }

   
//template <typename F>
//concept notify_fun = requires(F f, std::string_view msg)
//   {
//    f(msg);
//   };

/////////////////////////////////////////////////////////////////////////////
template<text::Enc enc, auto fnotify =default_notify> class ParserBase
{
    using buffer_t = text::buffer_t<enc>;

    buffer_t m_buf;
    char32_t m_curr_codepoint = text::null_codepoint; // Current extracted character
    std::size_t m_line = 1; // Current line number

 public:
    ParserBase(const std::string_view bytes) noexcept
      : m_buf(bytes)
       {
       }

    ParserBase(const ParserBase&) = delete; // Prevent copy
    ParserBase(ParserBase&&) = delete; // Prevent move
    ParserBase& operator=(const ParserBase&) = delete; // Prevent copy assignment
    ParserBase& operator=(ParserBase&&) = delete; // Prevent move assignment
    ~ParserBase() = default; // Yes, destructor is not virtual


    //-----------------------------------------------------------------------
    //[[nodiscard]] bool has_data() const noexcept { return m_buf.has_bytes(); }
    [[nodiscard]] char32_t curr_codepoint() const noexcept { return m_curr_codepoint; }
    [[nodiscard]] std::size_t curr_line() const noexcept { return m_line; }


 protected:
    //-----------------------------------------------------------------------
    void notify_issue(const std::string_view msg) const noexcept
       {
        fnotify( fmt::format("{} (line {}, offset {})", msg, m_line, m_buf.byte_pos()) );
       }
    parse_error create_parse_error(const std::string_view msg) const noexcept
       {
        return parse_error(msg, m_line, m_buf.byte_pos());
       }
    //parse_error create_parse_error(const std::string_view msg, const std::size_t lin, const std::size_t off) const noexcept
    //   {
    //    return parse_error(msg, lin, off);
    //   }


    //-----------------------------------------------------------------------
    // Extract next codepoint from buffer
    [[nodiscard]] bool get_next()
       {
        if( m_buf.has_codepoint() ) [[likely]]
           {
            m_curr_codepoint = m_buf.extract_codepoint();
            // Detect line increment? Nah, use eat_line_end()
            //if( text::is_endline(m_curr_codepoint) ) ++m_line;
            return true;
           }
        else if( m_buf.has_bytes() )
           {// Truncated codepoint!
            m_curr_codepoint = text::err_codepoint;
            throw create_parse_error("Truncated codepoint");
           }
        m_curr_codepoint = text::null_codepoint;
        return false;
       }

    //-----------------------------------------------------------------------
    // Querying current codepoint
    [[nodiscard]] bool is(const char32_t cp) noexcept
       {
        return m_curr_codepoint == cp;
       }
    [[nodiscard]] bool is_endline() noexcept
       {
        return text::is_endline(m_curr_codepoint);
       }
    [[nodiscard]] bool is_space() noexcept
       {
        return text::is_space(m_curr_codepoint);
       }
    [[nodiscard]] bool is_blank() noexcept
       {
        return text::is_blank(m_curr_codepoint);
       }
    [[nodiscard]] bool is_digit() noexcept
       {
        return text::is_digit(m_curr_codepoint);
       }

    //-----------------------------------------------------------------------
    // Skip spaces except new line
    void skip_blanks() noexcept
       {
        while( is_blank() && get_next() ) ;
       }

    //-----------------------------------------------------------------------
    // Skip any space, including new line
    void skip_any_space() noexcept
       {
        while( is_space() )
           {
            if( is_endline() ) ++m_line;
            get_next();
           }
       }

    //-----------------------------------------------------------------------
    [[maybe_unused]] bool eat_line_end() noexcept
       {
        if( is_endline() )
           {
            ++m_line;
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    // Skip empty lines
    void skip_empty_lines() noexcept
       {
        do{
           skip_blanks();
          }
        while( eat_line_end() );
       }

    //-----------------------------------------------------------------------
    //void ensure_line_ended_here()
    //   {
    //    skip_blanks();
    //    if( !eat_line_end() )
    //       {
    //        throw create_parse_error("Unexpected content at line end");
    //       }
    //   }

    //-----------------------------------------------------------------------
    void skip_line() noexcept
       {
        while( eat_line_end() )
           {
            get_next();
           }
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] bool eat(const char32_t cp) noexcept
       {
        if( is(cp) )
           {
            get_next();
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    // Read a (base10) positive integer literal
    [[nodiscard]] std::size_t extract_index()
       {
        if( !is_digit() )
           {
            throw create_parse_error(fmt::format("Invalid char \'{}\' in index", curr_codepoint()));
           }

        std::size_t result = (curr_codepoint()-U'0');
        const std::size_t base = 10u;
        while( get_next() && is_digit() )
           {
            result = (base*result) + (curr_codepoint()-U'0');
           }
        return result;
       }
};

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



// TESTS

