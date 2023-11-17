#ifndef GALOTFA_MODEL_CPP
#define GALOTFA_MODEL_CPP
#include "model.h"
#include "utils.h"
#include <complex>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <vector>

using std::complex;
namespace ana = galotfa::analysis;

complex< double > ana::An( int array_len, double mass[], double x[], double y[],
                           unsigned int order )
{
    complex< double > result = 0;
    complex< double > I( 0, 1 );  // the imaginary unit (0+1i)
    double            phi = 0;    // the azimuthal angle

    for ( int i = 0; i < array_len; ++i )
    {
        phi = atan2( y[ i ], x[ i ] );
        result += mass[ i ] * exp( order * phi * I );
    }

    // MPI reduction
    MPI_Allreduce( MPI_IN_PLACE, &result, 1, MPI_DOUBLE_COMPLEX, MPI_SUM, MPI_COMM_WORLD );
    return result;
}


double ana::s_bar( int array_len, double mass[], double x[], double y[] )
{
    auto A2   = An( array_len, mass, x, y, 2 );
    auto A0   = An( array_len, mass, x, y, 0 );
    auto Abar = A2 / A0;
    return abs( Abar );
}

double ana::s_buckle( int array_len, double mass[], double x[], double y[], double z[] )
{
    complex< double > numerator   = 0;
    double            denominator = 0;
    complex< double > I( 0, 1 );  // the imaginary unit (0+1i)
    double            phi = 0;    // the azimuthal angle

    for ( int i = 0; i < array_len; ++i )
    {
        phi = atan2( y[ i ], x[ i ] );
        numerator += z[ i ] * mass[ i ] * exp( 2 * phi * I );
        denominator += mass[ i ];
    }

    // MPI reduction
    MPI_Allreduce( MPI_IN_PLACE, &numerator, 1, MPI_DOUBLE_COMPLEX, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, &denominator, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

    return abs( numerator / denominator );
}

double ana::bar_major_axis( int array_len, double mass[], double x[], double y[] )
{
    auto A2 = An( array_len, mass, x, y, 2 );
    return arg( A2 ) / 2;  // divide by 2, as the argument of A2 is 2*phi
}

double ana::bar_radius( int array_len, double mass[], double x[], double y[], double rmin,
                        double rmax, int rbins, double major_axis, double angle_threshold,
                        double percentage, double* results )
{
    // static variables
    static complex< double > I( 0, 1 );   // the imaginary unit (0+1i)
    static double            phi    = 0;  // the azimuthal angle
    static int               index  = 0;  // the array index of the bin
    static int               binnum = rbins;
    static double            min = rmin, max = rmax, range = max - min, bin_size = range / binnum;
    static double            angle = angle_threshold * M_PI / 180, percent = percentage / 100;

    double*            s_bar    = new double[ binnum ]();
    double*            phis     = new double[ binnum ]();
    complex< double >* result   = new complex< double >[ binnum ]();
    double*            mass_sum = new double[ binnum ]();
    double             r        = 0;

    for ( int i = 0; i < array_len; ++i )
    {
        r = sqrt( x[ i ] * x[ i ] + y[ i ] * y[ i ] );
        if ( r < min || r > max )
            continue;

        index = ( int )( r - min ) / bin_size;
        if ( index == binnum )
            --index;

        phi = atan2( y[ i ], x[ i ] );
        result[ index ] += mass[ i ] * exp( 2 * phi * I );
        mass_sum[ index ] += mass[ i ];
    }

    // MPI reduction
    MPI_Allreduce( MPI_IN_PLACE, result, binnum, MPI_DOUBLE_COMPLEX, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, mass_sum, binnum, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

    for ( int i = 0; i < binnum; ++i )
    {
        s_bar[ i ] = abs( result[ i ] / mass_sum[ i ] );
        phis[ i ]  = arg( result[ i ] ) / 2;  // divide by 2, as the argument of A2 is 2*phi
    }

    results[ 0 ] = 0;  // calculate Rbar1
    for ( int i = 0; i < binnum; ++i )
    {
        if ( fabs( phis[ i ] - major_axis ) >= angle )
        {
            results[ 0 ] = min + ( i + 0.5 ) * bin_size;
            break;
        }
    }
    results[ 1 ] = ( ( std::max_element( s_bar, s_bar + binnum ) - s_bar ) + 0.5 ) * bin_size
                   + min;     // calculate Rbar2
    results[ 2 ]        = 0;  // calculate Rbar3
    int    max_location = std::max_element( s_bar, s_bar + binnum ) - s_bar;
    double max_s_bar    = s_bar[ max_location ];
    for ( int i = max_location; i < binnum; ++i )
    {
        if ( s_bar[ i ] <= percent * max_s_bar )
        {
            results[ 2 ] = min + ( i + 0.5 ) * bin_size;
            break;
        }
    }

    // release the memory
    delete[] s_bar;
    delete[] phis;
    delete[] result;
    delete[] mass_sum;

    return 0;
}

int ana::dispersion_tensor( int array_len, double x[], double y[], double z[], double vx[],
                            double vy[], double vz[], double lower_bound_x, double upper_bound_x,
                            double lower_bound_y, double upper_bound_y, double lower_bound_z,
                            double upper_bound_z, unsigned int num_bins_x, unsigned int num_bins_y,
                            unsigned int num_bins_z, double* tensor )
{
    double bin_size_x = ( upper_bound_x - lower_bound_x ) / num_bins_x;
    double bin_size_y = ( upper_bound_y - lower_bound_y ) / num_bins_y;
    double bin_size_z = ( upper_bound_z - lower_bound_z ) / num_bins_z;

    // sigma_ij = < v_i v_j > - < v_i > < v_j >
    // the cross terms of the dispersion tensor
    double* v00 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v01 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v02 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v10 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v11 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v12 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v20 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v21 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v22 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    // the second order moments of the velocity
    double* v0 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v1 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    double* v2 = new double[ num_bins_x * num_bins_y * num_bins_z ]();
    // the first order moments of the velocity
    unsigned long* count = new unsigned long[ num_bins_x * num_bins_y * num_bins_z ]();

    // the factor used to represent the index of the bin
    unsigned int factor1 = num_bins_y * num_bins_z;
    unsigned int factor2 = num_bins_z;

    unsigned int i, j, k;  // tmp variables for the index of the bin

    unsigned int index_x, index_y, index_z;  // tmp variables for the index of the bin
    for ( i = 0; i < ( unsigned int )array_len; ++i )
    {
        if ( x[ i ] < lower_bound_x || x[ i ] > upper_bound_x || y[ i ] < lower_bound_y
             || y[ i ] > upper_bound_y || z[ i ] < lower_bound_z || z[ i ] > upper_bound_z )
        {
            continue;
        }

        // calculate the index of the bin
        index_x = ( unsigned int )( x[ i ] - lower_bound_x ) / bin_size_x;
        index_y = ( unsigned int )( y[ i ] - lower_bound_y ) / bin_size_y;
        index_z = ( unsigned int )( z[ i ] - lower_bound_z ) / bin_size_z;

        // avoid the index out of range at the upper boundary
        if ( index_x == num_bins_x )
            --index_x;
        if ( index_y == num_bins_y )
            --index_y;
        if ( index_z == num_bins_z )
            --index_z;

        v0[ index_x * factor1 + index_y * factor2 + index_z ] += vx[ i ];
        v1[ index_x * factor1 + index_y * factor2 + index_z ] += vy[ i ];
        v2[ index_x * factor1 + index_y * factor2 + index_z ] += vz[ i ];
        v00[ index_x * factor1 + index_y * factor2 + index_z ] += vx[ i ] * vx[ i ];
        v01[ index_x * factor1 + index_y * factor2 + index_z ] += vx[ i ] * vy[ i ];
        v02[ index_x * factor1 + index_y * factor2 + index_z ] += vx[ i ] * vz[ i ];
        v10[ index_x * factor1 + index_y * factor2 + index_z ] += vy[ i ] * vx[ i ];
        v11[ index_x * factor1 + index_y * factor2 + index_z ] += vy[ i ] * vy[ i ];
        v12[ index_x * factor1 + index_y * factor2 + index_z ] += vy[ i ] * vz[ i ];
        v20[ index_x * factor1 + index_y * factor2 + index_z ] += vz[ i ] * vx[ i ];
        v21[ index_x * factor1 + index_y * factor2 + index_z ] += vz[ i ] * vy[ i ];
        v22[ index_x * factor1 + index_y * factor2 + index_z ] += vz[ i ] * vz[ i ];
        count[ index_x * factor1 + index_y * factor2 + index_z ] += 1;
    }

    // MPI reduction
    MPI_Allreduce( MPI_IN_PLACE, v0, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v1, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v2, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v00, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v01, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v02, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v10, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v11, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v12, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v20, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v21, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v22, num_bins_x * num_bins_y * num_bins_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, count, num_bins_x * num_bins_y * num_bins_z, MPI_UNSIGNED_LONG,
                   MPI_SUM, MPI_COMM_WORLD );

    // the factor used to represent the index of the bin
    unsigned int factor3 = num_bins_y * num_bins_z * 9;
    unsigned int factor4 = num_bins_z * 9;

    for ( i = 0; i < num_bins_x; ++i )
        for ( j = 0; j < num_bins_y; ++j )
            for ( k = 0; k < num_bins_z; ++k )
            {
                tensor[ i * factor3 + j * factor4 + k * 9 + 0 * 3 + 0 ] =
                    v00[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v0[ i * factor1 + j * factor2 + k ] * v0[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 0 * 3 + 1 ] =
                    v01[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v0[ i * factor1 + j * factor2 + k ] * v1[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 0 * 3 + 2 ] =
                    v02[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v0[ i * factor1 + j * factor2 + k ] * v2[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 1 * 3 + 0 ] =
                    v10[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v1[ i * factor1 + j * factor2 + k ] * v0[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 1 * 3 + 1 ] =
                    v11[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v1[ i * factor1 + j * factor2 + k ] * v1[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 1 * 3 + 2 ] =
                    v12[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v1[ i * factor1 + j * factor2 + k ] * v2[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 2 * 3 + 0 ] =
                    v20[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v2[ i * factor1 + j * factor2 + k ] * v0[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 2 * 3 + 1 ] =
                    v21[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v2[ i * factor1 + j * factor2 + k ] * v1[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
                tensor[ i * factor3 + j * factor4 + k * 9 + 2 * 3 + 2 ] =
                    v22[ i * factor1 + j * factor2 + k ] / count[ i * factor1 + j * factor2 + k ]
                    - v2[ i * factor1 + j * factor2 + k ] * v2[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ]
                          / count[ i * factor1 + j * factor2 + k ];
            }

    // release the memory
    delete[] v00;
    delete[] v01;
    delete[] v02;
    delete[] v10;
    delete[] v11;
    delete[] v12;
    delete[] v20;
    delete[] v21;
    delete[] v22;
    delete[] v0;
    delete[] v1;
    delete[] v2;
    delete[] count;

    return 0;
}

int ana::inertia_tensor( int array_len, double mass[], double x[], double y[], double z[],
                         double* tensor )
{
    memset( tensor, 0, sizeof( double ) * 9 );
    for ( int i = 0; i < array_len; ++i )
    {
        tensor[ 0 * 3 + 0 ] += mass[ i ] * ( y[ i ] * y[ i ] + z[ i ] * z[ i ] );
        tensor[ 0 * 3 + 1 ] -= mass[ i ] * x[ i ] * y[ i ];
        tensor[ 0 * 3 + 2 ] -= mass[ i ] * x[ i ] * z[ i ];
        tensor[ 1 * 3 + 0 ] -= mass[ i ] * y[ i ] * x[ i ];
        tensor[ 1 * 3 + 1 ] += mass[ i ] * ( x[ i ] * x[ i ] + z[ i ] * z[ i ] );
        tensor[ 1 * 3 + 2 ] -= mass[ i ] * y[ i ] * z[ i ];
        tensor[ 2 * 3 + 0 ] -= mass[ i ] * z[ i ] * x[ i ];
        tensor[ 2 * 3 + 1 ] -= mass[ i ] * z[ i ] * y[ i ];
        tensor[ 2 * 3 + 2 ] += mass[ i ] * ( x[ i ] * x[ i ] + y[ i ] * y[ i ] );
    }
    return 0;
}
#endif
