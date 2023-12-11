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
#include "text.hpp" // text::*


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
    throw std::runtime_error( fmt::format("Unrecognized project type: {}", prj_pth.filename().string()) );
}


//---------------------------------------------------------------------------
//template<text::enc_t enc> void parse(const std::string_view buf)
//   {
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
//   }

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
            //parse<UTF8>(buf);
            break;

        case UTF16LE:
            //parse<UTF16LE>(buf);
            break;

        case UTF16BE:
            //parse<UTF16BE>(buf);
            break;

        case UTF32LE:
            //parse<UTF32LE>(buf);
            break;

        case UTF32BE:
            //parse<UTF32BE>(buf);
            break;
       }

    // Write
    //if( out_pth.empty )
    //   {
    //   }
    
    issues.push_back("Doing nothing for now");
}

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
