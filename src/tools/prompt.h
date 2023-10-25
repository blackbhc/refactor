#ifndef _PROTOS_PROMPT_H_
#define _PROTOS_PROMPT_H_
#include <stdexcept>
#include <stdio.h>

// print a message for warning
#ifndef MPI_INCLUDED
// a single processor version printf
#define println( ... )         \
    {                          \
        printf( __VA_ARGS__ ); \
        printf( "\n" );        \
    }
#else
#define println( ... )                          \
    {                                           \
        int rank;                               \
        MPI_Comm_rank( MPI_COMM_WORLD, &rank ); \
        if ( rank == 0 )                        \
        {                                       \
            printf( __VA_ARGS__ );              \
            printf( "\n" );                     \
        }                                       \
    }
#endif

// print a message for warning
#define WARN( ... )                                               \
    {                                                             \
        println( "\033[0;1;33m [WARNING]: \033[0m" __VA_ARGS__ ); \
    }

// throw a error with a message
#define ERROR( ... )                                                         \
    {                                                                        \
        println( "\033[0;1;31m [ERROR]: \033[0m" __VA_ARGS__ );              \
        throw std::runtime_error( "Invalid file or parameter during run." ); \
    }
#endif
