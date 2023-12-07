#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp" // import boost.ut;

#include "../source/text.hpp" // text::buffer_t


/////////////////////////////////////////////////////////////////////////////
#ifdef TESTING //////////////////////////////////////////////////////////////
boost::ut::suite text_test_suite = []
{ ///////////////////////////////////////////////////////////////////////////
    using namespace boost::ut;
    using namespace std::literals; // "..."sv

    test("detect_encoding_of") = []
       {
        auto test_enc_of = [](const std::string_view bytes, const text::bom_ret_t expected, const char* const msg) noexcept -> void
           {
            const text::bom_ret_t retrieved = text::detect_encoding_of(bytes);
            expect( retrieved.enc==expected.enc and retrieved.bom_size==expected.bom_size ) << msg;
           };

        using enum text::enc_t;

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

    test("extract_codepoint") = []
       {
        using enum text::enc_t;

        std::size_t pos{};
        pos=0; expect( text::extract_codepoint<UTF8>("\x61"sv, pos) == U'a' );
        pos=0; expect( text::extract_codepoint<UTF8>("\xC3\xA0"sv, pos) == U'à' );
        pos=0; expect( text::extract_codepoint<UTF8>("\xC3\xA0"sv, pos)==U'à' );
        pos=0; expect( text::extract_codepoint<UTF8>("\xE2\x9F\xB6"sv, pos)==U'⟶' );
        pos=0; expect( text::extract_codepoint<UTF8>("\xF0\x9F\x8D\x8C"sv, pos)==U'🍌' );

        pos=0; expect( text::extract_codepoint<UTF16LE>("\x61\0"sv, pos)==U'a' );
        pos=0; expect( text::extract_codepoint<UTF16LE>("\xE0\0"sv, pos)==U'à' );
        pos=0; expect( text::extract_codepoint<UTF16LE>("\xF6\x27"sv, pos)==U'⟶' );
        pos=0; expect( text::extract_codepoint<UTF16LE>("\x3C\xD8\x4C\xDF"sv, pos)==U'🍌' );
        //pos=0; expect( text::extract_codepoint<UTF16LE>("\x7E\x23"sv, pos)==U'⍾' );
        //pos=0; expect( text::extract_codepoint<UTF16LE>("\x34\xD8\x1E\xDD"sv, pos)==U'𝄞' );

        pos=0; expect( text::extract_codepoint<UTF16BE>("\0\x61"sv, pos)==U'a' );
        pos=0; expect( text::extract_codepoint<UTF16BE>("\0\xE0"sv, pos)==U'à' );
        pos=0; expect( text::extract_codepoint<UTF16BE>("\x27\xF6"sv, pos)==U'⟶' );
        pos=0; expect( text::extract_codepoint<UTF16BE>("\xD8\x3C\xDF\x4C"sv, pos)==U'🍌' );

        pos=0; expect( text::extract_codepoint<UTF32LE>("\x61\0\0\0"sv, pos)==U'a' );
        pos=0; expect( text::extract_codepoint<UTF32LE>("\xE0\0\0\0"sv, pos)==U'à' );
        pos=0; expect( text::extract_codepoint<UTF32LE>("\xF6\x27\0\0"sv, pos)==U'⟶' );
        pos=0; expect( text::extract_codepoint<UTF32LE>("\x4C\xF3\x01\0"sv, pos)==U'🍌' );

        pos=0; expect( text::extract_codepoint<UTF32BE>("\0\0\0\x61"sv, pos)==U'a' );
        pos=0; expect( text::extract_codepoint<UTF32BE>("\0\0\0\xE0"sv, pos)==U'à' );
        pos=0; expect( text::extract_codepoint<UTF32BE>("\0\0\x27\xF6"sv, pos)==U'⟶' );
        pos=0; expect( text::extract_codepoint<UTF32BE>("\0\x01\xF3\x4C"sv, pos)==U'🍌' );
       };

    test("append_codepoint") = []
       {
        using enum text::enc_t;
        //text::write_codepoint<UTF8>(U'🍌', const std::string bytes, std::size_t& pos);
       };

};///////////////////////////////////////////////////////////////////////////
#endif // TESTING ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

    //using namespace boost::ut;
    //expect(1_i == 2);       // UDL syntax
    //expect(1 == 2_i);       // UDL syntax
    //expect(that % 1 == 2);  // Matcher syntax
    //expect(eq(1, 2));       // eq/neq/gt/ge/lt/le

    //expect((1 == 2_i) >> fatal); // fatal assertion
    //expect(42l == 42_l and 1 == 2_i) << "additional info";

    //"hello world"_test = [] { ... };
    //test("hello world") = [] { ... };

    //"logging"_test = [] {
    //  log << "pre";
    //  expect(42_i == 43) << "message on failure";
    //  log << "post";
    //};

    //for (const auto& i : std::vector{1, 2, 3}) {
    //  test("parameterized " + std::to_string(i)) = [i] { // 3 tests
    //    expect(that % i > 0); // 3 asserts
    //  };
    //}

    //"matchers"_test = [] {
    //  constexpr auto is_between = [](auto lhs, auto rhs) {
    //    return [=](auto value) {
    //      return that % value >= lhs and that % value <= rhs;
    //    };
    //  };
    //
    //  expect(is_between(1, 100)(42));
    //  expect(not is_between(1, 100)(0));
    //};

    // "exception/aborts"_test = [] {
    //   expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; })) << "throws runtime_error";
    //   expect(throws([] { throw 0; })) << "throws any exception";
    //   expect(nothrow([]{})) << "doesn't throw";
    //   expect(aborts([] { assert(false); }));
    // };

    //"[vector]"_test = []
    //   {
    //    std::vector<int> v(5);
    //
    //    expect((5_ul == std::size(v)) >> fatal);
    //
    //    should("resize bigger") = [v]
    //       {
    //        mut(v).resize(10);
    //        expect(10_ul == std::size(v));
    //       };
    //
    //    expect((5_ul == std::size(v)) >> fatal);
    //
    //    should("resize smaller") = [=]() mutable
    //       {
    //        v.resize(0);
    //        expect(0_ul == std::size(v));
    //       };
    //   };

    //test("vector") = []
    //   {
    //    given("I have a vector") = []
    //       {
    //        std::vector<int> v(5);
    //        expect((5_ul == std::size(v)) >> fatal);
    //
    //        when("I resize bigger") = [=]
    //           {
    //            mut(v).resize(10);
    //
    //            then("The size should increase") = [=]
    //               {
    //                expect(10_ul == std::size(v));
    //               };
    //           };
    //      };
    //   };

    //    "operators"_test = [] {
    //      expect(0_i == sum());
    //      expect(2_i != sum(1, 2));
    //      expect(sum(1) >= 0_i);
    //      expect(sum(1) <= 1_i);
    //    };
    //
    //    "message"_test = [] { expect(3_i == sum(1, 2)) << "wrong sum"; };
    //
    //    "expressions"_test = [] {
    //      expect(0_i == sum() and 42_i == sum(40, 2));
    //      expect(1_i == sum() or 0_i == sum());
    //      expect(1_i == sum() or (sum() != 0_i or sum(1) > 0_i)) << "compound";
    //    };
    //
    //    "that"_test = [] {
    //      expect(that % 0 == sum());
    //      expect(that % 42 == sum(40, 2) and that % (1 + 2) == sum(1, 2));
    //      expect(that % 1 != 2 or 2_i > 3);
    //    };
    //
    //    "eq/neq/gt/ge/lt/le"_test = [] {
    //      expect(eq(42, sum(40, 2)));
    //      expect(neq(1, 2));
    //      expect(eq(sum(1), 1) and neq(sum(1, 2), 2));
    //      expect(eq(1, 1) and that % 1 == 1 and 1_i == 1);
    //    };
    //
    //    "floating points"_test = [] {
    //      expect(42.1_d == 42.101) << "epsilon=0.1";
    //      expect(42.10_d == 42.101) << "epsilon=0.01";
    //      expect(42.10000001 == 42.1_d) << "epsilon=0.1";
    //    };
    //
    //    "strings"_test = [] {
    //      using namespace std::literals::string_view_literals;
    //      using namespace std::literals::string_literals;
    //
    //      expect("str"s == "str"s);
    //      expect("str1"s != "str2"s);
    //
    //      expect("str"sv == "str"sv);
    //      expect("str1"sv != "str2"sv);
    //
    //      expect("str"sv == "str"s);
    //      expect("str1"sv != "str2"s);
    //      expect("str"s == "str"sv);
    //      expect("str1"s != "str2"sv);
    //    };
    //
    //    "types"_test = [] {
    //      expect(type<int> == type<int>);
    //      expect(type<float> != type<double>);
    //
    //      [[maybe_unused]] const auto i = 0;
    //      expect(type<const int> == type<decltype(i)>);
    //      expect(type<decltype(i)> != type<int>);
    //    };
    //
    //    "containers"_test = [] {
    //      std::vector v1{1, 2, 3};
    //      std::vector v2{1, 2, 3};
    //      expect(v1 == v2);
    //      expect(std::vector{"a", "b"} != std::vector{"c"});
    //      expect(std::array{true, false} == std::array{true, false});
    //    };
    //
    //    "constant"_test = [] {
    //      constexpr auto compile_time_v = 42;
    //      auto run_time_v = 99;
    //      expect(constant<42_i == compile_time_v> and run_time_v == 99_i);
    //    };
    //
    //    "convertible"_test = [] {
    //      expect(bool(std::make_unique<int>()));
    //      expect(not bool(std::unique_ptr<int>{}));
    //    };
    //
    //    "boolean"_test = [] {
    //      expect("true"_b);
    //      expect("true"_b or not "true"_b);
    //      expect((not "true"_b) != "true"_b);
    //      expect("has value"_b == "value is set"_b);
    //    };


//---------------------------------------------------------------------------
int main()
{
    //return boost::ut::run({.report_errors = true});
}
