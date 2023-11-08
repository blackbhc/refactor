#ifndef GALOTFA_PRE_CPP
#define GALOTFA_PRE_CPP
#include "pre.h"
#include <mpi.h>
#include <string.h>
namespace ana = galotfa::analysis;
int ana::center_of_mass( unsigned long part_num, double masses[], double coords[][ 3 ],
                         double ( &center )[ 3 ] )
{
    // calculate the center of mass of the given array of particles
    double mass_sum = 0;
    memset( center, 0, sizeof( double ) * 3 );
    for ( unsigned long i = 0; i < part_num; ++i )
    {
        mass_sum += masses[ i ];
        // summation only for the numerator
        center[ 0 ] += masses[ i ] * coords[ i ][ 0 ];
        center[ 1 ] += masses[ i ] * coords[ i ][ 1 ];
        center[ 2 ] += masses[ i ] * coords[ i ][ 2 ];
    }
    MPI_Allreduce( MPI_IN_PLACE, center, 3, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, &mass_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

    // divide by the denominator
    if ( mass_sum > 0 )  // if there are particles
    {
        center[ 0 ] /= mass_sum;
        center[ 1 ] /= mass_sum;
        center[ 2 ] /= mass_sum;
    }
    return 0;
}  // namespace galotfa::analysis


#ifdef debug_pre
#include "../tools/prompt.h"
#include <math.h>
namespace unit_test {
int test_center_of_mass()
{
    int rank, size;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    const int part_num                = 1000;
    double    masses[ part_num ]      = { 0 };
    double    coords[ part_num ][ 3 ] = { { 0 } };
    double    center[ 3 ]             = { 0 };
    double    eps                     = 1e-10;  // the numerical error


    // a naive test: com = (0,0,0)
    if ( rank == 0 )
    {
        for ( int i = 0; i < part_num; ++i )
        {
            masses[ i ]      = 1.0;
            coords[ i ][ 0 ] = pow( -1, i );
            coords[ i ][ 1 ] = pow( -1, i + 1 );
            coords[ i ][ 2 ] = pow( -1, i + 2 );
        }
    }
    ana::center_of_mass( part_num, masses, coords, center );

    if ( rank == 0 )
    {
        if ( fabs( center[ 0 ] ) > eps )
            CHECK_RETURN( false );
        if ( fabs( center[ 1 ] ) > eps )
            CHECK_RETURN( false );
        if ( fabs( center[ 2 ] ) > eps )
            CHECK_RETURN( false );
    }

    // a naive test: com = (1,1,1)
    if ( rank == 0 )
    {
        for ( int i = 0; i < part_num; ++i )
        {
            masses[ i ]      = 1.0;
            coords[ i ][ 0 ] = ( double )( i / 100 );
            coords[ i ][ 1 ] = ( double )( ( i - ( i / 100 ) * 100 ) / 10 );
            coords[ i ][ 2 ] = ( double )( i % 10 );
        }
    }
    ana::center_of_mass( part_num, masses, coords, center );
    if ( rank == 0 )
    {
        if ( fabs( center[ 0 ] - 4.5 ) > eps )
            CHECK_RETURN( false );
        if ( fabs( center[ 1 ] - 4.5 ) > eps )
            CHECK_RETURN( false );
        if ( fabs( center[ 2 ] - 4.5 ) > eps )
            CHECK_RETURN( false );
    }

    CHECK_RETURN( true );
}
}  // namespace unit_test
#endif
#endif
