// Call the unit test functions for output part.
#ifndef _OUTPUT_TEST_
#define _OUTPUT_TEST_
#include "../output/writer.cpp"
#include "../output/writer.h"
#include "../tools/prompt.h"


// include the head file of the output part.
void test_output( void )
{
    println( "Testing the output part.\n" );
    int             success = 0;
    int             fail    = 0;
    int             unknown = 0;
    galotfa::writer writer( "test.h5" );

    // call the unit test functions for the output part.
    COUNT( writer.test_create_file() );
    SUMMARY( "output" );
}
#endif
