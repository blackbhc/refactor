#ifndef GALOTFA_ANALYSIS_UNTILS_CPP
#define GALOTFA_ANALYSIS_UNTILS_CPP
#include "utils.h"
#include <math.h>
namespace ana = galotfa::analysis;
bool ana::in_spheroid( double ( &pos )[ 3 ], double& size, double& ratio )
{
    return ( pos[ 0 ] * pos[ 0 ] + pos[ 1 ] * pos[ 1 ] + pos[ 2 ] * pos[ 2 ] / ( ratio * ratio )
             <= size * size );
}
bool ana::in_box( double ( &pos )[ 3 ], double& size, double& ratio )
{
    return ( pos[ 0 ] >= -size / 2 && pos[ 0 ] <= size / 2 && pos[ 1 ] >= -size / 2
             && pos[ 1 ] <= size / 2 && pos[ 2 ] >= -size * ratio / 2
             && pos[ 2 ] <= size * ratio / 2 );
}
bool ana::in_cylinder( double ( &pos )[ 3 ], double& size, double& ratio )
{
    return ( pos[ 0 ] * pos[ 0 ] + pos[ 1 ] * pos[ 1 ] <= size * size )
           && ( pos[ 2 ] >= -size * ratio / 2 && pos[ 2 ] <= size * ratio / 2 );
}
double ana::norm( double ( &vec )[ 3 ] )
{
    return sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
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

    if ( !ana::in_spheroid( test_coord1, size1, ratio1 )
         || !ana::in_spheroid( test_coord1, size1, ratio2 )
         || !ana::in_spheroid( test_coord1, size1, ratio3 )
         || !ana::in_spheroid( test_coord1, size2, ratio1 )
         || !ana::in_spheroid( test_coord1, size2, ratio2 )
         || !ana::in_spheroid( test_coord1, size2, ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_spheroid( test_coord2, size1, ratio1 )
         || ana::in_spheroid( test_coord2, size1, ratio2 )
         || ana::in_spheroid( test_coord2, size1, ratio3 )
         || ana::in_spheroid( test_coord2, size2, ratio1 )
         || !ana::in_spheroid( test_coord2, size2, ratio2 )
         || !ana::in_spheroid( test_coord2, size2, ratio3 ) )
        CHECK_RETURN( false );


    if ( ana::in_spheroid( test_coord3, size1, ratio1 )
         || ana::in_spheroid( test_coord3, size1, ratio2 )
         || ana::in_spheroid( test_coord3, size1, ratio3 )
         || ana::in_spheroid( test_coord3, size2, ratio1 )
         || ana::in_spheroid( test_coord3, size2, ratio2 )
         || ana::in_spheroid( test_coord3, size2, ratio3 ) )
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

    if ( !ana::in_box( test_coord1, size1, ratio1 ) || !ana::in_box( test_coord1, size1, ratio2 )
         || !ana::in_box( test_coord1, size1, ratio3 ) || !ana::in_box( test_coord1, size2, ratio1 )
         || !ana::in_box( test_coord1, size2, ratio2 )
         || !ana::in_box( test_coord1, size2, ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_box( test_coord2, size1, ratio1 ) || ana::in_box( test_coord2, size1, ratio2 )
         || ana::in_box( test_coord2, size1, ratio3 ) || ana::in_box( test_coord2, size2, ratio1 )
         || !ana::in_box( test_coord2, size2, ratio2 )
         || !ana::in_box( test_coord2, size2, ratio3 ) )
        CHECK_RETURN( false );


    if ( ana::in_box( test_coord3, size1, ratio1 ) || ana::in_box( test_coord3, size1, ratio2 )
         || ana::in_box( test_coord3, size1, ratio3 ) || ana::in_box( test_coord3, size2, ratio1 )
         || ana::in_box( test_coord3, size2, ratio2 ) || ana::in_box( test_coord3, size2, ratio3 ) )
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

    if ( !ana::in_cylinder( test_coord1, size1, ratio1 )
         || !ana::in_cylinder( test_coord1, size1, ratio2 )
         || !ana::in_cylinder( test_coord1, size1, ratio3 )
         || !ana::in_cylinder( test_coord1, size2, ratio1 )
         || !ana::in_cylinder( test_coord1, size2, ratio2 )
         || !ana::in_cylinder( test_coord1, size2, ratio3 ) )
        CHECK_RETURN( false );

    if ( ana::in_cylinder( test_coord2, size1, ratio1 )
         || ana::in_cylinder( test_coord2, size1, ratio2 )
         || ana::in_cylinder( test_coord2, size1, ratio3 )
         || ana::in_cylinder( test_coord2, size2, ratio1 )
         || !ana::in_cylinder( test_coord2, size2, ratio2 )
         || !ana::in_cylinder( test_coord2, size2, ratio3 ) )
        CHECK_RETURN( false );


    if ( ana::in_cylinder( test_coord3, size1, ratio1 )
         || ana::in_cylinder( test_coord3, size1, ratio2 )
         || ana::in_cylinder( test_coord3, size1, ratio3 )
         || ana::in_cylinder( test_coord3, size2, ratio1 )
         || ana::in_cylinder( test_coord3, size2, ratio2 )
         || ana::in_cylinder( test_coord3, size2, ratio3 ) )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}

}  // namespace unit_test
#endif
#endif
