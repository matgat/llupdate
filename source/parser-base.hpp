#pragma once
//  ---------------------------------------------
//  Common encoding agnostic parsing facilities
//  ---------------------------------------------
#include <cassert>
#include <concepts>
#include <stdexcept> // std::exception, std::runtime_error, ...
#include <string>
#include <string_view>
#include <functional> // std::function
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
    explicit parse_error(std::string&& msg, const std::size_t lin) noexcept
       : m_msg(msg)
       , m_line(lin)
        {}

    std::size_t line() const noexcept { return m_line; }

    const char* what() const noexcept override { return m_msg.c_str(); } // Could rise a '-Wweak-vtables'
};




/////////////////////////////////////////////////////////////////////////////
template<text::Enc enc> class ParserBase
{
    using buffer_t = text::buffer_t<enc>;
    using fnotify_t = std::function<void(const std::string_view)>;

    buffer_t m_buf;
    char32_t m_curr_codepoint = text::null_codepoint; // Current extracted character
    std::size_t m_line = 1; // Current line number
    fnotify_t m_on_notify_issue = default_notify;

 public:
    explicit ParserBase(const std::string_view bytes) noexcept
      : m_buf(bytes)
       {}

    ParserBase(const ParserBase&) = delete; // Prevent copy
    ParserBase(ParserBase&&) = delete; // Prevent move
    ParserBase& operator=(const ParserBase&) = delete; // Prevent copy assignment
    ParserBase& operator=(ParserBase&&) = delete; // Prevent move assignment
    ~ParserBase() = default; // Yes, destructor is not virtual


    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr char32_t curr_codepoint() const noexcept { return m_curr_codepoint; }
    [[nodiscard]] constexpr std::size_t curr_line() const noexcept { return m_line; }
    [[nodiscard]] constexpr bool has_bytes() const noexcept { return m_buf.has_bytes(); }
    //[[nodiscard]] constexpr std::size_t curr_byte_offset() const noexcept { return m_buf.byte_pos(); }

#ifdef TEST_UNITS
 public:
#else
 protected:
#endif
    //-----------------------------------------------------------------------
    //static constexpr void default_notify(const std::string_view msg) { fmt::print( fmt::runtime(msg)); }
    static constexpr void default_notify(const std::string_view) {}
    constexpr void set_on_notify_issue(const fnotify_t& f) { m_on_notify_issue = f; }
    constexpr void notify_issue(const std::string_view msg) const
       {
        m_on_notify_issue( fmt::format("{} (line {}, offset {})", msg, m_line, m_buf.byte_pos()) );
       }
    parse_error create_parse_error(std::string&& msg) const noexcept
       {
        return parse_error(std::move(msg), m_line);
       }

    //-----------------------------------------------------------------------
    // Extract next codepoint from buffer
    [[nodiscard]] constexpr bool get_next() noexcept
       {
        if( m_buf.has_codepoint() ) [[likely]]
           {
            if( text::is_endline(m_curr_codepoint) ) ++m_line;
            m_curr_codepoint = m_buf.extract_codepoint();
            return true;
           }
        else if( m_buf.has_bytes() )
           {// Truncated codepoint!
            m_curr_codepoint = text::err_codepoint;
            m_buf.set_as_depleted();
            notify_issue("! Truncated codepoint");
            return true;
           }
        m_curr_codepoint = text::null_codepoint;
        return false;
       }

    //-----------------------------------------------------------------------
    // Querying current codepoint
    [[nodiscard]] constexpr bool has_codepoint() noexcept
       {
        return m_curr_codepoint!=text::null_codepoint;
       }
    [[nodiscard]] constexpr bool is(const char32_t cp) noexcept
       {
        return m_curr_codepoint==cp;
       }
    [[nodiscard]] constexpr bool isnt(const char32_t cp) noexcept
       {
        return m_curr_codepoint!=cp;
       }
    [[nodiscard]] constexpr bool is_endline() noexcept
       {
        return text::is_endline(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool is_space() noexcept
       {
        return text::is_space(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool is_blank() noexcept
       {
        return text::is_blank(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool is_digit() noexcept
       {
        return text::is_digit(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool is_punct() noexcept
       {
        return text::is_punct(m_curr_codepoint);
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr bool eat(const char32_t cp) noexcept
       {
        if( is(cp) )
           {
            [[maybe_unused]] const bool has_next = get_next();
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    [[maybe_unused]] constexpr bool eat_endline() noexcept
       {
        if( is_endline() )
           {
            [[maybe_unused]] const bool has_next = get_next();
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    // Skip spaces except new line
    constexpr void skip_blanks() noexcept
       {
        while( is_blank() && get_next() ) ;
       }

    //-----------------------------------------------------------------------
    // Skip any space, including new line
    constexpr void skip_any_space() noexcept // aka skip_empty_lines()
       {
        while( is_space() && get_next() ) ;
       }

    //-----------------------------------------------------------------------
    constexpr void skip_line() noexcept
       {
        while( !is_endline() && get_next() ) ;
        [[maybe_unused]] const bool has_next = get_next(); // Skip also line end character
       }

    //-----------------------------------------------------------------------
    // Line should end: nothing more than spaces allowed
    constexpr void skip_endline()
       {
        skip_blanks();
        if( !eat_endline() )
           {
            throw create_parse_error("Unexpected content at line end");
           }
       }


    //-----------------------------------------------------------------------
    // Read a (base10) positive integer literal
    [[nodiscard]] constexpr std::size_t extract_index()
       {
        if( !is_digit() )
           {
            throw create_parse_error(fmt::format("Invalid char 0x{:X} in index", static_cast<std::uint32_t>(curr_codepoint())));
           }

        std::size_t result = (curr_codepoint()-U'0');
        constexpr std::size_t base = 10u;
        while( get_next() && is_digit() )
           {
            //assert( result < (std::numeric_limits<std::size_t>::max - (curr_codepoint()-U'0')) / base ); // Check overflows
            result = (base*result) + (curr_codepoint()-U'0');
           }
        return result;
       }

/*

    //-----------------------------------------------------------------------
    [[nodiscard]] bool eat(const std::u32string_view sv) noexcept
       {
        const std::size_t i_end = i+sv.size();
        if( i_end<=siz && sv==std::string_view(buf+i,sv.size()) )
           {
            i = i_end;
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    // Collect a delimited string (same line, excluding delims)
    MG::string_view collect_inner_same_line(const char ch_open, const char ch_close, const char chEscape)
       {
        assertm( i<siz && buf[i]==ch_open, "expecting initial delim" );
        ++i; // Skip ch_open
        const std::size_t i_start = i;
        while( i<siz )
           {
            if( buf[i]==chEscape )
               {// Do not check if next char is ch_close
                ++i; // Collect chEscape
                if(i>=siz) break; // No more data!
               }
            else if( buf[i]==ch_close )
               {
                ++i; // Skip ch_close
                return MG::string_view(buf+i_start, i-i_start-1);
               }
            else if( buf[i]=='\n' )
               {
                break;
               }
            ++i;
           }
        i = i_start; // Restore position (strong guarantee)
        throw parse_error( String().sprintf("Unclosed string (\'%c\' expected)",ch_close), m_filepath, line, i_start);
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_until( const char32_t ch )
       {
        const std::size_t i_start = i;
        std::size_t i_end = i_start; // Index past last char not space
        while( i<siz )
           {
            if( is(ch) )
               {
                break;
               }
            else if( buf[i]=='\n' )
               {
                ++line;
                ++i;
               }
            else if( !is_space() )
               {
                i_end = ++i;
               }
            else
               {
                ++i;
               }
           }
        return std::string_view(buf+i_start, i_end-i_start);
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_until(const char ch1, const char ch2, const char ch3)
       {
        const std::size_t line_start = line;
        const std::size_t i_start = i;
        const std::size_t i_max = siz-2; // Ultimo indice utile
        std::size_t i_end = i_start; // Index past last char not space

        while( i<i_max )
           {
            if( buf[i]==ch1 && buf[i+1]==ch2 && buf[i+2]==ch3 )
               {
                i += 3; // Eat closing sequence
                return std::string_view(buf+i_start, i_end-i_start);
               }
            else if( buf[i]=='\n' )
               {
                ++line;
                ++i;
               }
            else if( !is_space() )
               {
                i_end = ++i;
               }
            else
               {
                ++i;
               }
           }
        throw MG::parse_error( fmt::format("Unclosed block (%c%c%c expected)",ch1,ch2,ch3), file_path(), line_start, i_start);
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] std::string_view collect_until(const char ch1, const char ch2)
       {
        const std::size_t line_start = line;
        const std::size_t i_start = i;
        std::size_t i_end = i_start; // Index past last char not space

        while( i<i_last )
           {
            if( buf[i]==ch1 && buf[i+1]==ch2 )
               {
                i += 2; // Eat closing sequence
                return std::string_view(buf+i_start, i_end-i_start);
               }
            else if( buf[i]=='\n' )
               {
                ++line;
                ++i;
               }
            else if( !is_space() )
               {
                i_end = ++i;
               }
            else
               {
                ++i;
               }
           }
        throw MG::parse_error( fmt::format("Unclosed block (%c%c expected)",ch1,ch2), file_path(), line_start, i_start);
       }
*/

};

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::




/////////////////////////////////////////////////////////////////////////////
#ifdef TEST_UNITS ///////////////////////////////////////////////////////////
static ut::suite<"MG::ParserBase"> ParserBase_tests = []
{////////////////////////////////////////////////////////////////////////////
    using namespace std::literals; // "..."sv
    //using namespace ut::literals; // _ul
    using ut::expect;
    using ut::that;
    using ut::throws;
    using enum text::Enc;

    auto notify_sink = [](const std::string_view msg) -> void { ut::log << "parser: " << msg; };


    ut::test("empty") = []
       {
        MG::ParserBase<UTF8> parser{""sv};

        expect( not parser.get_next() and parser.curr_codepoint()==text::null_codepoint );
       };


    ut::test("simple utf-8") = []
       {
        MG::ParserBase<UTF8> parser{"\x61\xC3\xA0\x20\xC2\xB1\xE2\x88\x86"sv}; // u8"aà ±∆"

        expect( parser.get_next() and parser.is(U'a') );
        expect( parser.get_next() and parser.is(U'à') );
        expect( parser.get_next() and parser.is(U' ') );
        expect( parser.get_next() and parser.is(U'±') );
        expect( parser.get_next() and parser.is(U'∆') );
        expect( !parser.get_next() and parser.is(text::null_codepoint) );
       };


    ut::test("simple utf-16le") = []
       {
        MG::ParserBase<UTF16LE> parser{"\x61\x00\xE0\x00\x20\x00\xB1\x00\x06\x22"sv}; // u"aà ±∆"

        expect( parser.get_next() and parser.is(U'a') );
        expect( parser.get_next() and parser.is(U'à') );
        expect( parser.get_next() and parser.is(U' ') );
        expect( parser.get_next() and parser.is(U'±') );
        expect( parser.get_next() and parser.is(U'∆') );
        expect( !parser.get_next() and parser.is(text::null_codepoint) );
       };


    ut::test("eating spaces") = []
       {
        MG::ParserBase<UTF8> parser
           {
            "1\n"
            "2  \t\t  \r\n"
            "3 \t \n"
            "4 blah blah\r\n"
            "5\r\n"
            "6\t\t \r \t F\r\n"
            "   \n"
            "\t\t\n"
            "9 blah\n"sv
           };

        expect( parser.get_next() and parser.is(U'1') );
        parser.skip_blanks();
        parser.skip_any_space();
        expect( !parser.is_endline() and parser.is(U'1') and parser.curr_line()==1 );
        expect( parser.get_next() and parser.is_endline() and parser.curr_line()==1 );
        expect( parser.eat_endline() and parser.curr_line()==2 );

        expect( parser.is(U'2') );
        expect( parser.get_next() and parser.is_blank() );
        parser.skip_blanks();
        expect( parser.eat_endline() and parser.curr_line()==3 );

        expect( parser.is(U'3') and parser.curr_line()==3 and parser.get_next() );
        parser.skip_any_space();

        expect( parser.is(U'4') and parser.curr_line()==4 );
        parser.skip_line();

        expect( parser.is(U'5') and parser.curr_line()==5 and parser.get_next() );
        parser.skip_endline();

        expect( parser.is(U'6') and parser.curr_line()==6 and parser.get_next() );
        parser.skip_blanks();
        expect( parser.is(U'F') and parser.curr_line()==6 and parser.get_next() );
        parser.skip_any_space();

        expect( parser.is(U'9') and parser.curr_line()==9 );
        parser.skip_line();

        expect( !parser.has_bytes() and parser.curr_line()==9 );
       };


    ut::test("numbers") = [&notify_sink]
       {
        MG::ParserBase<UTF8> parser
           {
            "a=1234mm\n"
            "b=h1\n"
            "c=12"sv
           };
        parser.set_on_notify_issue(notify_sink);

        expect( parser.get_next() && parser.eat(U'a') and parser.eat(U'=') );
        expect( that % parser.extract_index()==1234u );
        expect( parser.is(U'm') and parser.curr_line()==1u );
        parser.skip_line();

        expect( parser.eat(U'b') and parser.eat(U'=') and parser.curr_line()==2 );
        expect( throws<MG::parse_error>([&parser] { [[maybe_unused]] auto n = parser.extract_index(); }) ) << "invalid index should throw MG::parse_error\n";
        expect( parser.eat(U'h') );
        expect( that % parser.extract_index()==1u );
        expect( parser.eat_endline() and parser.curr_line()==3u );

        expect( parser.eat(U'c') and parser.eat(U'=') );
        expect( that % parser.extract_index()==12u );
       };

};///////////////////////////////////////////////////////////////////////////
#endif // TEST_UNITS ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
