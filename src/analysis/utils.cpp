#ifndef GALOTFA_ANALYSIS_UNTILS_CPP
#define GALOTFA_ANALYSIS_UNTILS_CPP
#include "utils.h"
namespace ana = galotfa::analysis;
bool ana::in_spheroid( double& coordx, double& coordy, double& coordz, double& size, double& ratio )
{
    return ( coordx * coordx + coordy * coordy + coordz * coordz / ( ratio * ratio )
             <= size * size );
}
bool ana::in_box( double& coordx, double& coordy, double& coordz, double& size, double& ratio )
{
    return ( coordx >= -size / 2 && coordx <= size / 2 && coordy >= -size / 2 && coordy <= size / 2
             && coordz >= -size * ratio / 2 && coordz <= size * ratio / 2 );
}
bool ana::in_cylinder( double& coordx, double& coordy, double& coordz, double& size, double& ratio )
{
    return ( coordx * coordx + coordy * coordy <= size * size )
           && ( coordz >= -size * ratio / 2 && coordz <= size * ratio / 2 );
}

#ifdef debug_utils
#include "../tools/prompt.h"
namespace unit_test {
int test_in_spheroid()
{
    println( "Testsint in_spheroid() ..." );
    double test_coord1[] = { 0, 0, 0 };
    double test_coord2[] = { 1, 1, 1 };
    double test_coord3[] = { -2, 2, -2 };
    double ratio1        = 0.5;
    double ratio2        = 1.0;
    double ratio3        = 1.5;
    double size1         = 1.0;
    double size2         = 2.0;

    if ( !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio1 )
         || !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio2 )
         || !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio3 )
         || !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio1 )
         || !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio2 )
         || !ana::in_spheroid( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2,
                               ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio1 )
         || ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio2 )
         || ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio3 )
         || ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio1 )
         || !ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio2 )
         || !ana::in_spheroid( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2,
                               ratio3 ) )
        CHECK_RETURN( false );


    if ( ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio1 )
         || ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio2 )
         || ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio3 )
         || ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio1 )
         || ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio2 )
         || ana::in_spheroid( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2,
                              ratio3 ) )
        CHECK_RETURN( false );
    CHECK_RETURN( true );
}
int test_in_box()
{
    println( "Testsint in_box() ..." );
    double test_coord1[] = { 0, 0, 0 };
    double test_coord2[] = { 1, 1, 1 };
    double test_coord3[] = { -2, 2, -2 };
    double ratio1        = 0.5;
    double ratio2        = 1.0;
    double ratio3        = 1.5;
    double size1         = 1.0;
    double size2         = 2.0;
    if ( !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio1 )
         || !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio2 )
         || !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio3 )
         || !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio1 )
         || !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio2 )
         || !ana::in_box( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio1 )
         || ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio2 )
         || ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio3 )
         || ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio1 )
         || !ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio2 )
         || !ana::in_box( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio1 )
         || ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio2 )
         || ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio3 )
         || ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio1 )
         || ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio2 )
         || ana::in_box( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio3 ) )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}
int test_in_cylinder()
{
    println( "Testsint in_cylinder() ..." );
    double test_coord1[] = { 0, 0, 0 };
    double test_coord2[] = { 1, 1, 1 };
    double test_coord3[] = { -2, 2, -2 };
    double ratio1        = 0.5;
    double ratio2        = 1.0;
    double ratio3        = 1.5;
    double size1         = 1.0;
    double size2         = 2.0;

    if ( !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio1 )
         || !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio2 )
         || !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size1, ratio3 )
         || !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio1 )
         || !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2, ratio2 )
         || !ana::in_cylinder( test_coord1[ 0 ], test_coord1[ 1 ], test_coord1[ 2 ], size2,
                               ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio1 )
         || ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio2 )
         || ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size1, ratio3 )
         || ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio1 )
         || !ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2, ratio2 )
         || !ana::in_cylinder( test_coord2[ 0 ], test_coord2[ 1 ], test_coord2[ 2 ], size2,
                               ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio1 )
         || ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio2 )
         || ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size1, ratio3 )
         || ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio1 )
         || ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2, ratio2 )
         || ana::in_cylinder( test_coord3[ 0 ], test_coord3[ 1 ], test_coord3[ 2 ], size2,
                              ratio3 ) )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}

}  // namespace unit_test
#endif
#endif
