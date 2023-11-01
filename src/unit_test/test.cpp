#define DO_UNIT_TEST 1
#ifdef MPI_TEST
#include <mpi.h>
#endif
#include "../tools/prompt.h"
#include <stdio.h>


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
    println( "--------------------------------------------------------------------" );
    println( "Start the unit tests." );

    // HACK: run the unit test function at here
    println( "Running some unit tests ..." );
    println( "--------------------------------------------------------------------" );
    try
    {
#ifdef debug_string
        test_string();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_prompt
        test_prompt();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_parameter
        test_parameter();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_output
        test_output();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_pre
        test_pre();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_model
        test_model();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_particle
        test_particle();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_orbit
        test_orbit();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_group
        test_group();
        println( "--------------------------------------------------------------------" );
#endif
#ifdef debug_post
        test_post();
        println( "--------------------------------------------------------------------" );
#endif

#ifdef MPI_TEST
        MPI_Finalize();
#endif
    }
    catch ( const std::exception& e )
    {
        println( "Encounter an error during the unit test, error message: %s", e.what() );
#ifdef MPI_TEST
        MPI_Finalize();
#endif
    }

    return 0;
}
