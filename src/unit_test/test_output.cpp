// Call the unit test functions for output part.
#ifndef _OUTPUT_TEST_
#define _OUTPUT_TEST_
#include "../output/writer.h"
#include "../tools/prompt.h"
#include <stdio.h>


// include the head file of the output part.
void test_output( void )
{
    println( "Testing the output part.\n" );
    int success = 0;
    int fail    = 0;
    int unknown = 0;

    // call the unit test functions for the output part.
    COUNT( unit_test::test_createh5() );
    SUMMARY();
    return;
}
#endif
