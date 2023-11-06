// Call the unit test functions for output part.
#ifndef STRING_TEST
#define STRING_TEST
#include "../tools/prompt.h"
#include "../tools/string.cpp"
#include "../tools/string.h"
#include <vector>

// include the head file of the output part.
static std::vector< int > test_string( void )
{
    println( "Testing the string part ..." );
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    // call the unit test functions for the output part.
    COUNT( unit_test::test_trim() );
    COUNT( unit_test::test_split() );
    COUNT( unit_test::test_replace() );
    SUMMARY( "string" );

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}
#endif
