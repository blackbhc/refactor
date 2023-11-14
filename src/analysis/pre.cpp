#ifndef GALOTFA_PRE_CPP
#define GALOTFA_PRE_CPP
#include "pre.h"
#include "../analysis/utils.h"
#ifdef debug_pre
#include "../analysis/utils.cpp"
#endif
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

int ana::most_dense_pixel( unsigned long part_num, double coords[][ 3 ], double lower_bound_x,
                           double upper_bound_x, double lower_bound_y, double upper_bound_y,
                           double lower_bound_z, double upper_bound_z, unsigned int bin_num_x,
                           unsigned int bin_num_y, unsigned int bin_num_z, double ( &center )[ 3 ] )
{
    double* x = new double[ part_num ]();
    double* y = new double[ part_num ]();
    double* z = new double[ part_num ]();
    for ( unsigned long i = 0; i < part_num; ++i )
    {
        x[ i ] = coords[ i ][ 0 ];
        y[ i ] = coords[ i ][ 1 ];
        z[ i ] = coords[ i ][ 2 ];
    }
    // calculate the most dense pixel of the given array of particles
    vector< vector< double > > image_xy =
        ana::bin2d( part_num, x, y, x, lower_bound_x, upper_bound_x, lower_bound_y, upper_bound_y,
                    bin_num_x, bin_num_y, ana::stats_method::count );

    vector< vector< double > > image_xz =
        ana::bin2d( part_num, x, z, x, lower_bound_x, upper_bound_x, lower_bound_z, upper_bound_z,
                    bin_num_x, bin_num_z, ana::stats_method::count );

    // MPI Reduce
    for ( size_t i = 0; i < bin_num_x; ++i )
    {
        MPI_Allreduce( MPI_IN_PLACE, image_xy[ i ].data(), ( int )bin_num_y, MPI_DOUBLE, MPI_SUM,
                       MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, image_xz[ i ].data(), ( int )bin_num_z, MPI_DOUBLE, MPI_SUM,
                       MPI_COMM_WORLD );
    }

    // find the max pixel's position
    size_t max_x = 0, max_y = 0, max_z = 0;
    for ( size_t i = 0; i < ( size_t )bin_num_x; ++i )
    {
        for ( size_t j = 0; j < ( size_t )bin_num_y; ++j )
        {
            if ( image_xy[ i ][ j ] > image_xy[ max_x ][ max_y ] )
            {
                max_x = i;
                max_y = j;
            }
        }
    }

    for ( size_t i = 0; i < ( size_t )bin_num_z; ++i )
    {
        if ( image_xz[ max_x ][ i ] > image_xz[ max_x ][ max_z ] )
        {
            max_z = i;
        }
    }
    double pos_x = ( double )max_x + 0.5, pos_y = ( double )max_y + 0.5,
           pos_z = ( double )max_z + 0.5;
    center[ 0 ]  = pos_x / ( double )bin_num_x * ( upper_bound_x - lower_bound_x ) + lower_bound_x;
    center[ 1 ]  = pos_y / ( double )bin_num_y * ( upper_bound_y - lower_bound_y ) + lower_bound_y;
    center[ 2 ]  = pos_z / ( double )bin_num_z * ( upper_bound_z - lower_bound_z ) + lower_bound_z;

    // release memory
    delete[] x;
    delete[] y;
    delete[] z;
    return 0;
}


#ifdef debug_pre
#include "../tools/prompt.h"
#include <math.h>
namespace unit_test {
int test_center_of_mass()
{
    println( "Testing the center of mass function ..." );
    int rank, size;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    const int part_num                = 1000;
    double    masses[ part_num ]      = { 0 };
    double    coords[ part_num ][ 3 ] = { { 0 } };
    double    center[ 3 ]             = { 0 };
    double    eps                     = 1e-10;  // the numerical error


    // a naive test: com = (0,0,0)
    for ( int i = 0; i < part_num; ++i )
    {
        masses[ i ]      = 1.0;
        coords[ i ][ 0 ] = pow( -1, i );
        coords[ i ][ 1 ] = pow( -1, i + 1 );
        coords[ i ][ 2 ] = pow( -1, i + 2 );
    }

    ana::center_of_mass( part_num, masses, coords, center );

    if ( fabs( center[ 0 ] ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 1 ] ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 2 ] ) > eps )
        CHECK_RETURN( false );

    // a naive test: com = (1,1,1)
    for ( int i = 0; i < part_num; ++i )
    {
        masses[ i ]      = 1.0;
        coords[ i ][ 0 ] = ( double )( i / 100 );
        coords[ i ][ 1 ] = ( double )( ( i - ( i / 100 ) * 100 ) / 10 );
        coords[ i ][ 2 ] = ( double )( i % 10 );
    }

    ana::center_of_mass( part_num, masses, coords, center );
    if ( fabs( center[ 0 ] - 4.5 ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 1 ] - 4.5 ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 2 ] - 4.5 ) > eps )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}

int test_most_dense_pixel()
{
    println( "Testing the most dense pixel function ..." );
    int rank, size;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    const int part_num                = 1000;
    double    coords[ part_num ][ 3 ] = { { 0 } };
    double    center[ 3 ]             = { 0 };
    double    eps                     = 0.5;  // the numerical error, the most dense pixel is not
                                              // so accurate, so we set a larger error

    // a naive test: com = (-1, 1, -1)
    for ( int i = 0; i < part_num; ++i )
    {
        coords[ i ][ 0 ] = pow( -1, i );
        coords[ i ][ 1 ] = pow( -1, i + 1 );
        coords[ i ][ 2 ] = pow( -1, i + 2 );
    }
    ana::most_dense_pixel( part_num, coords, -1, 1, -1, 1, -1, 1, 9, 9, 9, center );


    if ( fabs( center[ 0 ] - -1 ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 1 ] - 1 ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 2 ] - -1 ) > eps )
        CHECK_RETURN( false );

    // a naive test: com = (0, 0, 0)
    for ( int i = 0; i < part_num; ++i )
    {
        coords[ i ][ 0 ] = ( double )( i / 100 );
        coords[ i ][ 1 ] = ( double )( ( i - ( i / 100 ) * 100 ) / 10 );
        coords[ i ][ 2 ] = ( double )( i % 10 );
    }

    ana::most_dense_pixel( part_num, coords, -1, 1, -1, 1, -1, 1, 9, 9, 9, center );
    if ( fabs( center[ 0 ] ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 1 ] ) > eps )
        CHECK_RETURN( false );
    if ( fabs( center[ 2 ] ) > eps )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}
}  // namespace unit_test
#endif
#endif
