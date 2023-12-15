#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp" // import boost.ut;
namespace ut = boost::ut;

#define TEST_UNITS // Include units embedded tests
#include "text.hpp" // text::*
#include "vectmap.hpp" // MG::vectmap<>
#include "parser-base.hpp" // MG::ParserBase
//#include "parser-xml.hpp" // xml::Parser
//#include "project-updater.hpp" // ll::update_project()

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
static ut::suite<"xml::Parser"> XmlParser_tests = []
{////////////////////////////////////////////////////////////////////////////
    using namespace std::literals; // "..."sv
    using ut::expect;
    using ut::that;
    using ut::throws;
    using enum text::Enc;

    //auto notify = [](const std::string_view msg) -> void { ut::log << msg; };

    ut::test("empty") = []
       {
        //xml::Parser<UTF8> parser{""sv};
       };

/*
    //-----------------------------------------------------------------------
    void test_generic(const char* const title)
       {
        log(fmt::format("  -- {} --",title) );
        const std::string_view buf = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                              "<!DOCTYPE doctype [\n"
                              "<!ELEMENT root (child+)>\n"
                              "]>\n"
                              "<!-- comment -->\n"
                              "<tag1/><tag2 attr1=\"1\" attr2=2 attr3/>\n"
                              "<tag3>blah</tag3>\n"
                              "< nms:tag4 \n attr1=\"1&lt;2\" \n attr2=\"2\" \n >blah</ nms:tag4 >\n"
                              "  some text\n"
                              "<![CDATA[\n"
                              "  Some <>not parsed<> text\n"
                              "]]>\n"
                              "<root>\n"
                              "    <child key1=123 key2=\"quoted value\"/>\n"
                              "    <child key1 key2=\"blah blah\">\n"
                              "        &apos;text&apos;\n"
                              "        <subchild>\n"
                              "            text text text\n"
                              "            text text text\n"
                              "        </subchild>\n"
                              "    </child>\n"
                              "</root>\n";

        xml::Parser parser(title, buf, &log);
        parser.set_fussy();
        parser.options().set_enable_comment_events();

        std::size_t k = 0;
        while( const xml::ParserEvent& event = parser.next_event() )
           {
            switch( k )
               {
                case  0: TEST_EXPECTM(event.is_proc_instr(), event.string()); break;
                case  1: TEST_EXPECTM(event.is_special_block(), event.string()); break;
                case  2: TEST_EXPECTM(event.is_comment(), event.string()); break;
                case  3: TEST_EXPECTM(event.is_open_tag("tag1") && event.attributes().size()==0, event.string()); break;
                case  4: TEST_EXPECTM(event.is_close_tag("tag1"), event.string()); break;
                case  5: TEST_EXPECTM(event.is_open_tag("tag2") && event.attributes().size()==3, event.string()); break;
                case  6: TEST_EXPECTM(event.is_close_tag("tag2"), event.string()); break;
                case  7: TEST_EXPECTM(event.is_open_tag("tag3") && event.attributes().size()==0, event.string()); break;
                case  8: TEST_EXPECTM(event.is_text(), event.string()); break;
                case  9: TEST_EXPECTM(event.is_close_tag("tag3"), event.string()); break;
                case 10: TEST_EXPECTM(event.is_open_tag("nms:tag4") && event.attributes().size()==2, event.string()); break;
                case 11: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 12: TEST_EXPECTM(event.is_close_tag("nms:tag4"), event.string()); break;
                case 13: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 14: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 15: TEST_EXPECTM(event.is_open_tag("root") && event.attributes().size()==0, event.string()); break;
                case 16: TEST_EXPECTM(event.is_open_tag("child") && event.attributes().size()==2, event.string()); break;
                case 17: TEST_EXPECTM(event.is_close_tag("child"), event.string()); break;
                case 18: TEST_EXPECTM(event.is_open_tag("child") && event.attributes().size()==2, event.string()); break;
                case 19: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 20: TEST_EXPECTM(event.is_open_tag("subchild") && event.attributes().size()==0, event.string()); break;
                case 21: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 22: TEST_EXPECTM(event.is_close_tag("subchild"), event.string()); break;
                case 23: TEST_EXPECTM(event.is_close_tag("child"), event.string()); break;
                case 24: TEST_EXPECTM(event.is_close_tag("root"), event.string()); break;

                default: err("Unexpected event " + event.string());
               }
            ++k;
           }
        TEST_EXPECT(k==25);
       }

    //-----------------------------------------------------------------------
    void test_bad(const char* const title)
       {
        log(fmt::format("  -- {} --",title) );
        std::string_view buf = "<!--\n\n\n\n";
        xml::Parser parser(title, buf, &log);
        TEST_EXPECT_EXCEPTION( parser.next_event() );
       }

    //-----------------------------------------------------------------------
    void test_interface(const char* const title)
       {
        log(fmt::format("  -- {} --",title) );
        const std::string_view buf = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                              "<?xml-stylesheet type=\"text/xsl\" href=\"Interface2XHTML.xsl\"?>\n"
                              "<!--\n"
                              "    Dizionario interfaccia unificata macchine Macotec\n"
                              "    ©2017-2022 gattanini@macotec.it\n"
                              "-->\n"
                              "<interface version=\"2022-09-06\"\n"
                              "           name=\"MacoLayer\"\n"
                              "           xmlns=\"http://www.macotec.it\">\n"
                              "\n"
                              "<!-- +------------------------------------------------------------------+\n"
                              "     ¦ Statistics and maintenance                                       ¦\n"
                              "     +------------------------------------------------------------------+ -->\n"
                              "<group name=\"statistics\">\n"
                              "\n"
                              "    <res    id=\"sheets-done\" tags=\"statistics,counter,sheets,done\" access=\"r\"\n"
                              "            type=\"int\">\n"
                              "        <text lang=\"en\" label=\"Done sheets\">Processed sheets count</text>\n"
                              "        <text lang=\"it\" label=\"Lastre lavorate\">Contatore lastre lavorate</text>\n"
                              "    </res>\n"
                              "\n"
                              "    <res    id=\"buffer-width\" tags=\"settings,machine,modules,size,width,buffer\" access=\"r\"\n"
                              "            type=\"double\" quantity=\"length\" unit=\"mm\" unit-coeff=\"0.001\"\n"
                              "            range=\"0:6000\" gran=\"0.1\" default=\"2400\">\n"
                              "        <text lang=\"en\" label=\"Buffer width\">Buffer longitudinal size</text>\n"
                              "        <text lang=\"it\" label=\"Largh polmone\">Larghezza del polmone</text>\n"
                              "    </res>\n"
                              "\n"
                              "</group> <!-- statistics -->\n"
                              "\n"
                              "</interface>\n";
        xml::Parser parser(title, buf, &log);

        std::size_t k = 0;
        while( const xml::ParserEvent& event = parser.next_event() )
           {
            switch( k )
               {
                case  0: TEST_EXPECTM(event.is_proc_instr(), event.string()); break;
                case  1: TEST_EXPECTM(event.is_proc_instr(), event.string()); break;
                case  2: TEST_EXPECTM(event.is_open_tag("interface") && event.attributes().size()==3, event.string()); break;
                case  3: TEST_EXPECTM(event.is_open_tag("group") && event.attributes().get_value_of("name")=="statistics", event.string()); break;
                case  4: TEST_EXPECTM(event.is_open_tag("res") && event.attributes().get_value_of("id")=="sheets-done", event.string()); break;
                case  5: TEST_EXPECTM(event.is_open_tag("text") && event.attributes().get_value_of("lang")=="en", event.string()); break;
                case  6: TEST_EXPECTM(event.is_text(), event.string()); break;
                case  7: TEST_EXPECTM(event.is_close_tag("text"), event.string()); break;
                case  8: TEST_EXPECTM(event.is_open_tag("text") && event.attributes().get_value_of("lang")=="it", event.string()); break;
                case  9: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 10: TEST_EXPECTM(event.is_close_tag("text"), event.string()); break;
                case 11: TEST_EXPECTM(event.is_close_tag("res"), event.string()); break;
                case 12: TEST_EXPECTM(event.is_open_tag("res") && event.attributes().get_value_of("id")=="buffer-width", event.string()); break;
                case 13: TEST_EXPECTM(event.is_open_tag("text") && event.attributes().get_value_of("lang")=="en", event.string()); break;
                case 14: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 15: TEST_EXPECTM(event.is_close_tag("text"), event.string()); break;
                case 16: TEST_EXPECTM(event.is_open_tag("text") && event.attributes().get_value_of("lang")=="it", event.string()); break;
                case 17: TEST_EXPECTM(event.is_text(), event.string()); break;
                case 18: TEST_EXPECTM(event.is_close_tag("text"), event.string()); break;
                case 19: TEST_EXPECTM(event.is_close_tag("res"), event.string()); break;
                case 20: TEST_EXPECTM(event.is_close_tag("group"), event.string()); break;
                case 21: TEST_EXPECTM(event.is_close_tag("interface"), event.string()); break;
                default: err("Unexpected event " + event.string());
               }
            ++k;
           }
        TEST_EXPECT(k==22);
       }
*/
       
};///////////////////////////////////////////////////////////////////////////
#endif // TEST_UNITS ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////




    //using namespace boost::ut;

    //skip / "skipped test"_test = [] { };

    //expect(1_i == 2);
    //expect( fatal(1 == 2_i) );
    //expect(that % 1 == 2);  // Matcher syntax
    //expect(eq(1, 2));       // eq/neq/gt/ge/lt/le

    //expect((1 == 2_i) >> fatal); // fatal assertion
    //expect(42l == 42_l and 1 == 2_i) << "additional info\n";

    //"lazy log"_test = [] {
    //  std::expected<bool, std::string> e = std::unexpected("lazy evaluated");
    //  expect(e.has_value()) << [&] { return e.error(); } << fatal;
    //  expect(e.value() == true);
    //};


    //"hello world"_test = [] { ... };
    //test("hello world") = [] { ... };

    //"logging"_test = [] {
    //  log << "pre " << 42;
    //  expect(42_i == 43) << "message on failure\n";
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
    //   expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; })) << "should throw runtime_error\n";
    //   expect(throws([] { throw 0; })) << "should throw any exception\n";
    //   expect(nothrow([]{})) << "should't throw\n";
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
    //    "message"_test = [] { expect(3_i == sum(1, 2)) << "wrong sum\n"; };
    //
    //    "expressions"_test = [] {
    //      expect(0_i == sum() and 42_i == sum(40, 2));
    //      expect(1_i == sum() or 0_i == sum());
    //      expect(1_i == sum() or (sum() != 0_i or sum(1) > 0_i)) << "compound\n";
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
    //      expect(42.1_d == 42.101) << "epsilon=0.1\n";
    //      expect(42.10_d == 42.101) << "epsilon=0.01\n";
    //      expect(42.10000001 == 42.1_d) << "epsilon=0.1\n";
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
//  "don't run"_test = [] { expect(0 == 1_i) << "don't run\n"; };
//}
