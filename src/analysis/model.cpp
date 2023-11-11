#ifndef GALOTFA_MODEL_CPP
#define GALOTFA_MODEL_CPP
#include "model.h"
#include "utils.h"
#include <complex>
#include <math.h>
#include <mpi.h>
#include <vector>

using std::complex;
namespace ana = galotfa::analysis;

complex< double > ana::An( unsigned int array_len, double mass[], double x[], double y[],
                           unsigned int order )
{
    complex< double > result = 0 + 0i;
    complex< double > I      = 0 + 1i;
    double            phi    = 0;  // the azimuthal angle

    for ( unsigned int i = 0; i < array_len; i++ )
    {
        phi = atan2( y[ i ], x[ i ] );
        result += mass[ i ] * exp( order * phi * I );
    }

    // MPI reduction
    MPI_Allreduce( MPI_IN_PLACE, &result, 1, MPI_DOUBLE_COMPLEX, MPI_SUM, MPI_COMM_WORLD );
    return result;
}


double ana::s_bar( unsigned int array_len, double mass[], double x[], double y[] )
{
    double s_bar = 0;
    auto   A2    = An( array_len, mass, x, y, 2 );
    auto   A0    = An( array_len, mass, x, y, 0 );
    auto   Abar  = A2 / A0;
    return abs( Abar );
}

double ana::s_buckle( unsigned int array_len, double mass[], double x[], double y[], double z[] )
{
    complex< double > numerator   = 0 + 0i;
    double            denominator = 0;
    complex< double > I           = 0 + 1i;
    double            phi         = 0;  // the azimuthal angle

    for ( unsigned int i = 0; i < array_len; i++ )
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

double ana::bar_major_axis( unsigned int array_len, double mass[], double x[], double y[] )
{
    auto A2 = An( array_len, mass, x, y, 2 );
    return arg( A2 ) / 2;  // divide by 2, as the argument of A2 is 2*phi
}

double ana::bar_length( unsigned int array_len, double mass[], double x[], double y[] )
{
    // TODO: to be implemented
    return 0;
}

int ana::dispersion_tensor( unsigned int array_len, double mass[], double x[], double y[],
                            double z[], double vx[], double vy[], double vz[], double lower_bound_x,
                            double upper_bound_x, double lower_bound_y, double upper_bound_y,
                            double lower_bound_z, double upper_bound_z, unsigned int num_bins_x,
                            unsigned int num_bins_y, unsigned int num_bins_z, double* tensor )
{
    double bin_size_x = ( upper_bound_x - lower_bound_x ) / num_bins_x;
    double bin_size_y = ( upper_bound_y - lower_bound_y ) / num_bins_y;
    double bin_size_z = ( upper_bound_z - lower_bound_z ) / num_bins_z;

    // sigma_ij = < v_i v_j > - < v_i > < v_j >
    // the cross terms of the dispersion tensor
    double* v00 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v01 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v02 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v10 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v11 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v12 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v20 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v21 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v22 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    // the second order moments of the velocity
    double* v0 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v1 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    double* v2 = new double[ bin_size_x * bin_size_y * bin_size_z ]();
    // the first order moments of the velocity
    unsigned long* count = new unsigned long[ bin_size_x * bin_size_y * bin_size_z ]();

    // the factor used to represent the index of the bin
    unsigned int factor1 = bin_size_y * bin_size_z;
    unsigned int factor2 = bin_size_z;

    unsigned int i, j, k;  // tmp variables for the index of the bin

    int index_x, index_y, index_z;  // tmp variables for the index of the bin
    for ( i = 0; i < array_len; ++i )
    {
        if ( x[ i ] < lower_bound_x || x[ i ] > upper_bound_x || y[ i ] < lower_bound_y
             || y[ i ] > upper_bound_y || z[ i ] < lower_bound_z || z[ i ] > upper_bound_z )
        {
            continue;
        }

        // calculate the index of the bin
        index_x = ( int )( x[ i ] - lower_bound_x ) / bin_size_x;
        index_y = ( int )( y[ i ] - lower_bound_y ) / bin_size_y;
        index_z = ( int )( z[ i ] - lower_bound_z ) / bin_size_z;

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
    MPI_Allreduce( MPI_IN_PLACE, v0, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v1, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v2, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v00, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v01, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v02, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v10, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v11, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v12, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v20, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v21, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, v22, bin_size_x * bin_size_y * bin_size_z, MPI_DOUBLE, MPI_SUM,
                   MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, count, bin_size_x * bin_size_y * bin_size_z, MPI_UNSIGNED_LONG,
                   MPI_SUM, MPI_COMM_WORLD );

    // the factor used to represent the index of the bin
    unsigned int factor3 = bin_size_y * bin_size_z * 9;
    unsigned int factor4 = bin_size_z * 9;

    for ( i = 0; i < bin_size_x; i++ )
        for ( j = 0; j < bin_size_y; i++ )
            for ( k = 0; k < bin_size_z; k++ )
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


    return 0;
}

#endif
