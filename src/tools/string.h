// This file define some functions to manipulate strings.
#ifndef __GALOTFA_STRING_H__
#define __GALOTFA_STRING_H__
#include <string>
#include <vector>
namespace galotfa {
namespace string {
    std::string                trim( std::string str, std::string blank = " \t\n\r\f\v" );
    std::vector< std::string > split( std::string str, std::string delimiter = " " );
    inline std::string         replace( std::string str, std::string from, std::string to );
}  // namespace string
}  // namespace galotfa

#ifdef debug_string
namespace unit_test {
int test_trim( void );
int test_split( void );
int test_replace( void );
}  // namespace unit_test
#endif



#endif
