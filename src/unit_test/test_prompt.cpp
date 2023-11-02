#include "../tools/prompt.cpp"
#include "../tools/prompt.h"

static void test_prompt()
{
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    COUNT( unit_test::warn_and_error() );
    SUMMARY( "prompt" );
}
