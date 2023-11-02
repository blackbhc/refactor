// Call the unit test functions for output part.
#ifndef OUTPUT_TEST
#define OUTPUT_TEST
#include "../output/writer.cpp"
#include "../output/writer.h"
#include "../tools/prompt.h"
#include <vector>

// include the head file of the output part.
static std::vector< int > test_output( void )
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

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}
#endif
