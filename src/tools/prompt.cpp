#ifndef __GALOTFA_PROMPT_CPP__
#define __GALOTFA_PROMPT_CPP__
#include "prompt.h"
#include <stdexcept>  // std::runtime_error
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>  // struct stat

#ifdef debug_prompt
namespace unit_test {
// the unit test for prompts
int warn_and_error()
{
    println( " Test the prompt functions." );
    println( "It should print a warning message and a error message, don't worry." );
    WARN( "This is a warning message" );
    try
    {
        ERROR( "This is a error message" );
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
