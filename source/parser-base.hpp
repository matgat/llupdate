#pragma once
//  ---------------------------------------------
//  Common encoding agnostic parsing facilities
//  ---------------------------------------------
#include <cassert>
#include <stdexcept> // std::exception, std::runtime_error
#include <concepts>
#include <functional> // std::function
#include <string>
#include <string_view>
using namespace std::literals; // "..."sv
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
template<text::Enc enc>
class ParserBase final
{
    using buffer_t = text::buffer_t<enc>;
    using fnotify_t = std::function<void(const std::string_view)>;

    struct context_t final
       {
        std::size_t line;
        std::size_t offset;
        std::size_t last_codepoint_byte_offset;
        char32_t curr_codepoint;
        buffer_t::context_t buf_context;
       };
    constexpr context_t save_context() const noexcept
       {
        return { m_line, m_offset, m_last_codepoint_byte_offset, m_curr_codepoint, m_buf.save_context() };
       }
    constexpr void restore_context(const context_t context) noexcept
       {
        m_line = context.line;
        m_offset = context.offset;
        m_last_codepoint_byte_offset = context.last_codepoint_byte_offset;
        m_curr_codepoint = context.curr_codepoint;
        m_buf.restore_context( context.buf_context );
       }

 private:
    buffer_t m_buf;
    std::size_t m_line = 1; // Current line number
    std::size_t m_offset = 0; // Index of next extracted codepoint
    std::size_t m_last_codepoint_byte_offset = 0; // Index of the first byte of the last extracted codepoint
    char32_t m_curr_codepoint = text::null_codepoint; // Current extracted character
    fnotify_t m_on_notify_issue = default_notify;

 public:
    explicit constexpr ParserBase(const std::string_view bytes) noexcept
      : m_buf(bytes)
       {}

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr bool has_bytes() const noexcept { return m_buf.has_bytes(); }
    [[nodiscard]] constexpr std::size_t curr_line() const noexcept { return m_line; }
    [[nodiscard]] constexpr std::size_t curr_offset() const noexcept { return m_offset; }
    [[nodiscard]] constexpr std::size_t curr_byte_offset() const noexcept { return m_buf.byte_pos(); }
    [[nodiscard]] constexpr char32_t curr_codepoint() const noexcept { return m_curr_codepoint; }

 public:
    //-----------------------------------------------------------------------
    static constexpr void default_notify([[maybe_unused]] const std::string_view msg) {} // { fmt::print(fmt::runtime(msg)); }
    constexpr void set_on_notify_issue(const fnotify_t& f) { m_on_notify_issue = f; }
    constexpr void notify_issue(const std::string_view msg) const
       {
        m_on_notify_issue( fmt::format("{} (line {} offset {})"sv, msg, m_line, m_offset) );
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
            m_last_codepoint_byte_offset = m_buf.byte_pos();
            if( text::is_endline(m_curr_codepoint) ) ++m_line;
            m_curr_codepoint = m_buf.extract_codepoint();
            ++m_offset;
            return true;
           }
        else if( m_buf.has_bytes() )
           {// Truncated codepoint!
            m_curr_codepoint = text::err_codepoint;
            m_buf.set_as_depleted();
            notify_issue("! Truncated codepoint"sv);
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
    [[nodiscard]] constexpr bool got(const char32_t cp) noexcept
       {
        return m_curr_codepoint==cp;
       }
    [[nodiscard]] constexpr bool isnt(const char32_t cp) noexcept
       {
        return m_curr_codepoint!=cp;
       }
    [[nodiscard]] constexpr bool got_endline() noexcept
       {
        return text::is_endline(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool got_space() noexcept
       {
        return text::is_space(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool got_blank() noexcept
       {
        return text::is_blank(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool got_digit() noexcept
       {
        return text::is_digit(m_curr_codepoint);
       }
    [[nodiscard]] constexpr bool got_punct() noexcept
       {
        return text::is_punct(m_curr_codepoint);
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr bool eat(const char32_t cp) noexcept
       {
        if( got(cp) )
           {
            [[maybe_unused]] const bool has_next = get_next();
            return true;
           }
        return false;
       }

    //-----------------------------------------------------------------------
    [[maybe_unused]] constexpr bool eat_endline() noexcept
       {
        if( got_endline() )
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
        while( got_blank() and get_next() ) ;
       }

    //-----------------------------------------------------------------------
    // Skip any space, including new line
    constexpr void skip_any_space() noexcept // aka skip_empty_lines()
       {
        while( got_space() and get_next() ) ;
       }

    //-----------------------------------------------------------------------
    constexpr void skip_line() noexcept
       {
        while( !got_endline() and get_next() ) ;
        [[maybe_unused]] const bool has_next = get_next(); // Skip also line end character
       }

    //-----------------------------------------------------------------------
    // Line should end: nothing more than spaces allowed
    constexpr void skip_endline()
       {
        skip_blanks();
        if( not eat_endline() )
           {
            throw create_parse_error(fmt::format("Unexpected content '{}' at line end"sv, text::to_utf8(curr_codepoint())));
           }
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr bool eat(const std::u32string_view sv) noexcept
       {
        assert( !sv.empty() );
        if( got(sv[0]) )
           {
            const auto context = save_context();
            std::size_t i = 0;
            while( get_next() )
               {
                if( ++i>=sv.size() )
                   {
                    return true;
                   }
                else if( not got(sv[i]) )
                   {
                    break;
                   }
               }
            restore_context( context );
           }
        return false;
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr std::string_view collect_bytes_until(const std::u32string_view end_block)
       {
        const auto start = save_context();
        std::size_t content_end_byte_pos = start.last_codepoint_byte_offset;
        std::size_t i = 0; // Matching codepoint index
        do {
            if( got(end_block[i]) )
               {// Matches a codepoint in end_block, is it the last?
                // If it's the first of end_block...
                if( i==0 )
                   {// ...Store the offset of the possible end of collected content
                    content_end_byte_pos = m_last_codepoint_byte_offset;
                   }
                // If it's the last of end_block...
                if( ++i>=end_block.size() )
                   {// ...I'm done
                    [[maybe_unused]] const bool has_next = get_next(); // Skip last end_block codepoint
                    return m_buf.get_slice_between(start.last_codepoint_byte_offset, content_end_byte_pos);
                   }
               }
            else if( i>0 )
               {// There was a partial match, reset it
                i = 0;
                if( got(end_block[0]) )
                   {// Matches the first, could it be the one?
                    ++i;
                    content_end_byte_pos = m_last_codepoint_byte_offset;
                   }
               }
           }
        while( get_next() ); [[likely]]

        restore_context( start ); // Strong guarantee
        throw create_parse_error( fmt::format("Should be closed by {}"sv, text::to_utf8(end_block)) );
       }


    //-----------------------------------------------------------------------
    [[nodiscard]] static constexpr bool is_always_false(const char32_t) noexcept { return false; }
    //-----------------------------------------------------------------------
    // auto bytes = parser.collect_bytes_until(text::is_any_of<U'=',U':'>, text::is_endline);
    template<std::predicate<const char32_t> CodepointPredicate =decltype(is_always_false)>
    [[nodiscard]] constexpr std::string_view collect_bytes_until(CodepointPredicate is_end, CodepointPredicate is_unexpected =is_always_false)
       {
        const auto start = save_context();
        do {
            if( is_end(curr_codepoint()) ) [[unlikely]]
               {
                std::string_view collected = m_buf.get_slice_between(start.last_codepoint_byte_offset, m_last_codepoint_byte_offset);
                [[maybe_unused]] const bool has_next = get_next(); // Skip end codepoint
                return collected;
               }
            else if( is_unexpected(curr_codepoint()) ) [[unlikely]]
               {
                break;
               }
           }
        while( get_next() ); [[likely]]

        restore_context( start ); // Strong guarantee
        throw create_parse_error( has_codepoint() ? fmt::format("Unexpected character '{}'"sv, text::to_utf8(curr_codepoint()))
                                                  : "Unexpected end (termination not found)"s );
       }
    //-----------------------------------------------------------------------
    template<char32_t end_codepoint>
    [[nodiscard]] constexpr std::string_view collect_bytes_until()
       {
        return collect_bytes_until(text::is<end_codepoint>, is_always_false);
       }

    //-----------------------------------------------------------------------
    template<std::predicate<const char32_t> CodepointPredicate =decltype(is_always_false)>
    [[nodiscard]] constexpr std::u32string collect_until(CodepointPredicate is_end, CodepointPredicate is_unexpected =is_always_false)
       {
        std::u32string collected;
        const auto start = save_context();
        do {
            if( is_end(curr_codepoint()) ) [[unlikely]]
               {
                [[maybe_unused]] const bool has_next = get_next(); // Skip end codepoint
                return collected;
               }
            else if( is_unexpected(curr_codepoint()) ) [[unlikely]]
               {
                break;
               }
            else
               {
                collected.push_back( curr_codepoint() );
               }
           }
        while( get_next() ); [[likely]]

        restore_context( start ); // Strong guarantee
        throw create_parse_error( has_codepoint() ? fmt::format("Unexpected character '{}'"sv, text::to_utf8(curr_codepoint()))
                                                  : "Unexpected end (termination not found)"s );
       }
    //-----------------------------------------------------------------------
    template<char32_t end_codepoint>
    [[nodiscard]] constexpr std::u32string collect_until()
       {
        return collect_until(text::is<end_codepoint>, is_always_false);
       }


    //-----------------------------------------------------------------------
    // Read a (base10) positive integer literal
    [[nodiscard]] constexpr std::size_t extract_index()
       {
        if( not got_digit() )
           {
            throw create_parse_error(fmt::format("Invalid char '{}' in index"sv, text::to_utf8(curr_codepoint())));
           }

        std::size_t result = (curr_codepoint()-U'0');
        constexpr std::size_t base = 10u;
        while( get_next() and got_digit() )
           {
            //assert( result < (std::numeric_limits<std::size_t>::max - (curr_codepoint()-U'0')) / base ); // Check overflows
            result = (base*result) + (curr_codepoint()-U'0');
           }
        return result;
       }
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

        expect( parser.get_next() and parser.got(U'a') );
        expect( parser.get_next() and parser.got(U'à') );
        expect( parser.get_next() and parser.got(U' ') );
        expect( parser.get_next() and parser.got(U'±') );
        expect( parser.get_next() and parser.got(U'∆') );
        expect( !parser.get_next() and parser.got(text::null_codepoint) );
       };


    ut::test("simple utf-16le") = []
       {
        MG::ParserBase<UTF16LE> parser{"\x61\x00\xE0\x00\x20\x00\xB1\x00\x06\x22"sv}; // u"aà ±∆"

        expect( parser.get_next() and parser.got(U'a') );
        expect( parser.get_next() and parser.got(U'à') );
        expect( parser.get_next() and parser.got(U' ') );
        expect( parser.get_next() and parser.got(U'±') );
        expect( parser.get_next() and parser.got(U'∆') );
        expect( !parser.get_next() and parser.got(text::null_codepoint) );
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

        expect( parser.get_next() and parser.got(U'1') );
        parser.skip_blanks();
        parser.skip_any_space();
        expect( !parser.got_endline() and parser.got(U'1') and parser.curr_line()==1 );
        expect( parser.get_next() and parser.got_endline() and parser.curr_line()==1 );
        expect( parser.eat_endline() and parser.curr_line()==2 );

        expect( parser.got(U'2') );
        expect( parser.get_next() and parser.got_blank() );
        parser.skip_blanks();
        expect( parser.eat_endline() and parser.curr_line()==3 );

        expect( parser.got(U'3') and parser.curr_line()==3 and parser.get_next() );
        parser.skip_any_space();

        expect( parser.got(U'4') and parser.curr_line()==4 );
        parser.skip_line();

        expect( parser.got(U'5') and parser.curr_line()==5 and parser.get_next() );
        parser.skip_endline();

        expect( parser.got(U'6') and parser.curr_line()==6 and parser.get_next() );
        parser.skip_blanks();
        expect( parser.got(U'F') and parser.curr_line()==6 and parser.get_next() );
        parser.skip_any_space();

        expect( parser.got(U'9') and parser.curr_line()==9 );
        parser.skip_line();

        expect( !parser.has_bytes() and parser.curr_line()==9 );
       };


    ut::test("parse utilities") = [&notify_sink]
       {
        MG::ParserBase<UTF8> parser
           {
            "abc123\n"
            "<tag>a=\"\" b=\"str\"</tag>\n"
            "/*///\n"
            "---\n"
            "****/end\n"
            "blah <!--ooo\n"
            "---\n"
            "-->"sv
           };
        parser.set_on_notify_issue(notify_sink);
        expect( parser.get_next() );

        expect( !parser.eat(U"abb") and !parser.eat(U"abcd") and parser.eat(U"abc") and parser.got(U'1') and parser.curr_line()==1u );
        expect( parser.eat(U'1') and !parser.eat(U'3') and parser.eat(U"2") and parser.eat(U'3') and parser.got(U'\n') and parser.curr_line()==1u );
        expect( parser.eat(U'\n') and parser.curr_line()==2u );

        expect( parser.eat(U'<') and parser.curr_line()==2u );
        expect( throws<MG::parse_error>([&parser] { [[maybe_unused]] auto n = parser.collect_bytes_until<U'☺'>(); }) ) << "missing closing character should throw\n";
        expect( parser.collect_bytes_until<U'>'>()=="tag"sv and parser.curr_line()==2u and parser.got(U'a') );
        expect( parser.eat(U"a=\"") );
        expect( throws<MG::parse_error>([&parser] { [[maybe_unused]] auto n = parser.collect_until(text::is<U'*'>, text::is_endline); }) ) << "missing closing character in same line should throw\n";
        expect( parser.collect_until(text::is<U'\"'>, text::is_endline)==U""sv and parser.got(U' ') );

        parser.skip_blanks();
        expect( parser.eat(U"b=\"") );
        expect( parser.collect_until(text::is<U'\"'>, text::is_endline)==U"str"sv and parser.eat(U"</"sv) );
        expect( parser.collect_bytes_until(text::is<U'>'>)=="tag"sv and parser.eat_endline() );

        expect( parser.eat(U"/*") and parser.curr_line()==3u );
        expect( parser.collect_bytes_until(U"*/")=="///\n---\n***"sv );
        expect( parser.eat(U"end") );

        expect( parser.collect_bytes_until(U"<!--")=="\nblah "sv and parser.got(U'o') );
        expect( parser.collect_bytes_until(U"-->")=="ooo\n---\n"sv and !parser.has_bytes() );
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

        expect( parser.get_next() and parser.eat(U'a') and parser.eat(U'=') );
        expect( that % parser.extract_index()==1234u );
        expect( parser.got(U'm') and parser.curr_line()==1u );
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
