// This file define some functions to manipulate strings.
#ifndef __GALOTFA_STRING_H__
#define __GALOTFA_STRING_H__
#include <string>
#include <vector>
namespace galotfa {
namespace string {
    std::string trim( std::string str, std::string blank = " \t\n\r\f\v" )
    {
        str.erase( 0, str.find_first_not_of( blank ) );
        str.erase( str.find_last_not_of( blank ) + 1 );
        // remove the possible comments
        return str;
    }

    std::vector< std::string > split( std::string str, std::string delimiter = " " )
    {
        std::vector< std::string > vals;
        while ( true )
        {
            size_t      pos    = str.find_first_of( delimiter );
            std::string substr = str.substr( 0, pos );
            if ( substr != "" )
                vals.push_back( substr );
            str.erase( 0, pos );
            str.erase( 0, str.find_first_not_of( delimiter ) );
            if ( str.find_first_of( delimiter ) == std::string::npos )
            {
                vals.push_back( str );
                break;
            }
        }
        return vals;
    }
}  // namespace string
}  // namespace galotfa

#ifdef debug_string
namespace unit_test {
int test_trim( void );
int test_split( void );
}  // namespace unit_test
#endif



#endif
