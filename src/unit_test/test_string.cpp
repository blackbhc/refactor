// Call the unit test functions for output part.
#ifndef _OUTPUT_TEST_
#define _OUTPUT_TEST_
#include "../tools/prompt.h"
#include "../tools/string.cpp"
#include "../tools/string.h"


// include the head file of the output part.
void test_string( void )
{
    println( "Testing the string part ..." );
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    // call the unit test functions for the output part.
    COUNT( unit_test::test_trim() );
    COUNT( unit_test::test_split() );
    SUMMARY( "string" );
}
#endif
