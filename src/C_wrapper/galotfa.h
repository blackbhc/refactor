#ifdef define_a_print
#include <stdio.h>
#endif

#include "../parameter/ini-parser.h"
#include "../tools/prompt.h"

extern "C" {
#ifdef define_a_print
void print()  // define a hello world function for test
{
    println( "Hello World!\n" );
};
#endif
}

// include the implementation files if in header-only type
#ifdef header_only
#include "../parameter/ini-parser.cpp"
#endif
