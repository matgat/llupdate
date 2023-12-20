#pragma once
//  ---------------------------------------------
//  Parse xml format
//  ---------------------------------------------
//  #include "xml-parser.hpp" // xml::Parser
//  ---------------------------------------------
#include <algorithm> // std::min

#include "parser-base.hpp" // MG::parse_error, MG::ParserBase
#include "string_map.hpp" // MG::string_map<>



//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace xml
{


/////////////////////////////////////////////////////////////////////////////
class ParserEvent final
{
 public:
    using Attributes = MG::string_map<std::u32string, std::optional<std::u32string>>;

 private:
    std::u32string m_value;
    Attributes m_attributes;
    enum class type : char
      {
       COMMENT =0, // <!-- ... -->
       TEXT, // >...<
       OPENTAG, // <tag attr1 attr2=val>
       CLOSETAG, // </tag> or />
       PROCINST, // <? ... ?>
       SPECIALBLOCK, // <!xxx ... !>
       NONE
      } m_type = type::NONE;

 public:
    constexpr void set_as_none() noexcept
       {
        m_type = type::NONE;
        m_value = {};
        m_attributes.clear();
       }

    constexpr void set_as_comment(std::u32string&& cmt) noexcept
       {
        m_type = type::COMMENT;
        m_value = std::move(cmt);
        m_attributes.clear();
       }

    constexpr void set_as_text(std::u32string&& txt) noexcept
       {
        m_type = type::TEXT;
        m_value = std::move(txt);
        m_attributes.clear();
       }

    constexpr void set_as_open_tag(std::u32string&& nam)
       {
        m_type = type::OPENTAG;
        m_value = std::move(nam);
        m_attributes.clear();
        if( m_value.empty() )
           {
            throw std::runtime_error("Empty open tag");
           }
       }

    constexpr void set_as_close_tag(std::u32string&& nam)
       {
        m_type = type::CLOSETAG;
        m_value = std::move(nam);
        m_attributes.clear();
        if( m_value.empty() )
           {
            throw std::runtime_error("Empty open tag");
           }
       }

    constexpr void set_as_proc_instr(std::u32string&& nam) noexcept
       {
        m_type = type::PROCINST;
        m_value = std::move(nam);
        m_attributes.clear();
       }

    constexpr void set_as_special_block(std::u32string&& nam) noexcept
       {
        m_type = type::SPECIALBLOCK;
        m_value = std::move(nam);
        m_attributes.clear();
       }

    [[nodiscard]] constexpr operator bool() const noexcept { return m_type!=type::NONE; }
    [[nodiscard]] constexpr bool is_comment() const noexcept { return m_type==type::COMMENT; }
    [[nodiscard]] constexpr bool is_text() const noexcept { return m_type==type::TEXT; }
    [[nodiscard]] constexpr bool is_open_tag() const noexcept { return m_type==type::OPENTAG; }
    [[nodiscard]] constexpr bool is_close_tag() const noexcept { return m_type==type::CLOSETAG; }
    [[nodiscard]] constexpr bool is_proc_instr() const noexcept { return m_type==type::PROCINST; }
    [[nodiscard]] constexpr bool is_special_block() const noexcept { return m_type==type::SPECIALBLOCK; }

    [[nodiscard]] constexpr bool is_open_tag(const std::u32string_view nam) const noexcept { return m_type==type::OPENTAG && m_value==nam; }
    [[nodiscard]] constexpr bool is_close_tag(const std::u32string_view nam) const noexcept { return m_type==type::CLOSETAG && m_value==nam; }

    [[nodiscard]] constexpr std::u32string const& value() const noexcept { return m_value; }

    [[nodiscard]] constexpr Attributes const& attributes() const noexcept { return m_attributes; }
    [[nodiscard]] constexpr Attributes& attributes() noexcept { return m_attributes; }
};



/////////////////////////////////////////////////////////////////////////////
template<text::Enc enc>
class Parser final : public MG::ParserBase<enc>
{
 private:
    ParserEvent m_event; // Current event
    bool m_emit_tag_close_event = false; // To signal a deferred tag close

    class Options final
       {
        private:
            bool m_enab_cmt_events = false; // Create event on comments

        public:
            [[nodiscard]] constexpr bool is_enable_comment_events() const noexcept { return m_enab_cmt_events; }
            constexpr void set_enable_comment_events(const bool b =true) noexcept { m_enab_cmt_events = b; }

       } m_Options;

 public:
    explicit Parser(const std::string_view bytes) noexcept
      : MG::ParserBase<enc>(bytes)
       {}

    [[nodiscard]] constexpr Options const& options() const noexcept { return m_Options; }
    [[nodiscard]] constexpr Options& options() noexcept { return m_Options; }

    [[nodiscard]] constexpr ParserEvent const& curr_event() const noexcept { return m_event; }
    [[nodiscard]] constexpr ParserEvent& mutable_curr_event() noexcept { return m_event; }

    [[nodiscard]] constexpr ParserEvent const& next_event()
       {
        //if( m_emit_tag_close_event )
        //   {
        //    m_emit_tag_close_event = false; // eat
        //    m_event.set_as_close_tag( m_event.value() );
        //   }
        //else
        //   {
        //    try{
        //        while( true )
        //           {
        //            skip_any_space();
        //            if( i>=siz )
        //               {// No more data!
        //                m_event.set_as_none();
        //                break;
        //               }
        //            else if( eat('<') )
        //               {
        //                check_xml_markup();
        //               }
        //            else
        //               {
        //                m_event.set_as_text( collect_text() );
        //                break;
        //               }
        //           }
        //       }
        //    catch(MG::parse_error&)
        //       {
        //        throw;
        //       }
        //    catch(std::runtime_error& e)
        //       {
        //        throw MG::parse_error(e.Message, m_filepath, line, i<=i_last ? i : i_last);
        //       }
        //   }

        return m_event;
       }


 private:

/*

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr bool check_xml_markup()
       {
        if( i>=siz )
           {
            throw std::runtime_error("Unclosed <");
           }

        // Could be a tag, comment, special block or processing instruction
        if( eat('!') )
           {// Could be a comment or a special block
            if( i>=siz )
               {
                throw std::runtime_error("Unclosed <!");
               }
            else if( eat("--") )
               {// A comment
                skip_any_space();
                const std::string_view cmt = collect_until('-','-','>');
                if( options().is_enable_comment_events() )
                   {
                    m_event.set_as_comment( cmt );
                    break;
                   }
               }
            else if( eat('[') )
               {// A conditional or <![CDATA[ section
                if( eat("CDATA[") )
                   {
                    skip_any_space();
                    m_event.set_as_text( collect_until(']',']','>') );
                    break;
                   }
                else
                   {
                    throw std::runtime_error("Conditional sections not yet supported");
                   }
               }
            else
               {// A special block: <!DOCTYPE, ...
                m_event.set_as_special_block( collect_until(']','>') );
                break;
               }
           }
        else if( eat('?') )
           {// A processing instruction
            m_event.set_as_proc_instr( collect_until('?','>') );
            break;
           }
        else if( eat('/') )
           {// A close tag
            m_event.set_as_close_tag( collect_tag_name() );
            skip_any_space();
            if( i>=siz || !eat('>') )
               {
                throw std::runtime_error("Invalid close tag");
               }
            break;
           }
        else
           {// A normal markup element (tag)
            collect_tag( m_event );
            break;
           }
       }

    //-----------------------------------------------------------------------
    constexpr void collect_tag(ParserEvent& ev)
       {
        const std::size_t line_start = line;
        const std::size_t i_start = i;
        ev.set_as_open_tag( collect_tag_name() );

        // Collect attributes
        auto namval = collect_attribute();
        while( !namval.first.is_empty() )
           {
            if( ev.attributes().contains() )
               {
                notify_issue();
               }
            ev.attributes().insert_unique(namval.first, namval.second);
            namval = collect_attribute();
           }

        // Detect tag close
        if( i<siz && eat('/') )
           {
            m_emit_tag_close_event = true;
           }

        // Expect >
        if( i>=siz || !eat('>') )
           {
            throw MG::parse_error(fmt::format("Tag {} must be closed with >",std::string(ev.value())), m_filepath, line_start, i_start);
           }
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr std::string_view collect_tag_name()
       {
        skip_any_space();
        const std::size_t i_start = i;
        while( i<siz && !is_space() && isnt(U'>') && isnt(U'/') )
           {
            if( !is_good_for_tag_name() )
               {
                throw std::runtime_error( fmt::format("Character '%c' not allowed in tag name",buf[i]) );
               }
            ++i;
           }
        return std::string_view(buf+i_start, i-i_start);
       }
    [[nodiscard]] constexpr bool is_good_for_tag_name() noexcept
       {
        return is_punct() && isnt(U'-') && isnt(U':');
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr std::pair<std::string_view,std::string_view> collect_attribute()
       {
        std::pair<std::string_view,std::string_view> namval;

        skip_any_space();
        if( i<siz )
           {// Collect attribute name
            namval.first = buf[i]=='\"' ? collect_inner_same_line('\"','\"','\\')
                                        : collect_unquoted_attr_name();
            if( !namval.first.is_empty() )
               {// Check possible value
                skip_any_space();
                if( i<siz )
                   {
                    if( eat('=') )
                       {
                        skip_any_space();
                        if( i<siz )
                           {// Collect attribute value
                            namval.second = buf[i]=='\"' ? collect_inner_same_line('\"','\"','\\')
                                                         : collect_unquoted_attr_value();
                           }
                        else
                           {
                            throw std::runtime_error("Truncated attribute value");
                           }
                       }
                   }
               }
           }
        return namval;
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr std::string_view collect_unquoted_attr_name()
       {
        assertm( i<siz && !is_space(), "collect_unquoted_attr_name() expects non-space char" );
        const std::size_t i_start = i;
        do {
            if( is(U'=') || is(U'>') || is(U'/') )
               {
                break;
               }
            else if( is_good_for_attrib_name() )
               {
                throw std::runtime_error( fmt::format("Character '%c' not allowed in attribute name",buf[i]) );
               }
            ++i;
           }
        while( i<siz && !is_space() );
        return std::string_view(buf+i_start, i-i_start);
       }
    [[nodiscard]] constexpr bool is_good_for_attrib_name() noexcept
       {
        return is_punct() && isnt(U'-');
       }

    //-----------------------------------------------------------------------
    [[nodiscard]] constexpr std::string_view collect_unquoted_attr_value()
       {
        assertm( i<siz && !is_space(), "collect_unquoted_attr_value() expects non-space char" );
        const std::size_t i_start = i;
        do {
            if( is(U'>') || is(U'/') )
               {
                break;
               }
            else if( is(U'=') || buf[i]=='\"' )
               {
                throw std::runtime_error( fmt::format("Character '%c' not allowed in attribute value",buf[i]) );
               }
            ++i;
           }
        while( i<siz && !is_space() );
        return std::string_view(buf+i_start, i-i_start);
       }
*/
};


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



/////////////////////////////////////////////////////////////////////////////
#ifdef TEST_UNITS ///////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
[[nodiscard]] constexpr std::string to_string(xml::ParserEvent const& ev)
   {
    if( ev.is_open_tag() )
        return fmt::format("<{}>", text::to_utf8(ev.value()));
        //return fmt::format("<{} {}>", text::to_utf8(ev.value()), text::to_utf8(to_string(ev.attributes())));

    else if( ev.is_close_tag() )
        return fmt::format("</{}>", text::to_utf8(ev.value()));

    else if( ev.is_comment() )
        return fmt::format("<!-- {} -->", text::to_utf8(ev.value()));

    else if( ev.is_text() )
        return fmt::format("{}", text::to_utf8(ev.value()));

    else if( ev.is_proc_instr() )
        return fmt::format("<?{}?>", text::to_utf8(ev.value()));

    else if( ev.is_special_block() )
        return fmt::format("<!{}!>", text::to_utf8(ev.value()));

    return "(none)";
   }
/////////////////////////////////////////////////////////////////////////////
static ut::suite<"xml::Parser"> XmlParser_tests = []
{////////////////////////////////////////////////////////////////////////////
    using namespace std::literals; // "..."sv
    using ut::expect;
    using ut::that;
    using ut::throws;


    ut::test("xml::ParserEvent") = []
       {
        xml::ParserEvent event;

        expect( that % event == false) << "empty event should evaluate false\n";

        event.set_as_comment(U"cmt"s);
        expect( that % event.is_comment() and event.value()==U"cmt"sv ) << "should be a comment event\n";

        event.set_as_text(U"txt"s);
        expect( that % event.is_text() and event.value()==U"txt"sv ) << "should be a text event\n";

        event.set_as_open_tag(U"otag"s);
        expect( that % event.is_open_tag(U"otag"sv) ) << "should be an open tag event\n";

        event.set_as_close_tag(U"ctag"s);
        expect( that % event.is_close_tag(U"ctag"sv) ) << "should be a close tag event\n";

        event.set_as_proc_instr(U"proc"s);
        expect( that % event.is_proc_instr() and event.value()==U"proc"sv ) << "should be a proc-instr event\n";

        event.set_as_special_block(U"block"s);
        expect( that % event.is_special_block() and event.value()==U"block"sv ) << "should be a special block event\n";
       };


    //auto notify = [](const std::string_view msg) -> void { ut::log << msg; };
    //ut::test("xml::Parser") = []
    //   {
    //    xml::Parser<text::Enc::UTF8> parser{""sv};
    //   };

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
