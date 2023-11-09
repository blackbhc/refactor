// Call the unit test functions for pre-processing part.
#ifndef UTILS_TEST
#define UTILS_TEST
// include the head file of the pre-processing part.
#include "../analysis/utils.cpp"
#include "../analysis/utils.h"
#include "../tools/prompt.h"
#include <stdio.h>
#include <vector>

static std::vector< int > test_utils()
{
    println( "Test the analysis utils part.\n" );
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    COUNT( unit_test::test_in_spheroid() );
    COUNT( unit_test::test_in_box() );
    COUNT( unit_test::test_in_cylinder() );
    COUNT( unit_test::test_vec() );
    COUNT( unit_test::test_mat() );
    COUNT( unit_test::test_bin1d() );
    COUNT( unit_test::test_bin2d() );
    SUMMARY( "analysis utils" );

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}

#endif
