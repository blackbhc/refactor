// Call the unit test functions for particle analysis part.
#ifndef PARTICLE_TEST
#define PARTICLE_TEST
#include "../analysis/particle.cpp"
#include "../analysis/particle.h"
#include "../tools/prompt.h"
#include <stdio.h>

static std::vector< int > test_particle()
{
    println( "Test the particle analysis part.\n" );
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    COUNT( unit_test::test_angular_momentum() );
    COUNT( unit_test::test_circularity() );
    SUMMARY( "particle analysis" );

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}
#endif
