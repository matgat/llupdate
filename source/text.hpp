#pragma once
//  ---------------------------------------------
//  Encoding aware text utilities
//  ---------------------------------------------
#include <cassert>
#include <cstdint> // std::uint8_t, std::uint16_t, ...
#include <cctype> // std::isspace(), ...
#include <utility> // std::unreachable()
#include <string>
#include <string_view>


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace text
{

    namespace details
       {
        //-------------------------------------------------------------------
        [[nodiscard]] constexpr std::uint16_t combine_chars(const char h, const char l) noexcept
           {
            return (static_cast<unsigned char>(h) << 8) |
                    static_cast<unsigned char>(l);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr std::uint32_t combine_chars(const char hh, const char hl, const char lh, const char ll) noexcept
           {
            return (static_cast<unsigned char>(hh) << 24) |
                   (static_cast<unsigned char>(hl) << 16) |
                   (static_cast<unsigned char>(lh) << 8) |
                    static_cast<unsigned char>(ll);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char high_byte_of(const std::uint16_t word) noexcept
           {
            return static_cast<char>((word >> 8) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char low_byte_of(const std::uint16_t word) noexcept
           {
            return static_cast<char>(word & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char hh_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 24) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char hl_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 16) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char lh_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 8) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] constexpr char ll_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>(dword & 0xFF);
           }
       }

enum class Enc : std::uint8_t
   {
    //ANSI, // Nah, time to drop codepages
    UTF8 =0,
    UTF16LE,
    UTF16BE,
    UTF32LE,
    UTF32BE
   };

const char32_t err_codepoint = U'�'; // replacement character '\u{FFFD}'
const char32_t null_codepoint = 0;


//---------------------------------------------------------------------------
// auto [enc, bom_size] = text::detect_encoding_of(bytes);
struct bom_ret_t final { Enc enc; std::uint8_t bom_size; };
bom_ret_t constexpr detect_encoding_of(const std::string_view bytes)
   {//      +--------------+-------------+-------+
    //      |  Encoding    |   Bytes     | Chars |
    //      |--------------|-------------|-------|
    //      | UTF-8        | EF BB BF    | ï»¿   |
    //      | UTF-16 (LE)  | FF FE       | ÿþ    |
    //      | UTF-16 (BE)  | FE FF       | þÿ    |
    //      | UTF-32 (LE)  | FF FE 00 00 | ÿþ..  |
    //      | UTF-32 (BE)  | 00 00 FE FF | ..þÿ  |
    //      +--------------+-------------+-------+
    using enum Enc;
    if( bytes.size()>2 ) [[likely]]
       {
        if( bytes[0]=='\xFF' && bytes[1]=='\xFE' )
           {
            if( bytes.size()>=4 && bytes[2]=='\x00' && bytes[3]=='\x00' )
               {
                return {UTF32LE, 4};
               }
            else
               {
                return {UTF16LE, 2};
               }
           }
        else if( bytes[0]=='\xFE' && bytes[1]=='\xFF' )
           {
            return {UTF16BE, 2};
           }
        else if( bytes.size()>=4 && bytes[0]=='\x00' && bytes[1]=='\x00' && bytes[2]=='\xFE' && bytes[3]=='\xFF' )
           {
            return {UTF32BE, 4};
           }
        else if( bytes[0]=='\xEF' && bytes[1]=='\xBB' && bytes[2]=='\xBF' )
           {
            return {UTF8, 3};
           }
       }
    // Fallback
    return {UTF8, 0};
   }


//---------------------------------------------------------------------------
// Decode: Extract a codepoint according to encoding and endianness
template<Enc enc> constexpr char32_t extract_codepoint(const std::string_view bytes, std::size_t& pos) noexcept;

//---------------------------------------------------------------------------
//template<> constexpr char32_t extract_codepoint<Enc::ANSI>(const std::string_view bytes, std::size_t& pos) noexcept
//{
//    assert( pos<bytes.size() );
//
//    return bytes[pos++];
//}

//---------------------------------------------------------------------------
template<> constexpr char32_t extract_codepoint<Enc::UTF8>(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( pos<bytes.size() );

    if( (bytes[pos] & 0x80)==0 ) [[likely]]
       {
        const char32_t codepoint = bytes[pos];
        ++pos;
        return codepoint;
       }

    else if( (pos+1)<bytes.size() && (bytes[pos] & 0xE0)==0xC0 && (bytes[pos+1] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((bytes[pos] & 0x1F) << 6) | (bytes[pos+1] & 0x3F);
        pos += 2;
        return codepoint;
       }

    else if( (pos+2)<bytes.size() && (bytes[pos] & 0xF0)==0xE0 && (bytes[pos+1] & 0xC0)==0x80 && (bytes[pos+2] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((bytes[pos] & 0x0F) << 12) | ((bytes[pos+1] & 0x3F) << 6) | (bytes[pos+2] & 0x3F);
        pos += 3;
        return codepoint;
       }

    else if( (pos+3)<bytes.size() && (bytes[pos] & 0xF8)==0xF0 && (bytes[pos+1] & 0xC0)==0x80 && (bytes[pos+2] & 0xC0)==0x80 && (bytes[pos+3] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((bytes[pos] & 0x07) << 18) | ((bytes[pos+1] & 0x3F) << 12) | ((bytes[pos+2] & 0x3F) << 6) | (bytes[pos+3] & 0x3F);
        pos += 4;
        return codepoint;
       }

    // Invalid utf-8 character
    ++pos;
    return err_codepoint;
}

//---------------------------------------------------------------------------
template<bool LE> constexpr char32_t extract_next_codepoint_from_utf16(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( (pos+1)<bytes.size() );

    auto get_code_unit = [](const std::string_view buf, const std::size_t i) -> std::uint16_t
                           {
                            if constexpr(LE) return details::combine_chars(buf[i+1], buf[i]); // Little endian
                            else             return details::combine_chars(buf[i], buf[i+1]); // Big endian
                           };

    // If the first codeunit is in the intervals [0x0000–0xD800), [0xE000–0xFFFF]
    // then coincides with the codepoint itself (Basic Multilingual Plane)
    // Otherwise the whole codepoint is composed by two codeunits:
    // the first in the interval [0xD800-0xDC00), and the second in [0xDC00–0xE000)
    // codeunit1 = 0b110110yyyyyyyyyy // 0xD800 + yyyyyyyyyy [0xD800-0xDC00)
    // codeunit2 = 0b110111xxxxxxxxxx // 0xDC00 + xxxxxxxxxx [0xDC00–0xE000)
    // codepoint = 0x10000 + yyyyyyyyyyxxxxxxxxxx

    const std::uint16_t codeunit1 = get_code_unit(bytes, pos);
    pos += 2;

    if( codeunit1<0xD800 || codeunit1>=0xE000 ) [[likely]]
       {// Basic Multilingual Plane
        return codeunit1;
       }

    if( codeunit1>=0xDC00 || (pos+1)>=bytes.size() ) [[unlikely]]
       {// Not a first surrogate!
        return err_codepoint;
       }

    // Here expecting the second codeunit
    const std::uint16_t codeunit2 = get_code_unit(bytes, pos);
    if( codeunit2<0xDC00 || codeunit2>=0xE000 ) [[unlikely]]
       {// Not a second surrogate!
        return err_codepoint;
       }

    // Ok, I have the two valid codeunits
    pos += 2;
    return 0x10000 + ((codeunit1 - 0xD800) << 10) + (codeunit2 - 0xDC00);
}
template<> constexpr char32_t extract_codepoint<Enc::UTF16LE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<true>(bytes, pos);
}
template<> constexpr char32_t extract_codepoint<Enc::UTF16BE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<false>(bytes, pos);
}

//---------------------------------------------------------------------------
template<> constexpr char32_t extract_codepoint<Enc::UTF32LE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( (pos+3)<bytes.size() );

    const char32_t codepoint = details::combine_chars(bytes[pos+3], bytes[pos+2], bytes[pos+1], bytes[pos]); // Little endian
    pos += 4;
    return codepoint;
}

//---------------------------------------------------------------------------
template<> constexpr char32_t extract_codepoint<Enc::UTF32BE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( (pos+3)<bytes.size() );

    const char32_t codepoint = details::combine_chars(bytes[pos], bytes[pos+1], bytes[pos+2], bytes[pos+3]); // Big endian
    pos += 4;
    return codepoint;
}


//---------------------------------------------------------------------------
// Encode: Write a codepoint according to encoding and endianness
template<Enc enc> constexpr void append_codepoint(const char32_t codepoint, std::string& bytes) noexcept;

//---------------------------------------------------------------------------
//template<> constexpr void append_codepoint<Enc::ANSI>(const char32_t codepoint, std::string& bytes) noexcept
//{
//    bytes += static_cast<char>(codepoint); // Narrowing!
//}

//---------------------------------------------------------------------------
template<> constexpr void append_codepoint<Enc::UTF8>(const char32_t codepoint, std::string& bytes) noexcept
{
    if( codepoint<0x80 ) [[likely]]
       {
        bytes.push_back( static_cast<char>(codepoint) );
       }
    else if( codepoint<0x800 )
       {
        bytes.push_back( static_cast<char>(0xC0 | (codepoint >> 6) ));
        bytes.push_back( static_cast<char>(0x80 | (codepoint & 0x3F)) );
       }
    else if( codepoint<0x10000 )
       {
        bytes.push_back( static_cast<char>(0xE0 | (codepoint >> 12)) );
        bytes.push_back( static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | (codepoint & 0x3F)) );
       }
    else
       {
        bytes.push_back( static_cast<char>(0xF0 | (codepoint >> 18)) );
        bytes.push_back( static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | (codepoint & 0x3F)) );
       }
}

//---------------------------------------------------------------------------
// Encode a codepoint outside Basic Multilingual Plane
constexpr std::pair<std::uint16_t,std::uint16_t> encode_as_utf16(uint32_t codepoint) noexcept
{
    assert( codepoint>=0x10000 );
    codepoint -= 0x10000;
    return { static_cast<std::uint16_t>((codepoint >> 10) + 0xD800),
             static_cast<std::uint16_t>((codepoint & 0x3FF) + 0xDC00) };
}

//---------------------------------------------------------------------------
template<> constexpr void append_codepoint<Enc::UTF16LE>(const char32_t codepoint, std::string& bytes) noexcept
{
    if( codepoint<0x10000 ) [[likely]]
       {
        const std::uint16_t codeunit = static_cast<std::uint16_t>(codepoint);
        bytes.push_back( details::low_byte_of( codeunit) );
        bytes.push_back( details::high_byte_of( codeunit ) );
       }
    else
       {
        const auto codeunits = encode_as_utf16(codepoint);
        bytes.push_back( details::low_byte_of( codeunits.first ) );
        bytes.push_back( details::high_byte_of( codeunits.first ) );
        bytes.push_back( details::low_byte_of( codeunits.second ) );
        bytes.push_back( details::high_byte_of( codeunits.second ) );
       }
}

//---------------------------------------------------------------------------
template<> constexpr void append_codepoint<Enc::UTF16BE>(const char32_t codepoint, std::string& bytes) noexcept
{
    if( codepoint<0x10000 ) [[likely]]
       {
        const std::uint16_t codeunit = static_cast<std::uint16_t>(codepoint);
        bytes.push_back( details::high_byte_of( codeunit ) );
        bytes.push_back( details::low_byte_of( codeunit) );
       }
    else
       {
        const auto codeunits = encode_as_utf16(codepoint);
        bytes.push_back( details::high_byte_of( codeunits.first ) );
        bytes.push_back( details::low_byte_of( codeunits.first ) );
        bytes.push_back( details::high_byte_of( codeunits.second ) );
        bytes.push_back( details::low_byte_of( codeunits.second ) );
       }
}

//---------------------------------------------------------------------------
template<> constexpr void append_codepoint<Enc::UTF32LE>(const char32_t codepoint, std::string& bytes) noexcept
{
    bytes.push_back( details::ll_byte_of( codepoint ) );
    bytes.push_back( details::lh_byte_of( codepoint ) );
    bytes.push_back( details::hl_byte_of( codepoint ) );
    bytes.push_back( details::hh_byte_of( codepoint ) );
}

//---------------------------------------------------------------------------
template<> constexpr void append_codepoint<Enc::UTF32BE>(const char32_t codepoint, std::string& bytes) noexcept
{
    bytes.push_back( details::hh_byte_of( codepoint ) );
    bytes.push_back( details::hl_byte_of( codepoint ) );
    bytes.push_back( details::lh_byte_of( codepoint ) );
    bytes.push_back( details::ll_byte_of( codepoint ) );
}




/////////////////////////////////////////////////////////////////////////////
template<Enc ENC> class buffer_t final
{
 private:
    std::string_view m_byte_buf;
    std::size_t m_byte_pos = 0; // Index of currently pointed byte

 public:
    explicit constexpr buffer_t(const std::string_view bytes) noexcept
      : m_byte_buf(bytes)
       {
       }

    [[nodiscard]] constexpr bool has_bytes() const noexcept
       {
        return m_byte_pos<m_byte_buf.size();
       }

    [[nodiscard]] constexpr bool has_codepoint() const noexcept
       {
        if constexpr(ENC==Enc::UTF16LE || ENC==Enc::UTF16BE)
           {
            return (m_byte_pos+1)<m_byte_buf.size();
           }
        else if constexpr(ENC==Enc::UTF32LE || ENC==Enc::UTF32BE)
           {
            return (m_byte_pos+3)<m_byte_buf.size();
           }
        else
           {
            return m_byte_pos<m_byte_buf.size();
           }
       }

    [[nodiscard]] constexpr char32_t extract_codepoint() noexcept
       {
        assert( has_codepoint() );
        return text::extract_codepoint<ENC>(m_byte_buf, m_byte_pos);
       }

    [[nodiscard]] constexpr std::string_view byte_buf() const noexcept { return m_byte_buf; }
    [[nodiscard]] constexpr std::size_t byte_pos() const noexcept { return m_byte_pos; }
};



//---------------------------------------------------------------------------
// Re-encode a buffer encoded as in_enc to out_enc
template<text::Enc in_enc,text::Enc out_enc> constexpr std::string re_encode(const std::string_view in_bytes)
{
    std::string out_bytes;
    out_bytes.reserve( in_bytes.size() );

    text::buffer_t<in_enc> text_buf(in_bytes);
    while( text_buf.has_codepoint() )
       {
        const char32_t codepoint = text_buf.extract_codepoint();
        append_codepoint<out_enc>(codepoint, out_bytes);
       }

    // Detect truncated
    if( text_buf.has_bytes() )
       {// Truncated codepoint!
        append_codepoint<out_enc>(err_codepoint, out_bytes);
       }

    return out_bytes;
}


//---------------------------------------------------------------------------
// const std::string out_bytes = text::encode_as<text::Enc::UTF8>(in_bytes);
template<text::Enc out_enc> constexpr std::string encode_as(const std::string_view in_bytes)
{
    const auto [in_enc, bom_size] = detect_encoding_of(in_bytes);
    if( in_enc==out_enc )
       {
        return std::string(in_bytes);
       }

    switch( in_enc )
       {using enum text::Enc;

        case UTF8:
            return re_encode<UTF8,out_enc>(in_bytes);

        case UTF16LE:
            return re_encode<UTF16LE,out_enc>(in_bytes);

        case UTF16BE:
            return re_encode<UTF16BE,out_enc>(in_bytes);

        case UTF32LE:
            return re_encode<UTF32LE,out_enc>(in_bytes);

        case UTF32BE:
            return re_encode<UTF32BE,out_enc>(in_bytes);
       }
    std::unreachable();
}



    //-----------------------------------------------------------------------
    //[[nodiscard]] bool is_identifier(const char32_t cp) noexcept
    //   {
    //    return std::isalnum(cp) || cp==U'_';
    //   }

    //-----------------------------------------------------------------------
   //[[nodiscard]] bool is_numeric_literal(const char32_t cp) noexcept
   //   {
   //    return std::isdigit(cp) || cp==U'+' || cp==U'-' || cp==U'.' || cp==U'E';
   //   }

    //-----------------------------------------------------------------------
    [[nodiscard]] bool is_space(const char32_t cp) noexcept
       {
        return std::isspace(cp);
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] bool is_endline(const char32_t cp) noexcept
       {
        return cp==U'\n';
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] bool is_blank(const char32_t cp) noexcept
       {
        return std::isspace(cp) && cp!=U'\n';
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] bool is_digit(const char32_t cp) noexcept
       {
        return std::isdigit(cp);
       }


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



/////////////////////////////////////////////////////////////////////////////
#ifdef TEST_UNITS ///////////////////////////////////////////////////////////
static ut::suite<"text:: "> text_tests = []
{ ///////////////////////////////////////////////////////////////////////////
    using ut::expect;
    using namespace std::literals; // "..."sv
    using enum text::Enc;

    ut::test("detect_encoding_of") = []
       {
        auto test_enc_of = [](const std::string_view bytes, const text::bom_ret_t expected, const char* const msg) noexcept -> void
           {
            const text::bom_ret_t retrieved = text::detect_encoding_of(bytes);
            ut::expect( retrieved.enc==expected.enc and retrieved.bom_size==expected.bom_size ) << msg;
           };

        test_enc_of("\xEF\xBB\xBF blah"sv, {UTF8,3}, "Full utf-8 BOM should be detected\n");
        test_enc_of("blah blah"sv, {UTF8,0}, "No BOM found should imply utf-8\n");
        test_enc_of("\xEF\xBB"sv, {UTF8,0}, "Incomplete utf-8 BOM should fall back to utf-8\n");
        test_enc_of(""sv, {UTF8,0}, "Empty buffer should fall back to utf-8\n");

        test_enc_of("\xFF\xFE blah"sv, {UTF16LE,2}, "Full utf-16-le BOM should be detected\n");
        test_enc_of("\xFF blah"sv, {UTF8,0}, "Incomplete utf-16-le BOM should fall back to utf-8\n");

        test_enc_of("\xFE\xFF blah"sv, {UTF16BE,2}, "Full utf-16-be BOM should be detected\n");
        test_enc_of("\xFE blah"sv, {UTF8,0}, "Incomplete utf-16-be BOM should fall back to utf-8\n");

        test_enc_of("\xFF\xFE\0\0 blah"sv, {UTF32LE,4}, "Full utf-32-le BOM should be detected\n");
        test_enc_of("\xFF\xFE\0 blah"sv, {UTF16LE,2}, "Incomplete utf-32-le BOM should be interpreted as utf-16-le\n");

        test_enc_of("\0\0\xFE\xFF blah"sv, {UTF32BE,4}, "Full utf-32-be BOM should be detected\n");
        test_enc_of("\0\0\xFE blah"sv, {UTF8,0}, "Incomplete utf-32-be BOM should fall back to utf-8\n");
        test_enc_of("\0\xFE\xFF blah"sv, {UTF8,0}, "Invalid utf-32-be BOM should fall back to utf-8\n");
       };

    ut::test("codepoints decode and encode") = []
       {
        static_assert( U'🍌'==0x1F34C );

        struct test_case_t final
           {
            const char32_t code_point;
            const std::string_view UTF8_encoded;
            const std::string_view UTF16LE_encoded;
            const std::string_view UTF16BE_encoded;
            const std::string_view UTF32LE_encoded;
            const std::string_view UTF32BE_encoded;

            constexpr std::string_view encoded_as(const text::Enc enc) const noexcept
               {
                switch( enc )
                   {
                    case UTF8: return UTF8_encoded;
                    case UTF16LE: return UTF16LE_encoded;
                    case UTF16BE: return UTF16BE_encoded;
                    case UTF32LE: return UTF32LE_encoded;
                    case UTF32BE: return UTF32BE_encoded;
                   }
                std::unreachable();
               }
           };
        constexpr std::array<test_case_t,4> test_cases =
           {{
             { U'a', "\x61"sv, "\x61\0"sv, "\0\x61"sv, "\x61\0\0\0"sv, "\0\0\0\x61"sv }
            ,{ U'à', "\xC3\xA0"sv, "\xE0\0"sv, "\0\xE0"sv, "\xE0\0\0\0"sv, "\0\0\0\xE0"sv }
            ,{ U'⟶', "\xE2\x9F\xB6"sv, "\xF6\x27"sv, "\x27\xF6"sv, "\xF6\x27\0\0"sv, "\0\0\x27\xF6"sv }
            ,{ U'🍌', "\xF0\x9F\x8D\x8C"sv, "\x3C\xD8\x4C\xDF"sv, "\xD8\x3C\xDF\x4C"sv, "\x4C\xF3\x01\0"sv, "\0\x01\xF3\x4C"sv }
           }};

        auto test_codepoint = []<text::Enc ENC>(const test_case_t& test_case) constexpr -> void
           {
                //ut::log << "Testing " << test_case.UTF8_encoded << " with " << static_cast<int>(std::to_underlying(ENC));
                std::size_t pos{};
                ut::expect( text::extract_codepoint<ENC>(test_case.encoded_as(ENC), pos) == test_case.code_point );
                std::string bytes;
                text::append_codepoint<ENC>(test_case.code_point, bytes);
                ut::expect( test_case.encoded_as(ENC) == bytes );
                if( test_case.encoded_as(ENC) != bytes )
                   {
                    ut::log << "original: ";
                    for( const char ch : test_case.encoded_as(ENC) )
                       {
                        ut::log << "0x" << std::hex << (static_cast<unsigned short>(ch) & 0xFF) << ' ';
                       }

                    ut::log << "encoded: ";
                    for( const char ch : bytes )
                       {
                        ut::log << "0x" << std::hex << (static_cast<unsigned short>(ch) & 0xFF) << ' ';
                       }
                   }
           };

        for(const auto& test_case : test_cases)
           {
            ut::test(test_case.UTF8_encoded) = [&test_case, &test_codepoint]
               {
                test_codepoint.template operator()<UTF8>(test_case);
                test_codepoint.template operator()<UTF16LE>(test_case);
                test_codepoint.template operator()<UTF16BE>(test_case);
                test_codepoint.template operator()<UTF32LE>(test_case);
                test_codepoint.template operator()<UTF32BE>(test_case);
               };
           }
       };

    //ut::test("text::buffer_t") = []
    //   {
    //    //
    //   };

    ut::test("text::encode_as") = []
       {
        ut::expect( text::encode_as<UTF8>(""sv) == ""sv) << "Implicit utf-8 empty string should be empty\n";

        struct test_case_t final
           {
            const std::string_view str;
            const std::string_view UTF8_encoded;
            const std::string_view UTF16LE_encoded;
            const std::string_view UTF16BE_encoded;
            const std::string_view UTF32LE_encoded;
            const std::string_view UTF32BE_encoded;

            constexpr std::string_view encoded_as(const text::Enc enc) const noexcept
               {
                switch( enc )
                   {
                    case UTF8: return UTF8_encoded;
                    case UTF16LE: return UTF16LE_encoded;
                    case UTF16BE: return UTF16BE_encoded;
                    case UTF32LE: return UTF32LE_encoded;
                    case UTF32BE: return UTF32BE_encoded;
                   }
                std::unreachable();
               }
           };
        constexpr std::array<test_case_t,2> test_cases =
           {{
             { " ab"sv,
               "\xEF\xBB\xBF ab"sv,
               "\xFF\xFE\x20\x00\x61\x00\x62\x00"sv,
               "\xFE\xFF\x00\x20\x00\x61\x00\x62"sv,
               "\xFF\xFE\x00\x00\x20\x00\x00\x00\x61\x00\x00\x00\x62\x00\x00\x00"sv,
               "\x00\x00\xFE\xFF\x00\x00\x00\x20\x00\x00\x00\x61\x00\x00\x00\x62"sv }
            ,{ "è una ⛵ ┌─┐"sv,
               "\xEF\xBB\xBF\xC3\xA8\x20\x75\x6E\x61\x20\xE2\x9B\xB5\x20\xE2\x94\x8C\xE2\x94\x80\xE2\x94\x90"sv,
               "\xFF\xFE\xE8\x00\x20\x00\x75\x00\x6E\x00\x61\x00\x20\x00\xF5\x26\x20\x00\x0C\x25\x00\x25\x10\x25"sv,
               "\xFE\xFF\x00\xE8\x00\x20\x00\x75\x00\x6E\x00\x61\x00\x20\x26\xF5\x00\x20\x25\x0C\x25\x00\x25\x10"sv,
               "\xFF\xFE\x00\x00\xE8\x00\x00\x00\x20\x00\x00\x00\x75\x00\x00\x00\x6E\x00\x00\x00\x61\x00\x00\x00\x20\x00\x00\x00\xF5\x26\x00\x00\x20\x00\x00\x00\x0C\x25\x00\x00\x00\x25\x00\x00\x10\x25\x00\x00"sv, "\x00\x00\xFE\xFF\x00\x00\x00\xE8\x00\x00\x00\x20\x00\x00\x00\x75\x00\x00\x00\x6E\x00\x00\x00\x61\x00\x00\x00\x20\x00\x00\x26\xF5\x00\x00\x00\x20\x00\x00\x25\x0C\x00\x00\x25\x00\x00\x00\x25\x10"sv }
           }};

        for(const auto& ex : test_cases)
           {
            ut::test(ex.UTF8_encoded) = [&ex]
               {
                ut::expect( text::encode_as<UTF8>(ex.encoded_as(UTF8))==ex.encoded_as(UTF8)) << "utf-8 to utf-8\n";
                ut::expect( text::encode_as<UTF16LE>(ex.encoded_as(UTF8))==ex.encoded_as(UTF16LE)) << "utf-8 to utf-16le\n";
                ut::expect( text::encode_as<UTF16BE>(ex.encoded_as(UTF8))==ex.encoded_as(UTF16BE)) << "utf-8 to utf-16be\n";
                ut::expect( text::encode_as<UTF32LE>(ex.encoded_as(UTF8))==ex.encoded_as(UTF32LE)) << "utf-8 to utf-32le\n";
                ut::expect( text::encode_as<UTF32BE>(ex.encoded_as(UTF8))==ex.encoded_as(UTF32BE)) << "utf-8 to utf-32be\n";
                ut::expect( text::encode_as<UTF8>(ex.encoded_as(UTF16LE))==ex.encoded_as(UTF8)) << "utf-16le to utf-8\n";
                ut::expect( text::encode_as<UTF16LE>(ex.encoded_as(UTF16LE))==ex.encoded_as(UTF16LE)) << "utf-16le to utf-16le\n";
                ut::expect( text::encode_as<UTF16BE>(ex.encoded_as(UTF16LE))==ex.encoded_as(UTF16BE)) << "utf-16le to utf-16be\n";
                ut::expect( text::encode_as<UTF32LE>(ex.encoded_as(UTF16LE))==ex.encoded_as(UTF32LE)) << "utf-16le to utf-32le\n";
                ut::expect( text::encode_as<UTF32BE>(ex.encoded_as(UTF16LE))==ex.encoded_as(UTF32BE)) << "utf-16le to utf-32be\n";
                ut::expect( text::encode_as<UTF8>(ex.encoded_as(UTF16BE))==ex.encoded_as(UTF8)) << "utf-16be to utf-8\n";
                ut::expect( text::encode_as<UTF16LE>(ex.encoded_as(UTF16BE))==ex.encoded_as(UTF16LE)) << "utf-16be to utf-16le\n";
                ut::expect( text::encode_as<UTF16BE>(ex.encoded_as(UTF16BE))==ex.encoded_as(UTF16BE)) << "utf-16be to utf-16be\n";
                ut::expect( text::encode_as<UTF32LE>(ex.encoded_as(UTF16BE))==ex.encoded_as(UTF32LE)) << "utf-16be to utf-32le\n";
                ut::expect( text::encode_as<UTF32BE>(ex.encoded_as(UTF16BE))==ex.encoded_as(UTF32BE)) << "utf-16be to utf-32be\n";
                ut::expect( text::encode_as<UTF8>(ex.encoded_as(UTF32LE))==ex.encoded_as(UTF8)) << "utf-32le to utf-8\n";
                ut::expect( text::encode_as<UTF16LE>(ex.encoded_as(UTF32LE))==ex.encoded_as(UTF16LE)) << "utf-32le to utf-16le\n";
                ut::expect( text::encode_as<UTF16BE>(ex.encoded_as(UTF32LE))==ex.encoded_as(UTF16BE)) << "utf-32le to utf-16be\n";
                ut::expect( text::encode_as<UTF32LE>(ex.encoded_as(UTF32LE))==ex.encoded_as(UTF32LE)) << "utf-32le to utf-32le\n";
                ut::expect( text::encode_as<UTF32BE>(ex.encoded_as(UTF32LE))==ex.encoded_as(UTF32BE)) << "utf-32le to utf-32be\n";
                ut::expect( text::encode_as<UTF8>(ex.encoded_as(UTF32BE))==ex.encoded_as(UTF8)) << "utf-32be to utf-8\n";
                ut::expect( text::encode_as<UTF16LE>(ex.encoded_as(UTF32BE))==ex.encoded_as(UTF16LE)) << "utf-32be to utf-16le\n";
                ut::expect( text::encode_as<UTF16BE>(ex.encoded_as(UTF32BE))==ex.encoded_as(UTF16BE)) << "utf-32be to utf-16be\n";
                ut::expect( text::encode_as<UTF32LE>(ex.encoded_as(UTF32BE))==ex.encoded_as(UTF32LE)) << "utf-32be to utf-32le\n";
                ut::expect( text::encode_as<UTF32BE>(ex.encoded_as(UTF32BE))==ex.encoded_as(UTF32BE)) << "utf-32be to utf-32be\n";
               };
           }
       };

};///////////////////////////////////////////////////////////////////////////
#endif // TESTING ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
