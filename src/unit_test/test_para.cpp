// Call the unit test functions for the parameter part.
#ifndef PARA_TEST
#define PARA_TEST
// include the head file of the parameter part.
#include "../parameter/ini-parser.cpp"
#include "../parameter/ini-parser.h"
#include "../parameter/para.cpp"
#include "../parameter/para.h"
#include "../tools/prompt.h"
#include <stdio.h>

static void test_parameter( void )
{
    println( "Test the parameter part." );

    // call the unit test functions for the parameter part.
    int                 success = 0;
    int                 fail    = 0;
    int                 unknown = 0;
    galotfa::ini_parser ini( "./galotfa.ini" );
    galotfa::para       para( ini );

    COUNT( ini.test_checksize() );
    COUNT( ini.test_trim() );
    COUNT( ini.test_split() );
    COUNT( ini.test_lineparser() );
    COUNT( ini.test_read() );
    COUNT( ini.test_get() );
    COUNT( para.test_print() );

    SUMMARY( "parameter" );
}

#endif
