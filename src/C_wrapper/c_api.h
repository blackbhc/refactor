#ifndef __GALOTFA_C_API_H__
#define __GALOTFA_C_API_H__
#ifdef define_a_print
#include <stdio.h>
#endif

#include "../parameter/ini-parser.h"
#include "../tools/prompt.h"
void mpi_init_if_need( void );
void mpi_final_if_need( void );

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
#endif
