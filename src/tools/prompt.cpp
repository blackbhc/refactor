#ifndef GALOTFA_PROMPT_CPP
#define GALOTFA_PROMPT_CPP
#include "prompt.h"
#include <stdexcept>  // std::runtime_error
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>  // struct stat

#ifdef debug_prompt
namespace unit_test {
// the unit test for prompts
static int warn_and_error()
{
    println( " Test the prompt functions." );
    println( "It should print a warning message and an error message, don't worry." );
    WARN( "This is a warning message" );
    try
    {
        ERROR( "This is an error message" );
    }
    catch ( std::runtime_error& e )
    {
        println( "It failed as expected: %s.", e.what() );
    }
    return 0;
}
};  // namespace unit_test
#endif
#endif
