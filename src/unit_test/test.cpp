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

#ifdef MPI_TEST
int main( int argc, char* argv[] )
{
    int rank, size;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    test_info( "--------------------------------------------------------------------\n" );
    test_info( "\t\tRunning test on %d processors ...\n", size );
#else
int main()
{
#endif
    println( "--------------------------------------------------------------------" );
    println( "Start the unit tests." );

    // HACK: run the unit test function at here
    println( "Running some unit tests ..." );
    try
    {
#ifdef debug_parameter
        test_parameter();
#endif
#ifdef debug_output
        test_output();
#endif
#ifdef debug_pre
        test_pre();
#endif
#ifdef debug_model
        test_model();
#endif
#ifdef debug_particle
        test_particle();
#endif
#ifdef debug_orbit
        test_orbit();
#endif
#ifdef debug_group
        test_group();
#endif
#ifdef debug_post
        test_post();
#endif
#ifdef debug_prompt
        test_prompt();
#endif

        println( "--------------------------------------------------------------------" );
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
