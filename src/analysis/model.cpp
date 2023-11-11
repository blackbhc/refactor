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

#endif
