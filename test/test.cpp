#define BOOST_UT_DISABLE_MODULE
//#include <https://raw.githubusercontent.com/boost-experimental/ut/master/include/boost/ut.hpp>
#include "ut.hpp" // import boost.ut;
namespace ut = boost::ut;

#define TEST_UNITS // Include units embedded tests
#include "string_map.hpp" // MG::string_map<>
#include "text.hpp" // text::*
#include "parser-base.hpp" // MG::ParserBase
#include "parser-xml.hpp" // xml::Parser
//#include "project-updater.hpp" // ll::update_project()


//---------------------------------------------------------------------------
int main()
{
    //using namespace boost::ut;
    //expect(that % 1 == 2);
}
