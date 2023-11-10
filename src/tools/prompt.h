#ifndef GALOTFA_PROMPT_H
#define GALOTFA_PROMPT_H
#include <stdexcept>
#include <stdio.h>

// for normal build mode, always use the mpi vertion prompts
#ifndef DO_UNIT_TEST
#include <mpi.h>
#define MPI_INCLUDED_LOCAL 1
#endif

// print a message for warning
#ifndef MPI_INCLUDED_LOCAL
// a single processor version printf
#define println( ... )         \
    {                          \
        printf( __VA_ARGS__ ); \
        printf( "\n" );        \
    }
#define fprintln( file_ptr, ... )         \
    {                                     \
        fprintf( file_ptr, __VA_ARGS__ ); \
        fprintf( file_ptr, "\n" );        \
    }
#else
#define println( ... )                           \
    {                                            \
        int _rank;                               \
        MPI_Comm_rank( MPI_COMM_WORLD, &_rank ); \
        if ( _rank == 0 )                        \
        {                                        \
            printf( __VA_ARGS__ );               \
            printf( "\n" );                      \
        }                                        \
    }
#define fprintln( file_ptr, ... )                \
    {                                            \
        int _rank;                               \
        MPI_Comm_rank( MPI_COMM_WORLD, &_rank ); \
        if ( _rank == 0 )                        \
        {                                        \
            fprintf( file_ptr, __VA_ARGS__ );    \
            fprintf( file_ptr, "\n" );           \
        }                                        \
    }
#endif

// print a message for warning
#define WARN( ... ) fprintln( stderr, "\033[0;1;33m[WARNING]: \033[0m" __VA_ARGS__ )

#define INFO( ... ) println( "\033[0;1;32m[INFO]: \033[0m" __VA_ARGS__ )

#define REACH INFO( "Reach %s %d line", __FILE__, __LINE__ )

// throw an error with a message
// TODO: support more error type
#define ERROR( ... )                                                         \
    {                                                                        \
        fprintln( stderr, "\033[0;1;31m[ERROR]: \033[0m" __VA_ARGS__ );      \
        throw std::runtime_error( "Invalid file or parameter during run." ); \
    }

#ifdef DO_UNIT_TEST
// macro for check the rusult of unit test, to make the code more compact
#define CHECK_RETURN( status_flag )                                                             \
    {                                                                                           \
        if ( !( status_flag ) )                                                                 \
        {                                                                                       \
            WARN( "The test failed at %d line in file %s, of function %s.", __LINE__, __FILE__, \
                  __func__ );                                                                   \
            return 1;                                                                           \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            println( "Pass the test." );                                                        \
            return 0;                                                                           \
        }                                                                                       \
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
