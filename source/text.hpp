#ifndef GUARD_text_hpp
#define GUARD_text_hpp
//  ---------------------------------------------
//  Encoding aware text utilities
//  ---------------------------------------------
#include <cstdint> // std::uint8_t
#include <cassert>
#include <string_view>
//#include <cuchar> // mbrtoc32, ...


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace text
{

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
// auto [enc, bom_size] = text::detect_encoding_of(buf);
struct bom_ret_t final { enc_t enc; std::uint8_t bom_size; };
bom_ret_t detect_encoding_of(const std::string_view buf)
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
    if( buf.size()>2 ) [[likely]]
       {
        if( buf[0]=='\xFF' && buf[1]=='\xFE' )
           {
            if( buf.size()>=4 && buf[2]=='\x00' && buf[3]=='\x00')
               {
                return {UTF32LE, 4};
               }
            else
               {
                return {UTF16LE, 2};
               }
           }
        else if( buf[0]=='\xFE' && buf[1]=='\xFF' )
           {
            return {UTF16BE, 2};
           }
        else if( buf.size()>=4 && buf[0]=='\x00' && buf[1]=='\x00' && buf[2]=='\xFE' && buf[3]=='\xFF' )
           {
            return {UTF32BE, 4};
           }
        else if( buf[0]=='\xEF' && buf[1]=='\xBB' && buf[2]=='\xBF' )
           {
            return {UTF8, 3};
           }
       }
    // Fallback
    return {UTF8, 0};
   }


//---------------------------------------------------------------------------
// Extract a codepoint according to encoding and endianness
template<enc_t enc> char32_t extract_next_codepoint(const std::string_view buf, std::size_t& pos) noexcept;

//---------------------------------------------------------------------------
//template<> char32_t extract_next_codepoint<enc_t::ANSI>(const std::string_view buf, std::size_t& pos) noexcept
//{
//    assert(pos<buf.size());
//
//    return buf[pos++];
//}

//---------------------------------------------------------------------------
template<> char32_t extract_next_codepoint<enc_t::UTF8>(const std::string_view buf, std::size_t& pos) noexcept
{
    assert(pos<buf.size());

    if( (buf[pos] & 0x80)==0 ) [[likely]]
       {
        const char32_t codepoint = buf[pos];
        ++pos;
        return codepoint;
       }

    else if( (pos+1)<buf.size() && (buf[pos] & 0xE0)==0xC0 && (buf[pos+1] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((buf[pos] & 0x1F) << 6) | (buf[pos+1] & 0x3F);
        pos += 2;
        return codepoint;
       }

    else if( (pos+2)<buf.size() && (buf[pos] & 0xF0)==0xE0 && (buf[pos+1] & 0xC0)==0x80 && (buf[pos+2] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((buf[pos] & 0x0F) << 12) | ((buf[pos+1] & 0x3F) << 6) | (buf[pos+2] & 0x3F);
        pos += 3;
        return codepoint;
       }

    else if( (pos+3)<buf.size() && (buf[pos] & 0xF8)==0xF0 && (buf[pos+1] & 0xC0)==0x80 && (buf[pos+2] & 0xC0)==0x80 && (buf[pos+3] & 0xC0)==0x80 )
       {
        const char32_t codepoint = ((buf[pos] & 0x07) << 18) | ((buf[pos+1] & 0x3F) << 12) | ((buf[pos+2] & 0x3F) << 6) | (buf[pos+3] & 0x3F);
        pos += 4;
        return codepoint;
       }

    // Invalid utf-8 character
    ++pos;
    return err_char;
}

//---------------------------------------------------------------------------
template<bool LE> char32_t extract_next_codepoint_from_utf16(const std::string_view buf, std::size_t& pos) noexcept
{
    assert((pos+1)<buf.size());
    aaa
    int a1 = buf[pos];
    int a2 = buf[pos+1];
    auto a3 = a2 << 8;
    auto a4 = a1 | a3;
    auto a5 = a1 | (a2 << 8); 
    auto a6 = buf[pos] | (buf[pos+1] << 8);

    auto get_code_unit = [buf,pos]() -> std::uint16_t
                               {
                                if constexpr(LE) return buf[pos] | (buf[pos+1] << 8); // Little endian
                                else             return (buf[pos] << 8) | buf[pos+1]; // Big endian
                               };

    // If the first codeunit is in the intervals [0x0000–0xD800), [0xE000–0xFFFF]
    // then coincides with the codepoint itself (Basic Multilingual Plane)
    // Otherwise the whole codepoint is composed by two codeunits:
    // the first in the interval [0xD800-0xDC00), and the second in [0xDC00–0xE000)
    // codeunit1 = 0b110110yyyyyyyyyy // 0xD800 + yyyyyyyyyy [0xD800-0xDC00)
    // codeunit2 = 0b110111xxxxxxxxxx // 0xDC00 + xxxxxxxxxx [0xDC00–0xE000)
    // codepoint = 0x10000 + yyyyyyyyyyxxxxxxxxxx

    const std::uint16_t codeunit1 = get_code_unit();
    pos += 2;

    if( codeunit1<0xD800 || codeunit1>=0xE000 ) [[likely]]
       {// Basic Multilingual Plane
        return codeunit1;
       }

    if( codeunit1>=0xDC00 || (pos+1)>=buf.size() ) [[unlikely]]
       {// Not a first surrogate!
        return err_char;
       }

    // Here expecting the second codeunit
    const std::uint16_t codeunit2 = get_code_unit();
    if( codeunit2<0xDC00 || codeunit2>=0xE000 ) [[unlikely]]
       {// Not a second surrogate!
        return err_char;
       }

    // Ok, I have the two valid codeunits
    pos += 2;
    return 0x10000 + ((codeunit1 - 0xD800) << 10) + (codeunit2 - 0xDC00);
}
template<> char32_t extract_next_codepoint<enc_t::UTF16LE>(const std::string_view buf, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<true>(buf, pos);
}
template<> char32_t extract_next_codepoint<enc_t::UTF16BE>(const std::string_view buf, std::size_t& pos) noexcept
{
    return extract_next_codepoint_from_utf16<false>(buf, pos);
}

//---------------------------------------------------------------------------
template<> char32_t extract_next_codepoint<enc_t::UTF32LE>(const std::string_view buf, std::size_t& pos) noexcept
{
    assert((pos+3)<buf.size());

    const char32_t codepoint = buf[pos] | (buf[pos+1] << 8) | (buf[pos+2] << 16) | (buf[pos+3] << 24); // Little endian
    pos += 4;
    return codepoint;
}

//---------------------------------------------------------------------------
template<> char32_t extract_next_codepoint<enc_t::UTF32BE>(const std::string_view buf, std::size_t& pos) noexcept
{
    assert((pos+3)<buf.size());

    const char32_t codepoint = (buf[pos] << 24) | (buf[pos+1] << 16) | (buf[pos+2] << 8) | buf[pos+3]; // Big endian
    pos += 4;
    return codepoint;
}


    // write_as_utf16(char32_t code_point)
    //   {
    //    if( code_point<=0xFFFF )
    //       {
    //        if( u16out )  *u16out++ = static_cast<char16_t>(code_point);
    //        ++outstr_size;
    //       }
    //    else
    //       {
    //        if (u16out)
    //           {
    //            code_point -= 0x10000;
    //            *u16out++ = static_cast<char16_t>((code_point >> 10) + 0xD800);
    //            *u16out++ = static_cast<char16_t>((code_point & 0x3FF) + 0xDC00);
    //           }
    //        outstr_size += 2;
    //       }
    //   }
    // How do we encode code points in [U+10000, U+10FFFF]?
    // 1. Subtract 0x010000 from the code point; this yields a 20 bit number in [0x0, 0xFFFFF]
    // 2. Split the number in half, with the upper ten bits in one half; the lower ten bits in the second
    // 3. The upper ten bits are added to 0xD800 to form the lead surrogate
    // 4. The lower ten bits are added to 0xDC00 to form the trail surrogate

    // 0x010000 is subtracted from the code point, leaving a 20-bit number in the range 0..0x0FFFFF;
    // the top ten bits (a number in the range 0..0x03FF) are added to 0xD800 to give the first code unit or high surrogate, which will be in the range 0xD800..0xDBFF;
    // the low ten bits (also in the range 0..0x03FF) are added to 0xDC00 to give the second code unit or low surrogate, which will be in the range 0xDC00..0xDFFF.
    // In C++14 and later this could be written as:
    //
    // #include <cstdint>
    //
    // using codepoint = std::uint32_t;
    // using utf16 = std::uint16_t;
    //
    // struct surrogate {
    //     utf16 high; // Leading
    //     utf16 low;  // Trailing
    // };
    //
    // constexpr surrogate split(codepoint const in) noexcept {
    //     auto const inMinus0x10000 = (in - 0x10000);
    //     surrogate const r{
    //             static_cast<utf16>((inMinus0x10000 / 0x400) + 0xd800), // High
    //             static_cast<utf16>((inMinus0x10000 % 0x400) + 0xdc00)}; // Low
    //     return r;
    // }
    // In the reverse direction one just has to combine the last 10 bits from the high surrogate and the last 10 bits from the low surrogate, and add 0x10000:
    //
    // constexpr codepoint combine(surrogate const s) noexcept {
    //     return static_cast<codepoint>(
    //             ((s.high - 0xd800) * 0x400) + (s.low - 0xdc00) + 0x10000);
    // }



/////////////////////////////////////////////////////////////////////////////
template<enc_t ENC> class buffer_t final
{
 private:
    std::string_view m_byte_buf;
    std::size_t m_byte_pos = 0; // Index of currently pointed byte

 public:
    explicit buffer_t(const std::string_view buf) noexcept
      : m_byte_buf(buf)
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

    [[nodiscard]] char32_t extract_next_codepoint() noexcept
       {
        assert( has_codepoint() );
        return text::extract_next_codepoint<ENC>(m_byte_buf, m_byte_pos);
       }

    [[nodiscard]] std::string_view byte_buf() const noexcept { return m_byte_buf; }
    [[nodiscard]] std::size_t byte_pos() const noexcept { return m_byte_pos; }
};

//---------------------------------------------------------------------------
//template<text::enc_t enc> std::string convert(const std::string_view buf)
//{
//    text::buffer_t<enc> text_buf(buf);
//    while( text_buf.has_codepoint() )
//       {
//        const char32_t codepoint = text_buf.extract_next_codepoint();
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



//



//---- end unit -------------------------------------------------------------
#endif
