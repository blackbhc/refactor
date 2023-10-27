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

#ifdef DO_UNIT_TEST
// macro for check the rusult of unit test, to make the code more compact
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

// count the result of unit test
// based on three status flag: success, fail and unknown
#define COUNT( x )              \
    {                           \
        int status = x;         \
        if ( status == 0 )      \
        {                       \
            success++;          \
        }                       \
        else if ( status == 1 ) \
        {                       \
            fail++;             \
        }                       \
        else                    \
        {                       \
            unknown++;          \
        }                       \
    }

// macro function to summary the result of unit test
// based on three status flag: success, fail and unknown
#define SUMMARY( module_name )                                                              \
    {                                                                                       \
        println( "\033[0mThe test results of \033[5;34m%s\033[0m part is:\033[0;32m %d "    \
                 "success, "                                                                \
                 "\033[0;31m%d fail, "                                                      \
                 "\033[0;33m%d unknown.\033[0m",                                            \
                 module_name, success, fail, unknown );                                     \
        if ( fail + unknown == 0 )                                                          \
        {                                                                                   \
            println( "All tests of \033[5;34m%s\033[0m part passed!\033[0m", module_name ); \
        }                                                                                   \
    }
#endif
#endif
