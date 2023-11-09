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

vector< double > ana::bin1d( unsigned int array_len, double coord[], double data[],
                             double lower_bound, double upper_bound, unsigned int bin_num,
                             stats_method method )
{
    if ( bin_num == 0 || array_len == 0 )
        ERROR( "The number of bins should be larger than 0." );
    vector< double > bins_1d( bin_num );  // the final result

    // the index of the data points in each bin
    // unsigned int bins[ bin_num ][ array_len ] = { 0 };
    unsigned int** bins = new unsigned int*[ bin_num ]();
    for ( unsigned int i = 0; i < bin_num; ++i )
        bins[ i ] = new unsigned int[ array_len ]();
    // the counts of the data points in each bin
    unsigned int* counts = new unsigned int[ bin_num ]();

    double range = upper_bound - lower_bound;

    // iterate over the data points to find its bin
    for ( unsigned int i = 0; i < array_len; ++i )
    {
        if ( coord[ i ] < lower_bound || coord[ i ] > upper_bound )
            continue;
        // the index of the bin
        unsigned int bin_index = ( unsigned int )( ( coord[ i ] - lower_bound ) / range * bin_num );
        if ( bin_index == bin_num )
            bin_index = bin_num - 1;  // avoid the overflow at the upper bound

        bins[ bin_index ][ counts[ bin_index ]++ ] = i;
    }

    if ( method == stats_method::count )
    {
        for ( unsigned int i = 0; i < bin_num; ++i )
            bins_1d[ i ] = ( double )counts[ i ];
    }
    else
    {
        double* data_points[ bin_num ];

        unsigned int i = 0, j = 0;
        for ( i = 0; i < bin_num; ++i )
        {
            if ( counts[ i ] > 0 )
            {
                data_points[ i ] = new double[ counts[ i ] ];
                for ( j = 0; j < counts[ i ]; ++j )
                    data_points[ i ][ j ] = data[ bins[ i ][ j ] ];
            }
        }
        switch ( method )
        {
        case stats_method::min: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = nan( "" );
                else
                    bins_1d[ i ] = ( double )*std::min_element( data_points[ i ],
                                                                data_points[ i ] + counts[ i ] );
            }
            break;
        }
        case stats_method::max: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = nan( "" );
                else
                    bins_1d[ i ] = ( double )*std::max_element( data_points[ i ],
                                                                data_points[ i ] + counts[ i ] );
            }
            break;
        }
        case stats_method::mean: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = nan( "" );
                else
                    bins_1d[ i ] = ( double )std::accumulate( data_points[ i ],
                                                              data_points[ i ] + counts[ i ], 0.0 )
                                   / counts[ i ];
            }
            break;
        }
        case stats_method::median: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = nan( "" );
                else
                {
                    std::sort( data_points[ i ], data_points[ i ] + counts[ i ] );
                    if ( counts[ i ] % 2 == 0 )
                        bins_1d[ i ] = ( double )( data_points[ i ][ counts[ i ] / 2 - 1 ]
                                                   + data_points[ i ][ counts[ i ] / 2 ] )
                                       / 2;
                    else
                        bins_1d[ i ] = ( double )data_points[ i ][ counts[ i ] / 2 ];
                }
            }
            break;
        }
        case stats_method::sum: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = 0;
                else
                    bins_1d[ i ] = ( double )std::accumulate( data_points[ i ],
                                                              data_points[ i ] + counts[ i ], 0.0 );
            }
            break;
        }
        case stats_method::std: {
            for ( i = 0; i < bin_num; ++i )
            {
                if ( counts[ i ] == 0 )
                    bins_1d[ i ] = nan( "" );
                else
                {
                    double mean =
                        std::accumulate( data_points[ i ], data_points[ i ] + counts[ i ], 0.0 )
                        / counts[ i ];
                    double sum = 0;
                    for ( j = 0; j < counts[ i ]; ++j )
                        sum += ( data_points[ i ][ j ] - mean ) * ( data_points[ i ][ j ] - mean );
                    bins_1d[ i ] = ( double )sqrt( sum / counts[ i ] );
                }
            }
            break;
        }
        default:  // never reach here
            break;
        }
        for ( i = 0; i < bin_num; ++i )
            if ( counts[ i ] > 0 )
                delete[] data_points[ i ];
    }

    // release the memory
    for ( unsigned int i = 0; i < bin_num; ++i )
        delete[] bins[ i ];
    delete[] bins;
    delete[] counts;

    return bins_1d;
}

vector< vector< double > >
ana::bin2d( unsigned int array_len, double coord_x[], double coord_y[ array_len ], double data[],
            double lower_bound_x, double upper_bound_x, double lower_bound_y, double upper_bound_y,
            unsigned int bin_numx, unsigned int bin_numy, ana::stats_method method )
{
    if ( bin_numx == 0 || bin_numy == 0 )
        ERROR( "The number of bins should be larger than 0." );

    vector< vector< double > > bins_2d( bin_numx,
                                        vector< double >( bin_numy ) );  // the final result

    // the index of the data points in each bin
    unsigned int*** bins = new unsigned int**[ bin_numx ];
    for ( unsigned int i = 0; i < bin_numx; ++i )
    {
        bins[ i ] = new unsigned int*[ bin_numy ];
        for ( unsigned int j = 0; j < bin_numy; ++j )
            bins[ i ][ j ] = new unsigned int[ array_len ]();
    }
    // the counts of the data points in each bin
    unsigned int** counts = new unsigned int*[ bin_numx ];
    for ( unsigned int i = 0; i < bin_numx; ++i )
        counts[ i ] = new unsigned int[ bin_numy ]();

    double range_x = upper_bound_x - lower_bound_x;
    double range_y = upper_bound_y - lower_bound_y;

    // iterate over the data points to find its bin
    for ( unsigned int i = 0; i < array_len; ++i )
    {
        if ( coord_x[ i ] < lower_bound_x || coord_x[ i ] > upper_bound_x )
            continue;
        else if ( coord_y[ i ] < lower_bound_y || coord_y[ i ] > upper_bound_y )
            continue;
        // the index of the bin
        unsigned int bin_index_x =
            ( unsigned int )( ( coord_x[ i ] - lower_bound_x ) / range_x * bin_numx );
        unsigned int bin_index_y =
            ( unsigned int )( ( coord_y[ i ] - lower_bound_y ) / range_y * bin_numy );
        if ( bin_index_x == bin_numx )
            bin_index_x = bin_numx - 1;  // avoid the overflow at the upper bound
        if ( bin_index_y == bin_numy )
            bin_index_y = bin_numy - 1;  // avoid the overflow at the upper bound

        bins[ bin_index_x ][ bin_index_y ][ counts[ bin_index_x ][ bin_index_y ]++ ] = i;
    }

    if ( method == stats_method::count )
    {
        for ( unsigned int i = 0; i < bin_numx; ++i )
            for ( unsigned int j = 0; j < bin_numy; ++j )
                bins_2d[ i ][ j ] = ( double )counts[ i ][ j ];
    }
    else
    {
        double* data_points[ bin_numx ][ bin_numy ];

        unsigned int i = 0, j = 0, k = 0;
        for ( i = 0; i < bin_numx; ++i )
            for ( j = 0; j < bin_numy; ++j )
            {
                if ( counts[ i ][ j ] > 0 )
                {
                    data_points[ i ][ j ] = new double[ counts[ i ][ j ] ];
                    for ( k = 0; k < counts[ i ][ j ]; ++k )
                        data_points[ i ][ j ][ k ] = data[ bins[ i ][ j ][ k ] ];
                }
            }
        switch ( method )
        {
        case stats_method::min: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = nan( "" );
                    else
                        bins_2d[ i ][ j ] = *std::min_element(
                            data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ] );
                }
            break;
        }
        case stats_method::max: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = nan( "" );
                    else
                        bins_2d[ i ][ j ] = *std::max_element(
                            data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ] );
                }
            break;
        }
        case stats_method::mean: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = nan( "" );
                    else
                        bins_2d[ i ][ j ] = ( double )std::accumulate(
                                                data_points[ i ][ j ],
                                                data_points[ i ][ j ] + counts[ i ][ j ], 0.0 )
                                            / counts[ i ][ j ];
                }
            break;
        }
        case stats_method::median: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = nan( "" );
                    else
                    {
                        std::sort( data_points[ i ][ j ],
                                   data_points[ i ][ j ] + counts[ i ][ j ] );
                        if ( counts[ i ][ j ] % 2 == 0 )
                            bins_2d[ i ][ j ] = ( data_points[ i ][ j ][ counts[ i ][ j ] / 2 - 1 ]
                                                  + data_points[ i ][ j ][ counts[ i ][ j ] / 2 ] )
                                                / 2;
                        else
                            bins_2d[ i ][ j ] = data_points[ i ][ j ][ counts[ i ][ j ] / 2 ];
                    }
                }
            break;
        }
        case stats_method::sum: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = 0;
                    else
                        bins_2d[ i ][ j ] = std::accumulate(
                            data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ], 0.0 );
                }
            break;
        }
        case stats_method::std: {
            for ( i = 0; i < bin_numx; ++i )
                for ( j = 0; j < bin_numy; ++j )
                {
                    if ( counts[ i ][ j ] == 0 )
                        bins_2d[ i ][ j ] = nan( "" );
                    else
                    {
                        double mean =
                            std::accumulate( data_points[ i ][ j ],
                                             data_points[ i ][ j ] + counts[ i ][ j ], 0.0 )
                            / counts[ i ][ j ];
                        double sum = 0;
                        for ( k = 0; k < counts[ i ][ j ]; ++k )
                            sum += ( data_points[ i ][ j ][ k ] - mean )
                                   * ( data_points[ i ][ j ][ k ] - mean );
                        bins_2d[ i ][ j ] = sqrt( sum / counts[ i ][ j ] );
                    }
                }
            break;
        }
        default:  // never reach here
            break;
        }
        for ( i = 0; i < bin_numx; ++i )
            for ( j = 0; j < bin_numy; ++j )
                if ( counts[ i ][ j ] > 0 )
                    delete[] data_points[ i ][ j ];
    }

    // release the memory
    for ( unsigned int i = 0; i < bin_numx; ++i )
    {
        for ( unsigned int j = 0; j < bin_numy; ++j )
            delete[] bins[ i ][ j ];
        delete[] bins[ i ];
    }
    delete[] bins;
    for ( unsigned int i = 0; i < bin_numx; ++i )
        delete[] counts[ i ];
    delete[] counts;
    return bins_2d;
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
             "dimension of the ana::vec." );
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

    auto res1 = ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::count );
    vector< double > expected1{ 1, 1, 1, 1, 1 };
    if ( res1 != expected1 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%lf", res1[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%lf", expected1[ i ] );
        CHECK_RETURN( false );
    }

    vector< double > res2 =
        ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::sum );
    vector< double > expected2{ 2, 2, 2, 2, 2 };
    if ( res2 != expected2 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", res2[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    vector< double > res3 =
        ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::max );
    if ( res3 != expected2 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    vector< double > res4 =
        ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::max );
    if ( res4 != expected2 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    vector< double > res5 =
        ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::mean );
    if ( res5 != expected2 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    vector< double > res6 =
        ana::bin1d( 5, test_coord1, test_data1, 0, 5, 5, ana::stats_method::mean );
    if ( res6 != expected2 )
    {
        println( "The result is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", res3[ i ] );
        println( "The expected is:" );
        for ( size_t i = 0; i < 5; ++i )
            println( "%f", expected2[ i ] );
        CHECK_RETURN( false );
    }

    double test_coord3[ 20 ] = { 0 };
    for ( int i = 0; i < 20; ++i )
        test_coord3[ i ] = ( double )i / 20 * 5;
    double test_data3[ 20 ] = { 0 };
    for ( int i = 0; i < 20; ++i )
        test_data3[ i ] = ( double )( i % 4 + 1 );

    vector< double > res7 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::count );
    vector< double > res8 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::sum );
    vector< double > res9 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::min );
    vector< double > res10 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::max );
    vector< double > res11 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::mean );
    vector< double > res12 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::median );
    vector< double > res13 =
        ana::bin1d( 20, test_coord3, test_data3, 0, 5, 5, ana::stats_method::std );
    vector< double > expected7{ 4, 4, 4, 4, 4 };
    vector< double > expected8{ 10, 10, 10, 10, 10 };
    vector< double > expected9{ 1, 1, 1, 1, 1 };
    vector< double > expected10{ 4, 4, 4, 4, 4 };
    vector< double > expected11{ 2.5, 2.5, 2.5, 2.5, 2.5 };
    vector< double > expected12{ 2.5, 2.5, 2.5, 2.5, 2.5 };
    vector< double > expected13{ 1.11803399, 1.11803399, 1.11803399, 1.11803399, 1.11803399 };

    // I am tired of writing the same code again and again ...
    if ( res7 != expected7 || res8 != expected8 || res9 != expected9 || res10 != expected10
         || res11 != expected11 || res12 != expected12 )
        CHECK_RETURN( false );

    for ( size_t i = 0; i < 5; ++i )
        if ( fabs( res13[ i ] - expected13[ i ] ) > 1e-6 )
            CHECK_RETURN( false );

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

    auto res1 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::min );
    auto res2 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::max );
    auto res3 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::sum );
    auto res4 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::count );
    auto res5 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::mean );
    auto res6 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::median );
    auto res7 =
        ana::bin2d( 100, coord_x, coord_y, data, 0, 100, 0, 100, 4, 4, ana::stats_method::std );

    vector< vector< double > > exp1{ { -528, nan( "" ), nan( "" ), nan( "" ) },
                                     { nan( "" ), -2208, nan( "" ), nan( "" ) },
                                     { nan( "" ), nan( "" ), -5328, nan( "" ) },
                                     { nan( "" ), nan( "" ), nan( "" ), -9408 } };
    vector< vector< double > > exp2{ { 483, nan( "" ), nan( "" ), nan( "" ) },
                                     { nan( "" ), 2303, nan( "" ), nan( "" ) },
                                     { nan( "" ), nan( "" ), 5183, nan( "" ) },
                                     { nan( "" ), nan( "" ), nan( "" ), 9603 } };
    vector< vector< double > > exp3{
        { -276, 0, 0, 0 }, { 0, 1451, 0, 0 }, { 0, 0, -3876, 0 }, { 0, 0, 0, 7551 }
    };
    vector< vector< double > > exp4{
        { 25, 0, 0, 0 }, { 0, 25, 0, 0 }, { 0, 0, 25, 0 }, { 0, 0, 0, 25 }
    };
    vector< vector< double > > exp5{ { -11.04, nan( "" ), nan( "" ), nan( "" ) },
                                     { nan( "" ), 58.04, nan( "" ), nan( "" ) },
                                     { nan( "" ), nan( "" ), -155.04, nan( "" ) },
                                     { nan( "" ), nan( "" ), nan( "" ), 302.04 } };
    vector< vector< double > > exp6{ { 0, nan( "" ), nan( "" ), nan( "" ) },
                                     { nan( "" ), 575, nan( "" ), nan( "" ) },
                                     { nan( "" ), nan( "" ), -2400, nan( "" ) },
                                     { nan( "" ), nan( "" ), nan( "" ), 5475 } };
    vector< vector< double > > exp7{ { 238.29166666, nan( "" ), nan( "" ), nan( "" ) },
                                     { nan( "" ), 1443.17745215, nan( "" ), nan( "" ) },
                                     { nan( "" ), nan( "" ), 3870.40920297, nan( "" ) },
                                     { nan( "" ), nan( "" ), nan( "" ), 7543.67958217 } };

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res1[ i ][ j ] - exp1[ i ][ j ] ) > 1e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res2[ i ][ j ] - exp2[ i ][ j ] ) > 2e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res3[ i ][ j ] - exp3[ i ][ j ] ) > 3e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res4[ i ][ j ] - exp4[ i ][ j ] ) > 4e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res5[ i ][ j ] - exp5[ i ][ j ] ) > 1e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res6[ i ][ j ] - exp6[ i ][ j ] ) > 1e-6 )
                CHECK_RETURN( false )

    for ( size_t i = 0; i < 4; ++i )
        for ( size_t j = 0; j < 4; ++j )
            if ( fabs( res7[ i ][ j ] - exp7[ i ][ j ] ) > 1e-6 )
                CHECK_RETURN( false )

    CHECK_RETURN( true );
}
}  // namespace unit_test
#endif
#endif
