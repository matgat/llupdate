#ifndef GUARD_text_hpp
#define GUARD_text_hpp
//  ---------------------------------------------
//  Encoding aware text utilities
//  ---------------------------------------------
#include <cstdint> // std::uint8_t, std::uint16_t, ...
#include <cassert>
#include <string>
#include <string_view>
//#include <cuchar> // mbrtoc32, ...


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace text
{

    namespace details
       {
        //-------------------------------------------------------------------
        [[nodiscard]] std::uint16_t combine_chars(const char h, const char l) noexcept
           {
            return (static_cast<unsigned char>(h) << 8) |
                    static_cast<unsigned char>(l);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] std::uint32_t combine_chars(const char hh, const char hl, const char lh, const char ll) noexcept
           {
            return (static_cast<unsigned char>(hh) << 24) |
                   (static_cast<unsigned char>(hl) << 16) |
                   (static_cast<unsigned char>(lh) << 8) |
                    static_cast<unsigned char>(ll);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char high_byte_of(const std::uint16_t word) noexcept
           {
            return static_cast<char>((word >> 8) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char low_byte_of(const std::uint16_t word) noexcept
           {
            return static_cast<char>(word & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char hh_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 24) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char hl_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 16) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char lh_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>((dword >> 8) & 0xFF);
           }

        //-------------------------------------------------------------------
        [[nodiscard]] char ll_byte_of(const std::uint32_t dword) noexcept
           {
            return static_cast<char>(dword & 0xFF);
           }
       }

enum class enc_t : std::uint8_t
   {
    //ANSI,
    UTF8,
    UTF16LE,
    UTF16BE,
    UTF32LE,
    UTF32BE
   };

const char32_t err_char = U'�'; // replacement character '\u{FFFD}'
const char32_t null_char = 0;


//---------------------------------------------------------------------------
// auto [enc, bom_size] = text::detect_encoding_of(bytes);
struct bom_ret_t final { enc_t enc; std::uint8_t bom_size; };
bom_ret_t detect_encoding_of(const std::string_view bytes)
   {//      +--------------+-------------+-------+
    //      |  Encoding    |   Bytes     | Chars |
    //      |--------------|-------------|-------|
    //      | UTF-8        | EF BB BF    | ï»¿   |
    //      | UTF-16 (LE)  | FF FE       | ÿþ    |
    //      | UTF-16 (BE)  | FE FF       | þÿ    |
    //      | UTF-32 (LE)  | FF FE 00 00 | ÿþ..  |
    //      | UTF-32 (BE)  | 00 00 FE FF | ..þÿ  |
    //      +--------------+-------------+-------+
    using enum enc_t;
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
template<enc_t enc> char32_t extract_codepoint(const std::string_view bytes, std::size_t& pos) noexcept;

//---------------------------------------------------------------------------
//template<> char32_t extract_codepoint<enc_t::ANSI>(const std::string_view bytes, std::size_t& pos) noexcept
//{
//    assert( pos<bytes.size() );
//
//    return bytes[pos++];
//}

//---------------------------------------------------------------------------
template<> char32_t extract_codepoint<enc_t::UTF8>(const std::string_view bytes, std::size_t& pos) noexcept
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
    return err_char;
}

//---------------------------------------------------------------------------
template<bool LE> char32_t extract_next_codepoint_from_utf16(const std::string_view bytes, std::size_t& pos) noexcept
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
        return err_char;
       }

    // Here expecting the second codeunit
    const std::uint16_t codeunit2 = get_code_unit(bytes, pos);
    if( codeunit2<0xDC00 || codeunit2>=0xE000 ) [[unlikely]]
       {// Not a second surrogate!
        return err_char;
       }

    // Ok, I have the two valid codeunits
    pos += 2;
    return 0x10000 + ((codeunit1 - 0xD800) << 10) + (codeunit2 - 0xDC00);
}
template<> char32_t extract_codepoint<enc_t::UTF16LE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<true>(bytes, pos);
}
template<> char32_t extract_codepoint<enc_t::UTF16BE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<false>(bytes, pos);
}

//---------------------------------------------------------------------------
template<> char32_t extract_codepoint<enc_t::UTF32LE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( (pos+3)<bytes.size() );

    const char32_t codepoint = details::combine_chars(bytes[pos+3], bytes[pos+2], bytes[pos+1], bytes[pos]); // Little endian
    pos += 4;
    return codepoint;
}

//---------------------------------------------------------------------------
template<> char32_t extract_codepoint<enc_t::UTF32BE>(const std::string_view bytes, std::size_t& pos) noexcept
{
    assert( (pos+3)<bytes.size() );

    const char32_t codepoint = details::combine_chars(bytes[pos], bytes[pos+1], bytes[pos+2], bytes[pos+3]); // Big endian
    pos += 4;
    return codepoint;
}


//---------------------------------------------------------------------------
// Encode: Write a codepoint according to encoding and endianness
template<enc_t enc> void append_codepoint(const char32_t code_point, std::string& bytes) noexcept;

//---------------------------------------------------------------------------
//template<> void append_codepoint<enc_t::ANSI>(const char32_t code_point, std::string& bytes) noexcept
//{
//    bytes += static_cast<char>(code_point); // Narrowing!
//}

//---------------------------------------------------------------------------
template<> void append_codepoint<enc_t::UTF8>(const char32_t code_point, std::string& bytes) noexcept
{
    if( code_point<0x80 ) [[likely]]
       {
        bytes.push_back( static_cast<char>(code_point) );
       }
    else if( code_point<0x800 )
       {
        bytes.push_back( static_cast<char>(0xC0 | (code_point >> 6) ));
        bytes.push_back( static_cast<char>(0x80 | (code_point & 0x3F)) );
       }
    else if( code_point<0x10000 )
       {
        bytes.push_back( static_cast<char>(0xE0 | (code_point >> 12)) );
        bytes.push_back( static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | (code_point & 0x3F)) );
       }
    else
       {
        bytes.push_back( static_cast<char>(0xF0 | (code_point >> 18)) );
        bytes.push_back( static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)) );
        bytes.push_back( static_cast<char>(0x80 | (code_point & 0x3F)) );
       }
}

//---------------------------------------------------------------------------
// Encode a codepoint outside Basic Multilingual Plane
std::pair<std::uint16_t,std::uint16_t> encode_as_utf16(uint32_t code_point) noexcept
{
    assert( code_point>=0x10000 );
    code_point -= 0x10000;
    return { static_cast<std::uint16_t>((code_point >> 10) + 0xD800),
             static_cast<std::uint16_t>((code_point & 0x3FF) + 0xDC00) };
}

//---------------------------------------------------------------------------
template<> void append_codepoint<enc_t::UTF16LE>(const char32_t code_point, std::string& bytes) noexcept
{
    if( code_point<0x10000 ) [[likely]]
       {
        const std::uint16_t codeunit = static_cast<std::uint16_t>(code_point);
        bytes.push_back( details::low_byte_of( codeunit) );
        bytes.push_back( details::high_byte_of( codeunit ) );
        return;
       }

    const auto codeunits = encode_as_utf16(code_point);
    bytes.push_back( details::low_byte_of( codeunits.first ) );
    bytes.push_back( details::high_byte_of( codeunits.first ) );
    bytes.push_back( details::low_byte_of( codeunits.second ) );
    bytes.push_back( details::high_byte_of( codeunits.second ) );
}

//---------------------------------------------------------------------------
template<> void append_codepoint<enc_t::UTF16BE>(const char32_t code_point, std::string& bytes) noexcept
{
    if( code_point<0x10000 ) [[likely]]
       {
        const std::uint16_t codeunit = static_cast<std::uint16_t>(code_point);
        bytes.push_back( details::high_byte_of( codeunit ) );
        bytes.push_back( details::low_byte_of( codeunit) );
        return;
       }

    const auto codeunits = encode_as_utf16(code_point);
    bytes.push_back( details::high_byte_of( codeunits.first ) );
    bytes.push_back( details::low_byte_of( codeunits.first ) );
    bytes.push_back( details::high_byte_of( codeunits.second ) );
    bytes.push_back( details::low_byte_of( codeunits.second ) );
}

//---------------------------------------------------------------------------
template<> void append_codepoint<enc_t::UTF32LE>(const char32_t code_point, std::string& bytes) noexcept
{
    bytes.push_back( details::ll_byte_of( code_point ) );
    bytes.push_back( details::lh_byte_of( code_point ) );
    bytes.push_back( details::hl_byte_of( code_point ) );
    bytes.push_back( details::hh_byte_of( code_point ) );
}

//---------------------------------------------------------------------------
template<> void append_codepoint<enc_t::UTF32BE>(const char32_t code_point, std::string& bytes) noexcept
{
    bytes.push_back( details::hh_byte_of( code_point ) );
    bytes.push_back( details::hl_byte_of( code_point ) );
    bytes.push_back( details::lh_byte_of( code_point ) );
    bytes.push_back( details::ll_byte_of( code_point ) );
}




/////////////////////////////////////////////////////////////////////////////
template<enc_t ENC> class buffer_t final
{
 private:
    std::string_view m_byte_buf;
    std::size_t m_byte_pos = 0; // Index of currently pointed byte

 public:
    explicit buffer_t(const std::string_view bytes) noexcept
      : m_byte_buf(bytes)
       {
       }

    [[nodiscard]] bool has_bytes() const noexcept
       {
        return m_byte_pos<m_byte_buf.size();
       }

    [[nodiscard]] bool has_codepoint() const noexcept
       {
        if constexpr(ENC==enc_t::UTF16LE || ENC==enc_t::UTF16BE)
           {
            return (m_byte_pos+1)<m_byte_buf.size();
           }
        else if constexpr(ENC==enc_t::UTF32LE || ENC==enc_t::UTF32BE)
           {
            return (m_byte_pos+3)<m_byte_buf.size();
           }
        else
           {
            return m_byte_pos<m_byte_buf.size();
           }
       }

    [[nodiscard]] char32_t extract_codepoint() noexcept
       {
        assert( has_codepoint() );
        return text::extract_codepoint<ENC>(m_byte_buf, m_byte_pos);
       }

    [[nodiscard]] std::string_view byte_buf() const noexcept { return m_byte_buf; }
    [[nodiscard]] std::size_t byte_pos() const noexcept { return m_byte_pos; }
};


//---------------------------------------------------------------------------
//template<text::enc_t enc> std::string convert(const std::string_view bytes)
//{
//    text::buffer_t<enc> text_buf(bytes);
//    while( text_buf.has_codepoint() )
//       {
//        const char32_t codepoint = text_buf.extract_codepoint();
//        // ...
//       }
//    // Detect truncated
//    if( text_buf.has_bytes() )
//       {
//        // Truncated codepoint!
//       }
//}


//---------------------------------------------------------------------------
//[[nodiscard]] constexpr std::string iso_latin1_to_utf8(const std::string_view ansi)
//{
//    std::string utf8;
//    utf8.reserve( (3 * ansi.size()) / 2 );
//    for( std::size_t i=0; i<ansi.size(); ++i )
//       {
//        if( !(ansi[i] & '\x80') ) // ansi[i] < 128
//           {
//            utf8 += ansi[i];
//           }
//        else
//           {
//            utf8 += '\xC0' | (ansi[i] >> 6);
//            utf8 += '\x80' | (ansi[i] & '\x3f');
//           }
//       }
//    return utf8;
//}


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



// TEST



//---- end unit -------------------------------------------------------------
#endif
