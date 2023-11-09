// Call the unit test functions for pre-processing part.
#ifndef PRE_TEST
#define PRE_TEST
// include the head file of the pre-processing part.
#include "../analysis/pre.cpp"
#include "../analysis/pre.h"
#include "../tools/prompt.h"
#include <stdio.h>
#include <vector>

static std::vector< int > test_pre()
{
    println( "Test the pre-processing part." );
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    COUNT( unit_test::test_center_of_mass() );
    COUNT( unit_test::test_most_dense_pixel() );
    SUMMARY( "pre-processing" );

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}

#endif
