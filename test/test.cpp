#include <array>

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp" // import boost.ut;
namespace ut = boost::ut;

#define TEST_UNITS // Include units embedded tests
#include "../source/text.hpp" // text::*
#include "../source/parser-base.hpp" // MG::ParserBase

// Custom reporter
//#include <iostream>
//namespace ft {
//template <class TReporter>
//struct runner : ut::runner<TReporter> {
//  template <class... Ts>
//  auto on(ut::events::test<Ts...> test) {
//    std::cout << test.name << '\n';
//    ut::runner<TReporter>::on(test);
//  }
//
//  using ut::runner<TReporter>::on;
//};
//}  // namespace ft
//
//template <class... Ts>
//inline auto ut::cfg<ut::override, Ts...> = ft::runner<ut::reporter<>>{};



/////////////////////////////////////////////////////////////////////////////
#ifdef TEST_UNITS ///////////////////////////////////////////////////////////
static ut::suite<"MG::ParserBase"> ParserBase_tests = []
{ ///////////////////////////////////////////////////////////////////////////
    using ut::expect;
    using namespace std::literals; // "..."sv

    ut::test("empty") = []
       {
        MG::ParserBase<text::Enc::UTF8> parser{""sv};
        expect( parser.curr_codepoint() == text::null_codepoint );
        //expect(throws([] { parser.get_next() })) << "should complain if...";
       };

    ut::test("simple utf-8") = []
       {
        MG::ParserBase<text::Enc::UTF8> parser{""sv};
        expect( parser.curr_codepoint() == text::null_codepoint );
       };

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
