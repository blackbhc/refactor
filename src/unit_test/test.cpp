#include <variant>
#define DO_UNIT_TEST 1
#ifdef MPI_TEST
#include <mpi.h>
#endif
#include "../tools/prompt.h"
#include <vector>

static void operator+=( std::vector< int >& vec1, std::vector< int > vec2 )
{
    if ( vec1.size() != vec2.size() )
        ERROR( "The size of two vectors to add are not equal. %ld != %ld", vec1.size(),
               vec2.size() );
    for ( size_t i = 0; i < vec1.size(); ++i )
        vec1[ i ] += vec2[ i ];
}

// include the unit test header files
#ifdef debug_parameter
#include "test_para.cpp"
#endif
#ifdef debug_output
#include "test_output.cpp"
#endif
#ifdef debug_pre
#include "test_pre.cpp"
#endif
#ifdef debug_model
#include "test_model.cpp"
#endif
#ifdef debug_particle
#include "test_particle.cpp"
#endif
#ifdef debug_orbit
#include "test_orbit.cpp"
#endif
#ifdef debug_group
#include "test_group.cpp"
#endif
#ifdef debug_post
#include "test_post.cpp"
#endif
#ifdef debug_prompt
#include "test_prompt.cpp"
#endif
#ifdef debug_string
#include "test_string.cpp"
#endif
#ifdef debug_utils
#include "test_utils.cpp"
#endif

#ifdef MPI_TEST
int main( int argc, char* argv[] )
{
    int rank, size;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    println( "--------------------------------------------------------------------" );
    println( "\t\tRunning test on %d processors ...", size );
#else
int main()
{
#endif
    std::vector< int > result = { 0, 0, 0 };  // the result of the unit test
    println( "--------------------------------------------------------------------" );
    println( "Start the unit tests." );

    // HACK: run the unit test function at here
    println( "Running some unit tests ..." );
    println( "--------------------------------------------------------------------" );
    try
    {
#ifdef debug_string
        result += test_string();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_prompt
        result += test_prompt();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_parameter
        result += test_parameter();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_output
        result += test_output();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_pre
        result += test_pre();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_model
        result += test_model();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_particle
        result += test_particle();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_orbit
        result += test_orbit();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_group
        result += test_group();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_post
        result += test_post();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_utils
        result += test_utils();
        println( "--------------------------------------------------------------------" );
#endif
    }
    catch ( const std::exception& e )
    {
        ERROR( "Encounter an unexpected error during the unit test, error message: %s", e.what() );
    }

    // print the summary of the unit test
    int success = result[ 0 ];
    int fail    = result[ 1 ];
    int unknown = result[ 2 ];
    println( "\033[0mThe test results of \033[5;34m%s\033[0m is:\033[0;32m %d success, "
             "\033[0;31m%d fail, "
             "\033[0;33m%d unknown.\033[0m",
             "all unit tests", success, fail, unknown );
    if ( fail + unknown == 0 )
    {
        println( "\033[5;34m%s\033[0m are passed!\033[0m", "All tests" );
    }
#ifdef MPI_TEST
    MPI_Finalize();
#endif
    return 0;
}
