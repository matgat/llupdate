//  ---------------------------------------------
//  A tool that updates the external libraries in
//  a LogicLab project file (ppjs, plcprj)
//  ---------------------------------------------
#include <stdexcept> // std::runtime_error
#include <string>
#include <string_view>
using namespace std::literals; // "..."sv
#include <vector>
#include <fmt/core.h> // fmt::*
#include <filesystem> // std::filesystem
namespace fs = std::filesystem;

#include "project-updater.hpp" // ll::update_project()


/////////////////////////////////////////////////////////////////////////////
class Arguments final
{
 public:
    Arguments(const int argc, const char* const argv[])
       {
        try{
            enum class STS
               {
                SEE_ARG,
                GET_OUTPATH
               } status = STS::SEE_ARG;

            for( int i=1; i<argc; ++i )
               {
                std::string_view arg{ argv[i] };
                switch( status )
                   {
                    case STS::GET_OUTPATH :
                        if( !m_out_path.empty() )
                           {
                            throw std::invalid_argument( fmt::format("Output file was already set to {}",m_out_path.string()) );
                           }
                        m_out_path = arg;
                        status = STS::SEE_ARG;
                        break;

                    default :
                        if( arg.size()>=2 && arg[0]=='-' )
                           {// A command switch!
                            arg.remove_prefix(arg[1]=='-' ? 2 : 1); // Skip hyphen(s)
                            if( arg=="out"sv || arg=="o"sv )
                               {
                                status = STS::GET_OUTPATH;
                               }
                            else if( arg=="verbose"sv || arg=="v"sv )
                               {
                                m_verbose = true;
                               }
                            else if( arg=="help"sv || arg=="h"sv )
                               {
                                print_help();
                                throw std::invalid_argument("Aborting after printing help");
                               }
                            else
                               {
                                throw std::invalid_argument( fmt::format("Unknown command switch: {}",arg) );
                               }
                           }
                        else
                           {// This must be the project path
                            if( !m_prj_path.empty() )
                               {
                                throw std::invalid_argument( fmt::format("Project file was already set to {}",m_prj_path.string()) );
                               }

                            m_prj_path = arg;
                            if( !fs::exists(m_prj_path) )
                               {
                                throw std::invalid_argument( fmt::format("File not found: {}",m_prj_path.string()) );
                               }
                           }
                   }
               } // each argument

            // The project path must be given
            if( m_prj_path.empty() )
               {
                throw std::invalid_argument("Project file not given");
               }
            if( fs::exists(m_out_path) && fs::equivalent(m_prj_path, m_out_path) )
               {
                throw std::runtime_error( fmt::format("Specified output file \"{}\" collides with original file",m_out_path.string()) );
               }
           }
        catch( std::exception& e)
           {
            throw std::invalid_argument(e.what());
           }
       }

    static void print_help() noexcept
       {
        fmt::print( "\nllupdate (ver. " __DATE__ ")\n"
                    "A tool that updates the external libraries in a LogicLab project\n"
                    "Supported formats: ppjs, plcprj\n"
                    "\n" );
       }

    static void print_usage() noexcept
       {
        fmt::print( "\nUsage:\n"
                    "   llupdate path/to/project.ppjs\n"
                    "       --out/-o (Specify generated file)\n"
                    "       --verbose/-v (Print more info on stdout)\n"
                    "\n" );
       }

    [[nodiscard]] const fs::path& prj_path() const noexcept { return m_prj_path; }
    [[nodiscard]] const fs::path& out_path() const noexcept { return m_out_path; }
    [[nodiscard]] bool verbose() const noexcept { return m_verbose; }

 private:
    fs::path m_prj_path;
    fs::path m_out_path;
    bool m_verbose = false;
};


//---------------------------------------------------------------------------
//[[nodiscard]] fs::path get_bck_path(const fs::path& orig_path, const Arguments& args)
//{
//    fs::path out_path{ orig_path.parent_path() };
//    if( args.job().out_file_name().empty() )
//       {
//        out_path /= fmt::format("~{}.tmp", orig_path.filename().string());
//       }
//    else
//       {
//        out_path /= args.job().out_file_name();
//        if( sys::are_paths_equivalent(out_path, orig_path) )
//           {
//            throw std::runtime_error( fmt::format("Specified output file \"{}\" collides with original file",args.job().out_file_name()) );
//           }
//       }
//    return out_path;
//}



//---------------------------------------------------------------------------
//[[nodiscard]] fs::path adapt_udt(Arguments& args, std::vector<std::string>& issues)
//{
//}


//---------------------------------------------------------------------------
int main( const int argc, const char* const argv[] )
{
    try{
        Arguments args(argc, argv);
        if( args.verbose() )
           {
            fmt::print( "---- llupdate (ver. " __DATE__ ") ----\n" );
            fmt::print( "Running in: {}\n", fs::current_path().string() );
           }

        std::vector<std::string> issues;

        if( args.verbose() )
           {
            fmt::print( "Updating project {}\n", args.prj_path().string() );
           }
        ll::update_project(args.prj_path(), args.out_path(), issues);

        if( issues.size()>0 )
           {
            fmt::print("[!] {} issues found\n", issues.size());
            for( const auto& issue : issues )
               {
                fmt::print("    {}\n", issue);
               }
            return 1;
           }

        return 0;
       }

    catch( std::invalid_argument& e )
       {
        fmt::print("!! {}\n", e.what());
        Arguments::print_usage();
       }

    //catch( parse_error& e)
    //   {
    //    fmt::print("!! {} ({}:{})\n", e.what(), args.prj_path().filename().string(), e.line());
    //    sys::edit_text_file( args.prj_path().string(), e.line() );
    //   }

    catch( std::exception& e )
       {
        fmt::print("!! Error: {}\n", e.what());
       }

    return 2;
}
