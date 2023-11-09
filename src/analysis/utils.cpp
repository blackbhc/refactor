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

int test_vec()
{
    println( "Test the vec struct ..." );
    ana::vec< 3, double > test_vec1{ 3, 4, 5 };
    ana::vec< 3, double > test_vec2 = { -3, -4, 5 };
    ana::vec< 3, double > test_vec3( test_vec1 );

    println( "The size of 3D double vec structure is %lu", sizeof( ana::vec< 3, double > ) );
    println( "Test it can raise an error when the size of the initializer list is not equal to the "
             "dimension of the vector." );
    println( "It should raise a warning, don't worry about it." );
    try
    {
        ana::vec< 3, double > test_vec4{ 3, 4 };
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise an error as expected: %s", e.what() );
    }

    println( "Test the norm() function ..." );
    if ( fabs( test_vec1.norm() - sqrt( 50 ) ) > 1e-6 )
        CHECK_RETURN( false );

    println( "Test the get_length() method ..." );
    if ( test_vec1.get_length() != 3 )
        CHECK_RETURN( false );

    println( "Test the inner product function ..." );
    if ( fabs( test_vec1 * test_vec2 ) > 1e-6 )
        CHECK_RETURN( false );

    println( "Test the scalar product function..." );
    test_vec3 *= 0.5;
    if ( test_vec3 != test_vec1 / 2 )
        CHECK_RETURN( false );

    println( "Test the normalize method ..." );
    test_vec1.normalize();
    if ( fabs( test_vec1.norm() - 1.0 ) > 1e-6 )
        CHECK_RETURN( false );

    println( "Test the cross product function ..." );
    ana::vec< 3, double > unit_x = { 1, 0, 0 };
    ana::vec< 3, double > unit_y = { 0, 1, 0 };
    ana::vec< 3, double > unit_z = { 0, 0, 1 };
    ana::vec< 3, double > vec0   = { 0, 0, 0 };
    if ( unit_x.cross( unit_y ) != unit_z )
        CHECK_RETURN( false );
    if ( unit_y.cross( unit_z ) != unit_x )
        CHECK_RETURN( false );
    if ( unit_z.cross( unit_x ) != unit_y )
        CHECK_RETURN( false );
    if ( unit_x.cross( unit_x ) != vec0 )
        CHECK_RETURN( false );
    if ( unit_y.cross( unit_y ) != vec0 )
        CHECK_RETURN( false );
    if ( unit_z.cross( unit_z ) != vec0 )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}

int test_mat()
{
    println( "Test the mat struct..." );
    ana::mat< 3, 3, double > test_mat1 = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
    ana::vec< 3, double >    test_vec1{ 1, 2, 3 };
    ana::mat< 3, 3, double > test_mat2 = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    ana::mat< 3, 3, double > test_mat3 = { { -2, 0, 0 }, { 0, 1, 0 }, { 0, 0, 3 } };
    ana::mat< 3, 3, double > test_mat4 = { { -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } };
    ana::mat< 3, 3, double > test_mat5 = { { 1, 1, 0 }, { 0, 1, -1 }, { 1, 0, -1 } };
    ana::mat< 4, 4, double > test_mat6 = {
        { 1, 1, 0, 0 }, { 0, 1, -1, 0 }, { 1, 0, -1, 0 }, { 0, -1, 0, 1 }
    };
    println( "The size of 3x3 double mat structure is %lu", sizeof( ana::mat< 3, 3, double > ) );

    println( "Test the is_square() method ..." );
    if ( !test_mat1.is_square() )
        CHECK_RETURN( false );

    println( "Test the subscrip operator ..." );
    if ( test_mat1[ 0 ] != test_vec1 )
        CHECK_RETURN( false );

    println( "Test the determinant method ..." );
    if ( fabs( test_mat1.determinant() - 0 ) > 1e-6 )
    {
        INFO( "The get determinant of test_mat1 is %.30lf", test_mat1.determinant() );
        CHECK_RETURN( false );
    }
    if ( fabs( test_mat2.determinant() - 1 ) > 1e-6 )
        CHECK_RETURN( false );
    if ( fabs( test_mat3.determinant() + 6 ) > 1e-6 )
        CHECK_RETURN( false );
    if ( fabs( test_mat4.determinant() - 1 ) > 1e-6 )
        CHECK_RETURN( false );
    if ( fabs( test_mat5.determinant() + 2 ) > 1e-6 )
        CHECK_RETURN( false );
    if ( fabs( test_mat6.determinant() + 2 ) > 1e-6 )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}

int test_bin1d()
{
    println( "Test the bin1d() function..." );

    double test_coord1[ 5 ] = { 0 };
    double test_coord2[ 5 ] = { 0 };
    for ( int i = 0; i < 5; ++i )
    {
        test_coord1[ i ] = ( double )i + .5;
        test_coord2[ i ] = ( double )i + pow( -1, i );
    }
    double test_data1[ 5 ] = { 0 };
    for ( int i = 0; i < 5; ++i )
        test_data1[ i ] = 2;

    ana::vec< 5, double > res1 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::count );
    ana::vec< 5, double > expected1{ 1, 1, 1, 1, 1 };
    if ( res1 != expected1 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%lf", res1[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%lf", expected1[ i ] );
        CHECK_RETURN( false );
    }

    ana::vec< 5, double > res2 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::sum );
    ana::vec< 5, double > expected2{ 2, 2, 2, 2, 2 };
    if ( res2 != expected2 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", res2[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    ana::vec< 5, double > res3 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::max );
    if ( res3 != expected2 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    ana::vec< 5, double > res4 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::max );
    if ( res4 != expected2 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    ana::vec< 5, double > res5 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::mean );
    if ( res5 != expected2 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    ana::vec< 5, double > res6 =
        ana::bin1d< 5, 5, double >( test_coord1, test_data1, 0, 5, ana::stats_method::mean );
    if ( res6 != expected2 )
    {
        println( "The result is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( int i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    double test_coord3[ 20 ] = { 0 };
    for ( int i = 0; i < 20; ++i )
        test_coord3[ i ] = ( double )i / 20 * 5;
    double test_data3[ 20 ] = { 0 };
    for ( int i = 0; i < 20; ++i )
        test_data3[ i ] = ( double )( i % 4 + 1 );

    ana::vec< 5, double > res7 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::count );
    ana::vec< 5, double > res8 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::sum );
    ana::vec< 5, double > res9 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::min );
    ana::vec< 5, double > res10 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::max );
    ana::vec< 5, double > res11 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::mean );
    ana::vec< 5, double > res12 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::median );
    ana::vec< 5, double > res13 =
        ana::bin1d< 20, 5, double >( test_coord3, test_data3, 0, 5, ana::stats_method::std );
    ana::vec< 5, double > expected7{ 4, 4, 4, 4, 4 };
    ana::vec< 5, double > expected8{ 10, 10, 10, 10, 10 };
    ana::vec< 5, double > expected9{ 1, 1, 1, 1, 1 };
    ana::vec< 5, double > expected10{ 4, 4, 4, 4, 4 };
    ana::vec< 5, double > expected11{ 2.5, 2.5, 2.5, 2.5, 2.5 };
    ana::vec< 5, double > expected12{ 2.5, 2.5, 2.5, 2.5, 2.5 };
    ana::vec< 5, double > expected13{ 1.11803399, 1.11803399, 1.11803399, 1.11803399, 1.11803399 };

    if ( res7 != expected7 || res8 != expected8 || res9 != expected9 || res10 != expected10
         || res11 != expected11 || res12 != expected12 || res13 != expected13 )
    {
        // println( "The result is:" );
        // for ( int i = 0; i < 5; ++i )
        //     println( "%lf", res7[ i ] );
        // println( "The expected is:" );
        // for ( int i = 0; i < 5; ++i )
        //     println( "%lf", expected7[ i ] );
        CHECK_RETURN( false );
    }

    // I am tired of writing the same code again and again ...
    // if ( res7 != expected7 || res8 != expected8 || res9 != expected9 || res10 != expected10
    //      || res11 != expected11 || res12 != expected12 || res13 != expected13 )
    //     CHECK_RETURN( false );


    CHECK_RETURN( true );
}

int test_bin2d( void )
{
    println( "Test the bin2d() function..." );
    double coord_x[ 100 ] = { 0 }, coord_y[ 100 ] = { 0 };
    for ( int i = 0; i < 100; ++i )
    {
        coord_y[ i ] = coord_x[ i ] = i;
    }
    double data[ 100 ] = { 0 };
    for ( int i = 0; i < 100; i++ )
    {
        data[ i ] = double( i * 2 - i * i ) * pow( -1, i );
    }

    ana::mat< 4, 4, double > res1 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::min );
    ana::mat< 4, 4, double > res2 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::max );
    ana::mat< 4, 4, double > res3 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::sum );
    ana::mat< 4, 4, double > res4 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::count );
    ana::mat< 4, 4, double > res5 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::mean );
    ana::mat< 4, 4, double > res6 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::median );
    ana::mat< 4, 4, double > res7 = ana::bin2d< 100, 4, 4, double >(
        coord_x, coord_y, data, 0, 100, 0, 100, ana::stats_method::std );

    ana::mat< 4, 4, double > exp1{ { -528, nan( "" ), nan( "" ), nan( "" ) },
                                   { nan( "" ), -2208, nan( "" ), nan( "" ) },
                                   { nan( "" ), nan( "" ), -5328, nan( "" ) },
                                   { nan( "" ), nan( "" ), nan( "" ), -9408 } };
    ana::mat< 4, 4, double > exp2{ { 483, nan( "" ), nan( "" ), nan( "" ) },
                                   { nan( "" ), 2303, nan( "" ), nan( "" ) },
                                   { nan( "" ), nan( "" ), 5183, nan( "" ) },
                                   { nan( "" ), nan( "" ), nan( "" ), 9603 } };
    ana::mat< 4, 4, double > exp3{
        { -276, 0, 0, 0 }, { 0, 1451, 0, 0 }, { 0, 0, -3876, 0 }, { 0, 0, 0, 7551 }
    };
    ana::mat< 4, 4, double > exp4{
        { 25, 0, 0, 0 }, { 0, 25, 0, 0 }, { 0, 0, 25, 0 }, { 0, 0, 0, 25 }
    };
    ana::mat< 4, 4, double > exp5{ { -11.04, nan( "" ), nan( "" ), nan( "" ) },
                                   { nan( "" ), 58.04, nan( "" ), nan( "" ) },
                                   { nan( "" ), nan( "" ), -155.04, nan( "" ) },
                                   { nan( "" ), nan( "" ), nan( "" ), 302.04 } };
    ana::mat< 4, 4, double > exp6{ { 0, nan( "" ), nan( "" ), nan( "" ) },
                                   { nan( "" ), 575, nan( "" ), nan( "" ) },
                                   { nan( "" ), nan( "" ), -2400, nan( "" ) },
                                   { nan( "" ), nan( "" ), nan( "" ), 5475 } };
    ana::mat< 4, 4, double > exp7{ { 238.29166666, nan( "" ), nan( "" ), nan( "" ) },
                                   { nan( "" ), 1443.17745215, nan( "" ), nan( "" ) },
                                   { nan( "" ), nan( "" ), 3870.40920297, nan( "" ) },
                                   { nan( "" ), nan( "" ), nan( "" ), 7543.67958217 } };

    if ( res1 != exp1 || res2 != exp2 || res3 != exp3 || res4 != exp4 || res5 != exp5
         || res6 != exp6 || res7 != exp7 )
        CHECK_RETURN( false );

    CHECK_RETURN( true );
}
}  // namespace unit_test
#endif
#endif
