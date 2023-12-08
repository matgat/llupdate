#include <array>

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp" // import boost.ut;
namespace tst = boost::ut;

#define TEST_UNITS // Include units embedded tests
#include "../source/text.hpp" // text::buffer_t

// Custom reporter
//#include <iostream>
//namespace ft {
//template <class TReporter>
//struct runner : tst::runner<TReporter> {
//  template <class... Ts>
//  auto on(tst::events::test<Ts...> test) {
//    std::cout << test.name << '\n';
//    tst::runner<TReporter>::on(test);
//  }
//
//  using tst::runner<TReporter>::on;
//};
//}  // namespace ft
//
//template <class... Ts>
//inline auto tst::cfg<tst::override, Ts...> = ft::runner<tst::reporter<>>{};



/////////////////////////////////////////////////////////////////////////////
#ifdef TEST_UNITS ///////////////////////////////////////////////////////////
static tst::suite<"text tests"> text_tests = []
{ ///////////////////////////////////////////////////////////////////////////
    using namespace tst;
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

    test("codepoints decode and encode") = []
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
           };
        constexpr std::array<test_case_t,4> test_cases =
           {{
             { U'a', "\x61"sv, "\x61\0"sv, "\0\x61"sv, "\x61\0\0\0"sv, "\0\0\0\x61"sv }
            ,{ U'à', "\xC3\xA0"sv, "\xE0\0"sv, "\0\xE0"sv, "\xE0\0\0\0"sv, "\0\0\0\xE0"sv }
            ,{ U'⟶', "\xE2\x9F\xB6"sv, "\xF6\x27"sv, "\x27\xF6"sv, "\xF6\x27\0\0"sv, "\0\0\x27\xF6"sv }
            ,{ U'🍌', "\xF0\x9F\x8D\x8C"sv, "\x3C\xD8\x4C\xDF"sv, "\xD8\x3C\xDF\x4C"sv, "\x4C\xF3\x01\0"sv, "\0\x01\xF3\x4C"sv }
           }};

      #define TEST_ENC(ENC) \
           { \
            log << "Testing " << test_case.UTF8_encoded << " with " #ENC; \
            std::size_t pos{}; \
            expect( text::extract_codepoint<ENC>(test_case.ENC##_encoded, pos) == test_case.code_point ); \
            std::string bytes; \
            text::append_codepoint<ENC>(test_case.code_point, bytes); \
            expect( test_case.ENC##_encoded == bytes ); \
            if( test_case.ENC##_encoded!=bytes ) \
               { \
                log << #ENC " failed "; \
                log << "original: "; \
                for(const char ch : test_case.ENC##_encoded) \
                    { \
                     log << "0x" << std::hex << static_cast<unsigned short>(ch) << ' '; \
                    } \
                log << "encoded: "; \
                for(const char ch : bytes) \
                    { \
                     log << "0x" << std::hex << static_cast<unsigned short>(ch) << ' '; \
                    } \
               } \
           }

        using enum text::enc_t;
        for(const auto& test_case : test_cases)
           {
            test(test_case.UTF8_encoded) = [&test_case]
               {
                TEST_ENC(UTF8)
                TEST_ENC(UTF16LE)
                TEST_ENC(UTF16BE)
                TEST_ENC(UTF32LE)
                TEST_ENC(UTF32BE)
               };
           }
      #undef TEST_ENC
       };

    //test("xxx") = []
    //   {
    //    using enum text::enc_t;
    //   };

};///////////////////////////////////////////////////////////////////////////
#endif // TESTING ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

    //using namespace boost::ut;

    //skip / "skipped test"_test = [] { };

    //expect(1_i == 2);
    //expect( fatal(1 == 2_i) );
    //expect(that % 1 == 2);  // Matcher syntax
    //expect(eq(1, 2));       // eq/neq/gt/ge/lt/le

    //expect((1 == 2_i) >> fatal); // fatal assertion
    //expect(42l == 42_l and 1 == 2_i) << "additional info";

    //"lazy log"_test = [] {
    //  std::expected<bool, std::string> e = std::unexpected("lazy evaluated");
    //  expect(e.has_value()) << [&] { return e.error(); } << fatal;
    //  expect(e.value() == true);
    //};


    //"hello world"_test = [] { ... };
    //test("hello world") = [] { ... };

    //"logging"_test = [] {
    //  log << "pre " << 42;
    //  expect(42_i == 43) << "message on failure";
    //  log << "post " << 43;
    //};

    // expect(type<int> == type<int>);

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
    //   expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; })) << "should throw runtime_error";
    //   expect(throws([] { throw 0; })) << "should throw any exception";
    //   expect(nothrow([]{})) << "should't throw";
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
    //    using namespace boost::ut::bdd;
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

// cli
//int main(int argc, const char** argv)
//{
//  using namespace boost::ut;
//
//  cfg<override> = {.filter = argc > 1 ? argv[1] : "",
//                   .colors = argc > 2 and argv[2][0] == '0'
//                                 ? colors{.none = "", .pass = "", .fail = ""}
//                                 : colors{},
//                   .dry_run = argc > 3 ? argv[3][0] == '1' : false};
//
//  "cli"_test = [] {
//    "pass"_test = [] { expect(42 == 42_i); };
//    "fail"_test = [] { expect(0 == 42_i); };
//  };
//}

//filtering tests
//int main()
//{
//  using namespace boost::ut;
//
//  cfg<override> = {.filter = "run.sub1"};
//
//  "run"_test = [] {
//    "sub1"_test = [] { expect(42 == 42_i); };
//    "sub2"_test = [] { expect(43 == 42_i); };
//  };
//
//  "don't run"_test = [] { expect(0 == 1_i) << "don't run"; };
//}
