// Call the unit test functions for output part.
#ifndef OUTPUT_TEST
#define OUTPUT_TEST
#include "../output/writer.cpp"
#include "../output/writer.h"
#include "../tools/prompt.h"


// include the head file of the output part.
static void test_output( void )
{
    println( "Testing the output part." );
    int             success = 0;
    int             fail    = 0;
    int             unknown = 0;
    galotfa::writer writer( "test.hdf5" );

    // call the unit test functions for the output part.
    COUNT( writer.test_node() );
    COUNT( writer.test_open_file() );
    COUNT( writer.test_create_close() );
    COUNT( writer.test_create_group() );
    COUNT( writer.test_create_dataset() );
    COUNT( writer.test_push() );
    SUMMARY( "output" );
}
#endif
