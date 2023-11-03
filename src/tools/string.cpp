#ifndef GALOTFA_STRING_CPP
#define GALOTFA_STRING_CPP
#include "./string.h"
#include "../tools/prompt.h"
#include <algorithm>
#include <string>
namespace galotfa {
std::string string::trim( std::string str, std::string blank )
{
    str.erase( 0, str.find_first_not_of( blank ) );
    str.erase( str.find_last_not_of( blank ) + 1 );
    // remove the possible comments
    return str;
}

std::vector< std::string > string::split( std::string str, std::string delimiter )
{
    if ( delimiter == "" )
    {
        WARN( "The delimiter for string split is empty!" );
        return { str };
    }
    else if ( str == "" )
    {
        WARN( "The string to be split is empty!" );
        return { "" };
    }
    std::vector< std::string > vals;
    while ( true )
    {
        size_t pos = str.find_first_of( delimiter );
        if ( pos == std::string::npos )
        {
            vals.push_back( str );
            break;
        }
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

std::string string::replace( std::string str, std::string from, std::string to )
{
    if ( from.empty() || to.empty() )
    {
        WARN( "The string replace pattern is empty!" );
        return str;
    }
    size_t pos = 0;
    while ( true )
    {
        pos = str.find( from, pos );
        if ( pos == std::string::npos )
            break;
        str.replace( pos, from.length(), to );
        pos += to.length();
    }
    return str;
}
}  // namespace galotfa

#ifdef debug_string
namespace unit_test {
int test_trim( void )
{
    println( "Testing galotfa::string::trim(std::string str, std::string blank) ..." );

    char str1[] = "  \t  \n  \r  \f  \v  OK";
    char str2[] = "  \t  \n  \r  \f  \v  # test the comment";
    char str3[] = "# This is a comment \n\n";
    char str4[] = "  \t  \n  \r  \f  \vstr before comment# asdqw";
    char str5[] = " enclosed by space ";
    char str6[] = "1:::-+2;3-4";

    bool res1 = galotfa::string::trim( str1 ) == "OK";
    bool res2 = galotfa::string::trim( str2 ) == "# test the comment";
    bool res3 = galotfa::string::trim( str3 ) == "# This is a comment";
    bool res4 = galotfa::string::trim( str4 ) == "str before comment# asdqw";
    bool res5 = galotfa::string::trim( str5 ) == "enclosed by space";
    bool res6 = galotfa::string::trim( str6 ) == "1:::-+2;3-4";

    bool success = res1 && res2 && res3 && res4 && res5 && res6;

    CHECK_RETURN( success );
}

int test_split( void )
{
    println( "Testing galotfa::string::split(std::string str, std::string blank) ..." );

    std::string str1 = " 1,2,3 4";
    std::string str2 = "1 2\t 3 \t\t4";
    std::string str3 = "1+2:3:4";
    std::string str4 = "1:::-+2\t3-4";
    std::string str5 = "1.012+string+3&4";

    std::vector< std::string > target1 = { "1", "2", "3", "4" };
    std::vector< std::string > target2 = { "1.012", "string", "3", "4" };
    std::vector< std::string > target3 = { "1.012+string+3&4" };


    auto res1 = galotfa::string::split( str1, " \t,-:+&" );
    auto res2 = galotfa::string::split( str2, " \t,-:+&" );
    auto res3 = galotfa::string::split( str3, " \t,-:+&" );
    auto res4 = galotfa::string::split( str4, " \t,-:+&" );
    auto res5 = galotfa::string::split( str5, " \t,-:+&" );
    auto res6 = galotfa::string::split( str5, "OKOK" );

    bool success = ( res1 == target1 ) && ( res2 == target1 ) && ( res3 == target1 )
                   && ( res4 == target1 ) && ( res5 == target2 ) && ( res6 == target3 );
    CHECK_RETURN( success );
}

#include <cassert>
int test_replace( void )
{
    println(
        "Testing galotfa::string::replace(std::string str, std::string from, std::string to) ..." );

    // test replace 1 to 1
    std::string str1    = "1 2 3 4";
    std::string str2    = "_2_3_4_";
    std::string target1 = "1_2_3_4";
    std::string target2 = " 2 3 4 ";
    bool        res1    = galotfa::string::replace( str1, " ", "_" ) == target1;
    bool        res2    = galotfa::string::replace( str2, "_", " " ) == target2;

    // test replace 1 to many
    std::string str3    = "1 2 3 4";
    std::string str4    = "_2_3_4_";
    std::string target3 = "1OK2OK3OK4";
    std::string target4 = "OK2OK3OK4OK";
    bool        res3    = galotfa::string::replace( str3, " ", "OK" ) == target3;
    bool        res4    = galotfa::string::replace( str4, "_", "OK" ) == target4;

    // test replace n to m
    std::string str5    = "Test a test for unit test";
    std::string target5 = "T awesome really awesome! a t awesome really awesome! for unit t "
                          "awesome really awesome!";
    bool res5 = galotfa::string::replace( str5, "est", " awesome really awesome!" ) == target5;

    // test replace from empty string
    println( "Test replace empty string, it should return original string and print warning ... " );
    std::string str6    = "Test a test for unit test";
    std::string target6 = "Test a test for unit test";
    bool        res6    = galotfa::string::replace( str6, "", " awesome really awes" ) == target6;

    // test replace to special character
    std::string str7    = "Test a test for unit test";
    std::string target7 = "T\t a t\t for unit t\t";
    bool        res7    = galotfa::string::replace( str7, "est", "\t" ) == target7;

    bool success = res1 && res2 && res3 && res4 && res5 && res6 && res7;
    CHECK_RETURN( success );
}
}  // namespace unit_test
#endif
#endif
