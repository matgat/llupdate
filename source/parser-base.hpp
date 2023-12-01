#ifndef GUARD_parser_base_hpp
#define GUARD_parser_base_hpp
//  ---------------------------------------------
//  Common encoding agnostic parsing facilities
//  ---------------------------------------------
#include <stdexcept> // std::exception, std::runtime_error, ...
#include <vector>
#include <string_view>
#include <fmt/core.h> // fmt::format

#include "text.hpp" // text::buffer_t
#include "debug.hpp" // DLOG#


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


/////////////////////////////////////////////////////////////////////////////
template<typename buf_t> class Parser_Base
{
 protected:
    const char* const buf;
    const std::size_t siz; // buffer size
    const std::size_t i_last; // index of the last character
    std::size_t i; // Current character
    std::size_t line; // Current line number
    std::vector<std::string>& issues; // Problems found

 public:
    Parser_Base(const text::buffer_t& buf,
                std::vector<std::string>& lst )
      : buf(dat.data())
      , siz(dat.size())
      , i_last(siz-1u) // siz>0
      , line(1)
      , i(0)
      , issues(lst)
       {
        if( i>=siz )
           {
            throw std::runtime_error("No data to parse (empty file?)");
           }
       }

    Parser_Base(const Parser_Base&) = delete; // Prevent copy
    Parser_Base(Parser_Base&&) = delete; // Prevent move
    Parser_Base& operator=(const Parser_Base&) = delete; // Prevent copy assignment
    Parser_Base& operator=(Parser_Base&&) = delete; // Prevent move assignment
    ~Parser_Base() = default; // Yes, destructor is not virtual


    //-----------------------------------------------------------------------
    [[nodiscard]] bool has_data() const noexcept { return i<siz; }
    [[nodiscard]] std::size_t curr_line() const noexcept { return line; }
    [[nodiscard]] std::size_t curr_pos() const noexcept { return i; }


    //-----------------------------------------------------------------------
    void notify_issue(const std::string_view msg) const noexcept
       {
        issues.push_back( fmt::format("{} (line {}, offset {})", msg, line, i) );
       }
    parse_error create_parse_error(const std::string_view msg) const noexcept
       {
        return parse_error(msg, line, i<=i_last ? i : i_last);
       }
    parse_error create_parse_error(const std::string_view msg, const std::size_t lin, const std::size_t off) const noexcept
       {
        return parse_error(msg, lin, off<=i_last ? off : i_last);
       }

 protected:

    //-----------------------------------------------------------------------
    [[nodiscard]] static bool is_identifier(const char ch) noexcept
       {
        return std::isalnum(ch) || ch=='_';
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] static bool is_numeric_literal(const char ch) noexcept
       {
        return std::isdigit(ch) || ch=='+' || ch=='-' || ch=='.' || ch=='E';
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] static bool is_blank(const char ch) noexcept
       {
        return std::isspace(ch) && ch!='\n';
       }

    //-----------------------------------------------------------------------
    // Skip space chars except new line
    void skip_blanks() noexcept
       {
        while( i<siz && is_blank(buf[i]) ) ++i;
       }

    //-----------------------------------------------------------------------
    // Skip any space, including new line
    void skip_any_space() noexcept
       {
        while( i<siz && std::isspace(buf[i]) )
           {
            if( buf[i]=='\n' ) ++line;
            ++i;
           }
       }

    //-----------------------------------------------------------------------
    // Tell if skipped any space, including new line
    //[[nodiscard]] bool eat_any_space() noexcept
    //   {
    //    assert(i<siz);
    //    if( std::isspace(buf[i]) )
    //       {
    //        do {
    //            if( buf[i]=='\n' ) ++line;
    //            ++i;
    //           }
    //        while( i<siz && std::isspace(buf[i]) );
    //        return true;
    //       }
    //    return false;
    //   }

    //-----------------------------------------------------------------------
    [[maybe_unused]] bool eat_line_end() noexcept
       {
        assert(i<siz);
        if( buf[i]=='\n' )
           {
            ++i;
            ++line;
            return true;
           }
        return false;
       }


    //-----------------------------------------------------------------------
    // Skip empty lines
    void skip_empty_lines() noexcept
       {
        do{ skip_blanks(); }
        while( eat_line_end() );
       }


    //-----------------------------------------------------------------------
    void check_if_line_ended_after(const std::string_view fmt_str, const std::string_view fmt_arg)
       {
        skip_blanks();
        if( !eat_line_end() )
           {
            notify_issue("Unexpected content after {}: {}", fmt::format(fmt::runtime(fmt_str), fmt_arg), str::escape(skip_line()));
           }
       }


    //-----------------------------------------------------------------------
    [[maybe_unused]] std::string_view skip_line() noexcept
       {
        // Intercept the edge case already at buffer end:
        if(i>i_last) return std::string_view(buf+i_last, 0);

        const std::size_t i_start = i;
        while( i<siz && !eat_line_end() ) ++i;
        return std::string_view(buf+i_start, i-i_start);
        // Note: If '\n' not found returns what remains in buf and i==siz
       }


    //-----------------------------------------------------------------------
    //void skip_line_check_no_further_content()
    //   {
    //    while( i<siz && !eat_line_end() )
    //       {
    //        if( !std::isspace(buf[i]) )
    //           {
    //            notify_issue("Unexpected content: {}", str::escape(skip_line()));
    //           }
    //        ++i;
    //       }
    //   }


    //-----------------------------------------------------------------------
    [[nodiscard]] bool eat(const char ch) noexcept
       {
        assert(i<siz);
        if( buf[i]==ch )
           {
            ++i; // Skip ch
            return true;
           }
        return false;
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] bool eat(const std::string_view s) noexcept
       {
        const std::size_t i_end = i+s.length();
        if( i_end<=siz && s==std::string_view(buf+i,s.length()) )
           {
            i = i_end;
            return true;
           }
        return false;
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] bool eat_token(const std::string_view s) noexcept
       {
        const std::size_t i_end = i+s.length();
        if( ((i_end<siz && !std::isalnum(buf[i_end])) || i_end==siz) && s==std::string_view(buf+i,s.length()) )
           {
            i = i_end;
            return true;
           }
        return false;
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_token() noexcept
       {
        assert(i<siz);
        const std::size_t i_start = i;
        while( i<siz && !std::isspace(buf[i]) ) ++i;
        return std::string_view(buf+i_start, i-i_start);
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_identifier() noexcept
       {
        assert(i<siz);
        const std::size_t i_start = i;
        while( i<siz && is_identifier(buf[i]) ) ++i;
        return std::string_view(buf+i_start, i-i_start);
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_numeric_value() noexcept
       {
        assert(i<siz);
        const std::size_t i_start = i;
        while( i<siz && is_numeric_literal(buf[i]) ) ++i;
        return std::string_view(buf+i_start, i-i_start);
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_digits() noexcept
       {
        assert(i<siz);
        const std::size_t i_start = i;
        while( i<siz && std::isdigit(buf[i]) ) ++i;
        return std::string_view(buf+i_start, i-i_start);
       }


    //-----------------------------------------------------------------------
    // Read a (base10) positive integer literal
    [[nodiscard]] std::size_t extract_index()
       {
        if( i>=siz )
           {
            throw create_parse_error("Index not found");
           }

        if( buf[i]=='+' )
           {
            ++i;
            if( i>=siz )
               {
                throw create_parse_error("Invalid index \'+\'");
               }
           }
        else if( buf[i]=='-' )
           {
            throw create_parse_error("Index can\'t be negative");
           }

        if( !std::isdigit(buf[i]) )
           {
            throw create_parse_error(fmt::format("Invalid char \'{}\' in index", buf[i]));
           }

        std::size_t result = (buf[i]-'0');
        const std::size_t base = 10u;
        while( ++i<siz && std::isdigit(buf[i]) )
           {
            result = (base*result) + (buf[i]-'0');
           }
        return result;
       }


    //-----------------------------------------------------------------------
    // Read a (base10) integer literal
    [[nodiscard]] int extract_integer()
       {
        if( i>=siz )
           {
            throw create_parse_error("No integer found");
           }
        int sign = 1;
        if( buf[i]=='+' )
           {
            //sign = 1;
            ++i;
            if( i>=siz )
               {
                throw create_parse_error("Invalid integer \'+\'");
               }
           }
        else if( buf[i]=='-' )
           {
            sign = -1;
            ++i;
            if( i>=siz )
               {
                throw create_parse_error("Invalid integer \'-\'");
               }
           }
        if( !std::isdigit(buf[i]) )
           {
            throw create_parse_error(fmt::format("Invalid char \'{}\' in integer", buf[i]));
           }
        int result = (buf[i]-'0');
        const int base = 10;
        while( ++i<siz && std::isdigit(buf[i]) )
           {
            result = (base*result) + (buf[i]-'0');
           }
        return sign * result;
       }


    //-----------------------------------------------------------------------
    //[[nodiscard]] std::string_view collect_until_char(const char ch)
    //   {
    //    const std::size_t i_start = i;
    //    while( i<siz )
    //       {
    //        if( buf[i]==ch )
    //           {
    //            //DLOG1("    [*] Collected \"{}\" at line {}\n", std::string_view(buf+i_start, i-i_start), line)
    //            return std::string_view(buf+i_start, i-i_start);
    //           }
    //        else if( buf[i]=='\n' ) ++line;
    //        ++i;
    //       }
    //    throw create_parse_error(fmt::format("Unclosed content (\'{}\' expected)", str::escape(ch)));
    //   }


    //-----------------------------------------------------------------------
    //[[nodiscard]] std::string_view collect_until_char_same_line(const char ch)
    //   {
    //    const std::size_t i_start = i;
    //    while( i<siz )
    //       {
    //        if( buf[i]==ch )
    //           {
    //            //DLOG1("    [*] Collected \"{}\" at line {}\n", std::string_view(buf+i_start, i-i_start), line)
    //            return std::string_view(buf+i_start, i-i_start);
    //           }
    //        else if( buf[i]=='\n' ) break;
    //        ++i;
    //       }
    //    throw create_parse_error(fmt::format("Unclosed content (\'{}\' expected)", str::escape(ch)));
    //   }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_until_char_trimmed(const char ch)
       {
        const std::size_t line_start = line; // Store current line...
        const std::size_t i_start = i;       // ...and position
        std::size_t i_end = i_start; // Index past last char not blank
        while( i<siz )
           {
            if( buf[i]==ch )
               {
                //++i; // Nah, do not eat ch
                return std::string_view(buf+i_start, i_end-i_start);
               }
            else if( buf[i]=='\n' )
               {
                ++line;
                ++i;
               }
            else
               {
                if( !is_blank(buf[i]) ) i_end = ++i;
                else ++i;
               }
           }
        throw create_parse_error(fmt::format("Unclosed content (\'{}\' expected)", str::escape(ch)), line_start, i_start);
       }


    //-----------------------------------------------------------------------
    //[[nodiscard]] std::string_view collect_until_token(const std::string_view tok)
    //   {
    //    const std::size_t i_start = i;
    //    const std::size_t max_siz = siz-tok.length();
    //    while( i<max_siz )
    //       {
    //        if( buf[i]==tok[0] && eat_token(tok) )
    //           {
    //            return std::string_view(buf+i_start, i-i_start-tok.length());
    //           }
    //        else if( buf[i]=='\n' ) ++line;
    //        ++i;
    //       }
    //    throw create_parse_error(fmt::format("Unclosed content (\"{}\" expected)",tok), line_start, i_start);
    //   }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_until_newline_token(const std::string_view tok)
       {
        const std::size_t line_start = line;
        const std::size_t i_start = i;
        while( i<siz )
           {
            if( buf[i]=='\n' )
               {
                ++i;
                ++line;
                skip_blanks();
                if( eat_token(tok) )
                   {
                    return std::string_view(buf+i_start, i-i_start-tok.length());
                   }
               }
            else ++i;
           }
        throw create_parse_error(fmt::format("Unclosed content (\"{}\" expected)",tok), line_start, i_start);
       }
};



//---- end unit -------------------------------------------------------------
#endif
