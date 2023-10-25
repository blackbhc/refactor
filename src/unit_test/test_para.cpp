// Call the unit test functions for the parameter part.
#ifndef _PARA_TEST_
#define _PARA_TEST_
#include "../parameter/ini-parser.cpp"
#include "../parameter/ini-parser.h"
#include "test_info.hpp"
#include <stdio.h>

// include the head file of the parameter part.
void test_parameter( void )
{
    println( "Test the parameter part." );

    // call the unit test functions for the parameter part.
    int                 success = 0;
    int                 fail    = 0;
    int                 unknown = 0;
    galotfa::ini_parser ini( "./galotfa.ini" );
    int                 status = ini.test_check_size_works();

    if ( status == 0 )
    {
        success++;
    }
    else if ( status == 1 )
    {
        fail++;
    }
    else
    {
        unknown++;
    }

    println( "\033[0mThe test results of \033[4;34mparameter\033[0m part is:\033[0;32m %d success, "
             "\033[0;31m%d fail, "
             "\033[0;33m%d unknown.\033[0m",
             success, fail, unknown );
    if ( fail + unknown == 0 )
    {
        println( "All tests of \033[4;34mparameter\033[0m part passed!\033[0m" );
    }
}
#endif
