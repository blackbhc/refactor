#ifndef __GALOTFA_STRING_CPP__
#define __GALOTFA_STRING_CPP__
#include "./string.h"
#include <string>
#ifdef debug_string
int unit_test::test_trim( void )
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

int unit_test::test_split( void )
{
    println( "Testing galotfa::string::split(std::string str, std::string blank) ..." );

    std::string str1 = " 1,2,3 4";
    std::string str2 = "1 2\t 3 \t\t4";
    std::string str3 = "1+2:3:4";
    std::string str4 = "1:::-+2\t3-4";
    std::string str5 = "1.012+string+3&4";

    std::vector< std::string > target1 = { "1", "2", "3", "4" };
    std::vector< std::string > target2 = { "1.012", "string", "3", "4" };

    auto res1 = galotfa::string::split( str1, " \t,-:+&" );
    auto res2 = galotfa::string::split( str2, " \t,-:+&" );
    auto res3 = galotfa::string::split( str3, " \t,-:+&" );
    auto res4 = galotfa::string::split( str4, " \t,-:+&" );
    auto res5 = galotfa::string::split( str5, " \t,-:+&" );

    bool success = ( res1 == target1 ) && ( res2 == target1 ) && ( res3 == target1 )
                   && ( res4 == target1 ) && ( res5 == target2 );
    CHECK_RETURN( success );
}
#endif
#endif
