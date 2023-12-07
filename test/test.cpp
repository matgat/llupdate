#include <gtest/gtest.h>

#include "../source/text.hpp" // text::buffer_t


/////////////////////////////////////////////////////////////////////////////
#ifdef TESTING //////////////////////////////////////////////////////////////
namespace text_test /////////////////////////////////////////////////////////
{ ///////////////////////////////////////////////////////////////////////////

    using namespace std::literals; // "..."sv



    /////////////////////////////////////////////////////////////////////////
    class DetectEncodingTest : public testing::Test
    {
     public:
        static void enc_of_is(const std::string_view bytes, const text::bom_ret_t expected)
           {
            const text::bom_ret_t retrieved = text::detect_encoding_of(bytes);
            EXPECT_EQ(retrieved.enc, expected.enc);
            EXPECT_EQ(retrieved.bom_size, expected.bom_size);
           }
    };
    TEST_F(DetectEncodingTest, DetectEncoding)
    {
        using enum text::enc_t;

        enc_of_is("\xEF\xBB\xBF blah"sv, {UTF8,3}); // << "Detect full utf-8 BOM";
        enc_of_is("blah blah"sv, {UTF8,0}); // << "No BOM found should imply utf-8";
        enc_of_is("\xEF\xBB"sv, {UTF8,0}); // << "Invalid/incomplete utf-8 BOM";
        enc_of_is(""sv, {UTF8,0}); // << "Empty buffer falls back to utf-8";

        enc_of_is("\xFF\xFE blah"sv, {UTF16LE,2}); // << "Detect full utf-16-le BOM";
        enc_of_is("\xFF blah"sv, {UTF8,0}); // << "Invalid/incomplete utf-16-le BOM";

        enc_of_is("\xFE\xFF blah"sv, {UTF16BE,2}); // << "Detect full utf-16-be BOM";
        enc_of_is("\xFE blah"sv, {UTF8,0}); // << "Invalid/incomplete utf-16-be BOM";

        enc_of_is("\xFF\xFE\0\0 blah"sv, {UTF32LE,4}); // << "Detect full utf-32-le BOM";
        enc_of_is("\xFF\xFE\0 blah"sv, {UTF16LE,2}); // << "Invalid/incomplete utf-32-le BOM";

        enc_of_is("\0\0\xFE\xFF blah"sv, {UTF32BE,4}); // << "Detect full utf-32-be BOM";
        enc_of_is("\0\0\xFE blah"sv, {UTF8,0}); // << "Invalid/incomplete utf-32-be BOM";
        enc_of_is("\0\xFE\xFF blah"sv, {UTF8,0}); // << "Invalid/incomplete utf-32-be BOM";
    }



    /////////////////////////////////////////////////////////////////////////
    class ExtractCodePointTest : public testing::Test
    {
     public:
        //template<text::enc_t enc> static char32_t codepoint_of(const std::u8string_view bytes)
        //   {
        //    std::size_t pos = 0;
        //    return text::extract_next_codepoint<enc>(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()), pos);
        //   }
        template<text::enc_t enc> static char32_t codepoint_of(const std::string_view bytes)
           {
            std::size_t pos = 0;
            return text::extract_next_codepoint<enc>(bytes,pos);
           }
    };
    TEST_F(ExtractCodePointTest, ExtractCodePoint)
    {
        using enum text::enc_t;

        EXPECT_TRUE( codepoint_of<UTF8>("\x61"sv)==U'a' );
        EXPECT_TRUE( codepoint_of<UTF8>("\xC3\xA0"sv)==U'à' );
        EXPECT_TRUE( codepoint_of<UTF8>("\xE2\x9F\xB6"sv)==U'⟶' );
        EXPECT_TRUE( codepoint_of<UTF8>("\xF0\x9F\x8D\x8C"sv)==U'🍌' );

        EXPECT_TRUE( codepoint_of<UTF16LE>("\x61\0"sv)==U'a' );
        EXPECT_TRUE( codepoint_of<UTF16LE>("\xE0\0"sv)==U'à' );
        EXPECT_TRUE( codepoint_of<UTF16LE>("\xF6\x27"sv)==U'⟶' );
        EXPECT_TRUE( codepoint_of<UTF16LE>("\x3C\xD8\x4C\xDF"sv)==U'🍌' );
        //EXPECT_TRUE( codepoint_of<UTF16LE>("\x7E\x23"sv)==U'⍾' );
        //EXPECT_TRUE( codepoint_of<UTF16LE>("\x34\xD8\x1E\xDD"sv)==U'𝄞' );

        EXPECT_TRUE( codepoint_of<UTF16BE>("\0\x61"sv)==U'a' );
        EXPECT_TRUE( codepoint_of<UTF16BE>("\0\xE0"sv)==U'à' );
        EXPECT_TRUE( codepoint_of<UTF16BE>("\x27\xF6"sv)==U'⟶' );
        EXPECT_TRUE( codepoint_of<UTF16BE>("\xD8\x3C\xDF\x4C"sv)==U'🍌' );

        EXPECT_TRUE( codepoint_of<UTF32LE>("\x61\0\0\0"sv)==U'a' );
        EXPECT_TRUE( codepoint_of<UTF32LE>("\xE0\0\0\0"sv)==U'à' );
        EXPECT_TRUE( codepoint_of<UTF32LE>("\xF6\x27\0\0"sv)==U'⟶' );
        EXPECT_TRUE( codepoint_of<UTF32LE>("\x4C\xF3\x01\0"sv)==U'🍌' );

        EXPECT_TRUE( codepoint_of<UTF32BE>("\0\0\0\x61"sv)==U'a' );
        EXPECT_TRUE( codepoint_of<UTF32BE>("\0\0\0\xE0"sv)==U'à' );
        EXPECT_TRUE( codepoint_of<UTF32BE>("\0\0\x27\xF6"sv)==U'⟶' );
        EXPECT_TRUE( codepoint_of<UTF32BE>("\0\x01\xF3\x4C"sv)==U'🍌' );
    }


} ///////////////////////////////////////////////////////////////////////////
#endif // TESTING ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
