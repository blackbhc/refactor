#ifndef PROMPT_TEST
#define PROMPT_TEST
#include "../tools/prompt.cpp"
#include "../tools/prompt.h"
#include <vector>

static std::vector< int > test_prompt()
{
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    COUNT( unit_test::warn_and_error() );
    SUMMARY( "prompt" );

    std::vector< int > result = { 0, 0, 0 };
    result[ 0 ]               = success;
    result[ 1 ]               = fail;
    result[ 2 ]               = unknown;
    return result;
}
#endif
