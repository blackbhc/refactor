#include "c_api.h"
#include <mpi.h>

void mpi_init_if_need( void )
{
    int flag;
    MPI_Initialized( &flag );
    if ( !flag )
        MPI_Init( NULL, NULL );
}

void mpi_final_if_need( void )
{
    int flag;
    MPI_Initialized( &flag );
    if ( !flag )
        MPI_Finalize();
}
