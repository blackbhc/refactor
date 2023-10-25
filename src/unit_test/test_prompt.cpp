#include "../tools/prompt.h"
#include "../tools/prompt.cpp"
#include "test_info.hpp"

void test_prompt()
{
    int success = 0;
    int fail    = 0;
    int unknown = 0;
    int status  = unit_test::warn_and_error();
    if ( status == 0 )
    {
        success++;
    }
    else if ( status == 1 )
    {
        fail++;
    }
    else
    {
        unknown++;
    }

    println( "\033[0mThe test results of \033[4;34mprompt\033[0m part is:\033[0;32m %d success, "
             "\033[0;31m%d fail, "
             "\033[0;33m%d unknown.\033[0m",
             success, fail, unknown );
    if ( fail + unknown == 0 )
    {
        println( "All tests of \033[4;34mprompt\033[0m part passed!\033[0m" );
    }
}
