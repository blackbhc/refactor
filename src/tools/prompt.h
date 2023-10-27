#ifndef __GALOTFA_PROMPT_H__
#define __GALOTFA_PROMPT_H__
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
#define fprintln( ... )                 \
    {                                   \
        fprintf( stderr, __VA_ARGS__ ); \
        fprintf( stderr, "\n" );        \
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
#define fprintln( ... )                         \
    {                                           \
        int rank;                               \
        MPI_Comm_rank( MPI_COMM_WORLD, &rank ); \
        if ( rank == 0 )                        \
        {                                       \
            fprintf( stderr, __VA_ARGS__ );     \
            fprintf( stderr, "\n" );            \
        }                                       \
    }
#endif

// print a message for warning
#define WARN( ... )                                                \
    {                                                              \
        fprintln( "\033[0;1;33m [WARNING]: \033[0m" __VA_ARGS__ ); \
    }

// throw a error with a message
#define ERROR( ... )                                                         \
    {                                                                        \
        fprintln( "\033[0;1;31m [ERROR]: \033[0m" __VA_ARGS__ );             \
        throw std::runtime_error( "Invalid file or parameter during run." ); \
    }

// macro for check the result of unit test, to make the code more compact
#define CHECK_RETURN( status_flag )                                              \
    {                                                                            \
        if ( !( status_flag ) )                                                  \
        {                                                                        \
            WARN( "The test failed at %d line in file %s", __LINE__, __FILE__ ); \
            return 1;                                                            \
        }                                                                        \
        else                                                                     \
        {                                                                        \
            println( "The test passed." );                                       \
            return 0;                                                            \
        }                                                                        \
    }
#endif
