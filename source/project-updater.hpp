#pragma once
//  ---------------------------------------------
//  Updates the libraries in a LogicLab project
//  ---------------------------------------------
#include <stdexcept> // std::runtime_error
#include <string>
#include <string_view>
//#include <vector>
#include <fmt/core.h> // fmt::*
#include <filesystem> // std::filesystem
namespace fs = std::filesystem;

#include "memory_mapped_file.hpp" // sys::memory_mapped_file
#include "parser-xml.hpp" // xml::Parser


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace ll
{

//---------------------------------------------------------------------------
enum class project_type : std::uint8_t { ppjs, plcprj };
[[nodiscard]] project_type recognize_project_type( const fs::path& prj_pth )
{
    // The comparison should be case insensitive?
    const std::string ext{ prj_pth.extension().string() };
    if( ext==".ppjs" )
       {
        return project_type::ppjs;
       }
    else if( ext==".plcprj" )
       {
        return project_type::plcprj;
       }
    throw std::runtime_error( fmt::format("Unrecognized project type: {}", ext) );
}


//---------------------------------------------------------------------------
template<text::Enc enc> void parse(const std::string_view buf)
   {
    xml::Parser<enc> parser{buf};
    parser.options().set_collect_comment_text(false);
    parser.options().set_collect_text_sections(false);
    //parser.set_on_notify_issue(notify_sink);

    while( const xml::ParserEvent& event = parser.next_event() )
       {
        if( event.is_open_tag(U"lib") and event.attributes().contains(U"name") )
           {
            fmt::print("{} opened at offset:{} line:{}\n", text::to_utf8(event.attributes()[U"name"].value()), event.start_byte_offset(), parser.curr_line());
           }
        else if( event.is_close_tag(U"lib") )
           {
            fmt::print("closed at offset:{} line:{}\n", event.start_byte_offset(), parser.curr_line());
           }
       }
   }

//---------------------------------------------------------------------------
void update_project( const fs::path& prj_pth, fs::path out_pth, std::vector<std::string>& issues )
{
    const project_type prj_type = recognize_project_type(prj_pth);
    const sys::memory_mapped_file mem_mapped_file{prj_pth.string()};
    const std::string_view bytes{mem_mapped_file.as_string_view()};

    if( bytes.empty() )
       {
        throw std::runtime_error("No data to parse (empty file?)");
       }

   const auto [enc, bom_size] = text::detect_encoding_of(bytes);

   switch( prj_type )
       {using enum project_type;

        case ppjs:
            break;

        case plcprj:
            break;
       }

    switch( enc )
       {using enum text::Enc;

        case UTF8:
            parse<UTF8>(bytes);
            break;

        case UTF16LE:
            parse<UTF16LE>(bytes);
            break;

        case UTF16BE:
            parse<UTF16BE>(bytes);
            break;

        case UTF32LE:
            parse<UTF32LE>(bytes);
            break;

        case UTF32BE:
            parse<UTF32BE>(bytes);
            break;
       }

    // Write
    //if( out_pth.empty )
    //   {
    //   }

    issues.push_back("Doing nothing for now");
}

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
