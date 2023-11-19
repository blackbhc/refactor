#ifndef GALOTFA_PARTICLE_CPP
#define GALOTFA_PARTICLE_CPP
#include "particle.h"
#include "../analysis/utils.h"
#ifdef debug_particle
#include "../analysis/utils.cpp"
#endif
#include <mpi.h>
#include <string.h>
namespace ana = galotfa::analysis;

int ana::angular_momentum( int part_num, double masses[], double coords[][ 3 ], double vels[][ 3 ],
                           double ( &center )[ 3 ], double** angular_momentum )
{
    if ( angular_momentum == nullptr )
    {
        WARN( "The angular momentum array is not allocated." );
        return 1;
    }

    for ( int i = 0; i < part_num; ++i )
    {
        angular_momentum[ i ][ 0 ] = masses[ i ]
                                     * ( ( coords[ i ][ 1 ] - center[ 1 ] ) * vels[ i ][ 2 ]
                                         - ( coords[ i ][ 2 ] - center[ 2 ] ) * vels[ i ][ 1 ] );
        angular_momentum[ i ][ 1 ] = masses[ i ]
                                     * ( ( coords[ i ][ 2 ] - center[ 2 ] ) * vels[ i ][ 0 ]
                                         - ( coords[ i ][ 0 ] - center[ 0 ] ) * vels[ i ][ 2 ] );
        angular_momentum[ i ][ 2 ] = masses[ i ]
                                     * ( ( coords[ i ][ 0 ] - center[ 0 ] ) * vels[ i ][ 1 ]
                                         - ( coords[ i ][ 1 ] - center[ 1 ] ) * vels[ i ][ 0 ] );
    }

    return 0;
}

int ana::circularity( int part_num, double coords[][ 3 ], double vels[][ 3 ],
                      double ( &center )[ 3 ], double circularity[] )
{
    for ( int i = 0; i < part_num; ++i )
    {
        circularity[ i ] =
            ( ( coords[ i ][ 0 ] - center[ 0 ] ) * vels[ i ][ 1 ]
              - ( coords[ i ][ 1 ] - center[ 1 ] ) * vels[ i ][ 0 ] )
            / ( sqrt( ( coords[ i ][ 0 ] - center[ 0 ] ) * ( coords[ i ][ 0 ] - center[ 0 ] )
                      + ( coords[ i ][ 1 ] - center[ 1 ] ) * ( coords[ i ][ 1 ] - center[ 1 ] )
                      + ( coords[ i ][ 2 ] - center[ 2 ] ) * ( coords[ i ][ 2 ] - center[ 2 ] ) )
                * sqrt( vels[ i ][ 0 ] * vels[ i ][ 0 ] + vels[ i ][ 1 ] * vels[ i ][ 1 ]
                        + vels[ i ][ 2 ] * vels[ i ][ 2 ] ) );
    }
    return 0;
}

int ana::circularity_3d( int part_num, double coords[][ 3 ], double vels[][ 3 ],
                         double ( &center )[ 3 ], double circularity_3d[] )
{
    double** angular_momentum = new double*[ part_num ];
    for ( int i = 0; i < part_num; ++i )
    {
        angular_momentum[ i ] = new double[ 3 ];
    }
    double* masses = new double[ part_num ];  // unit mass
    for ( int i = 0; i < part_num; ++i )
    {
        masses[ i ] = 1;
    }
    ana::angular_momentum( part_num, masses, coords, vels, center, angular_momentum );

    for ( int i = 0; i < part_num; ++i )
    {
        circularity_3d[ i ] =
            ( sqrt( angular_momentum[ i ][ 0 ] * angular_momentum[ i ][ 0 ]
                    + angular_momentum[ i ][ 1 ] * angular_momentum[ i ][ 1 ]
                    + angular_momentum[ i ][ 2 ] * angular_momentum[ i ][ 2 ] ) )
            / ( sqrt( ( coords[ i ][ 0 ] - center[ 0 ] ) * ( coords[ i ][ 0 ] - center[ 0 ] )
                      + ( coords[ i ][ 1 ] - center[ 1 ] ) * ( coords[ i ][ 1 ] - center[ 1 ] )
                      + ( coords[ i ][ 2 ] - center[ 2 ] ) * ( coords[ i ][ 2 ] - center[ 2 ] ) )
                * sqrt( vels[ i ][ 0 ] * vels[ i ][ 0 ] + vels[ i ][ 1 ] * vels[ i ][ 1 ]
                        + vels[ i ][ 2 ] * vels[ i ][ 2 ] ) );
    }

    // release the memory
    for ( int i = 0; i < part_num; ++i )
    {
        delete[] angular_momentum[ i ];
    }
    delete[] angular_momentum;
    delete[] masses;
    return 0;
}

#ifdef debug_particle
#include "../tools/prompt.h"
#include <math.h>
namespace unit_test {

bool fequal( double a, double b )
{
    return fabs( a - b ) < 1e-10;
}

int test_angular_momentum()
{
    println( "Testing the angular momentum function ..." );

    double   coords[ 3 ][ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    double   vels[ 3 ][ 3 ]   = { { 0, 1, 0 }, { 0, 0, 1 }, { 1, 0, 0 } };
    double   masses[ 3 ]      = { 1, 1, 1 };
    double   center[ 3 ]      = { 0, 0, 0 };
    double   target[ 3 ][ 3 ] = { { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } };
    double** angular_momentum = new double*[ 3 ];
    for ( int i = 0; i < 3; ++i )
    {
        angular_momentum[ i ] = new double[ 3 ];
    }

    ana::angular_momentum( 3, masses, coords, vels, center, angular_momentum );

    for ( int i = 0; i < 3; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            if ( !fequal( angular_momentum[ i ][ j ], target[ i ][ j ] ) )
            {
                INFO( "Taget: %lf, Result: %lf", target[ i ][ j ], angular_momentum[ i ][ j ] );
                CHECK_RETURN( false );
            }
        }
    }

    // release the memory
    for ( int i = 0; i < 3; ++i )
    {
        delete[] angular_momentum[ i ];
    }
    delete[] angular_momentum;

    CHECK_RETURN( true );
}

int test_circularity()
{
    println( "Testing the circularity function ..." );

    double coords[ 3 ][ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 } };
    double vels[ 3 ][ 3 ]   = { { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 } };
    double center[ 3 ]      = { 0, 0, 0 };
    double target[ 3 ]      = { 1, 0, 1 / sqrt( 2 ) };
    double circularity[ 3 ];

    ana::circularity( 3, coords, vels, center, circularity );

    for ( int i = 0; i < 3; ++i )
    {
        if ( !fequal( circularity[ i ], target[ i ] ) )
        {
            INFO( "Taget: %lf, Result: %lf", target[ i ], circularity[ i ] );
            CHECK_RETURN( false );
        }
    }

    CHECK_RETURN( true );
}

int test_circularity_3d()
{
    println( "Testing the circularity_3d function ..." );

    double coords[ 3 ][ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 } };
    double vels[ 3 ][ 3 ]   = { { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 } };
    double center[ 3 ]      = { 0, 0, 0 };
    double target[ 3 ]      = { 1, 1, 1 / sqrt( 2 ) };
    double circularity_3d[ 3 ];

    ana::circularity_3d( 3, coords, vels, center, circularity_3d );

    for ( int i = 0; i < 3; ++i )
    {
        if ( !fequal( circularity_3d[ i ], target[ i ] ) )
        {
            INFO( "Taget: %lf, Result: %lf", target[ i ], circularity_3d[ i ] );
            CHECK_RETURN( false );
        }
    }

    CHECK_RETURN( true );
}
}  // namespace unit_test
#endif
#endif
