#ifndef GALOTFA_ANALYSIS_UNTILS_H
#define GALOTFA_ANALYSIS_UNTILS_H
#include "../tools/prompt.h"
#include <algorithm>
#include <gsl/gsl_linalg.h>
#include <initializer_list>
#include <math.h>
#include <numeric>
#include <string.h>
#include <vector>
using std::vector;
namespace galotfa {
namespace analysis {
    bool in_spheroid( double ( &pos )[ 3 ], double& size, double& ratio );

    bool in_box( double ( &pos )[ 3 ], double& size, double& ratio );

    bool in_cylinder( double ( &pos )[ 3 ], double& size, double& ratio );

    double norm( double ( &vec )[ 3 ] );

    // the vector class: support ordinary operations
    template < unsigned int N, typename Type > struct vec
    {
        Type data[ N ] = { 0 };

        inline double norm( void )
        {
            Type sum = 0;
            for ( unsigned int i = 0; i < N; ++i )
                sum += data[ i ] * data[ i ];
            return sqrt( sum );
        }

        void normalize( void )
        {
            if ( this->norm() > 0 )
                ( *this ) /= norm();
        }

        inline unsigned int get_length( void )
        {
            return N;
        }

        vec( void ) {}

        // the constructor from an initializer list
        vec( std::initializer_list< Type > list )
        {
            if ( list.size() != N )
            {
                WARN( "The size of the initializer list is not equal to the dimension of the "
                      "vector, just ignore the given list." );
                return;
            }
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = *( list.begin() + i );
        }

        // the constructor from an array
        vec( Type ( &array )[ N ] )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = array[ i ];
        }

        // the constructor from a single number
        vec( Type number )
        {
            memset( data, ( int )number, sizeof( Type ) * N );
        }

        // the copy constructor
        vec( const vec< N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                data[ i ] = rhs.data[ i ];
        }


        inline const Type& operator[]( unsigned int i ) const
        {
            return data[ i ];
        }
        inline const Type& operator[]( int i ) const
        {
            return data[ i ];
        }

        inline void operator=( const Type ( &array )[ N ] )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = array[ i ];
        }
        inline void operator=( const vec< N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = rhs.data[ i ];
        }
        inline void operator=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = rhs;
        }

        inline vec< N, Type > operator+( const vec< N, Type >& rhs ) const
        {
            vec< N, Type > result;
            for ( unsigned int i = 0; i < N; ++i )
                result.data[ i ] = data[ i ] + rhs.data[ i ];
            return result;
        }
        inline vec< N, Type > operator+( const Type& rhs ) const
        {
            vec< N, Type > result;
            for ( unsigned int i = 0; i < N; ++i )
                result.data[ i ] = data[ i ] + rhs;
            return result;
        }
        inline void operator+=( const vec< N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] += rhs.data[ i ];
        }
        inline void operator+=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] += rhs;
        }

        inline vec< N, Type > operator-( const vec< N, Type >& rhs ) const
        {
            vec< N, Type > result;
            for ( unsigned int i = 0; i < N; ++i )
                result.data[ i ] = data[ i ] - rhs.data[ i ];
            return result;
        }
        inline void operator-=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] -= rhs;
        }

        inline Type operator*( const vec< N, Type >& rhs ) const
        {
            Type result = 0;
            for ( unsigned int i = 0; i < N; ++i )
                result += data[ i ] * rhs.data[ i ];
            return result;
        }
        inline vec< N, Type > operator*( const Type& rhs ) const
        {
            vec< N, Type > result( *this );
            result *= rhs;
            return result;
        }
        inline void operator*=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] *= rhs;
        }

        inline vec< N, Type > operator/( const Type& rhs ) const
        {
            vec< N, Type > result( *this );
            result /= rhs;
            return result;
        }
        inline void operator/=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] /= rhs;
        }

        inline bool operator==( const vec< N, Type >& rhs ) const
        {
            auto error = this->operator-( rhs );
            return error.norm() < 1e-6;
            // the numerical error is set to be 1e-6
        }
        inline bool operator!=( const vec< N, Type >& rhs ) const
        {
            return !( this->operator==( rhs ) );
        }

        vec< N, Type > cross( const vec< N, Type >& rhs ) const
        {
            if ( N != 3 )
                ERROR( "The cross product is only defined for 3D vectors." );
            vec< 3, Type > result;
            result.data[ 0 ] = data[ 1 ] * rhs.data[ 2 ] - data[ 2 ] * rhs.data[ 1 ];
            result.data[ 1 ] = data[ 2 ] * rhs.data[ 0 ] - data[ 0 ] * rhs.data[ 2 ];
            result.data[ 2 ] = data[ 0 ] * rhs.data[ 1 ] - data[ 1 ] * rhs.data[ 0 ];
            return result;
        }
    };
    template < unsigned int M, unsigned int N, typename Type > struct mat
    {
        Type         data[ M ][ N ];
        unsigned int row = M;
        unsigned int col = N;

        mat( void ) {}

        // constructor from an array
        mat( Type ( &array )[ M ][ N ] )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = array[ i ][ j ];
        }

        // constructor from an initializer_list
        mat( std::initializer_list< std::initializer_list< Type > > list )
        {
            if ( list.size() != M )
            {
                WARN( "The size of the initializer list is not equal to the dimension of the "
                      "vector, ignore the given list." );
                return;
            }
            else if ( ( *list.begin() ).size() != N )
            {
                WARN( "The size of the initializer list is not equal to the dimension of the "
                      "vector, ignore the given list." );
                return;
            }
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = *( ( *( list.begin() + i ) ).begin() + j );
        }

        // constructor from a single number
        mat( const Type& number )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = number;
        }

        // copy constructor
        mat( const mat< M, N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = rhs.data[ i ][ j ];
        }

        inline bool is_square( void )
        {
            return M == N;
        }

        inline vec< N, Type > operator[]( unsigned int i )
        {
            vec< N, Type > result = this->data[ i ];
            return result;
        }
        inline vec< N, Type > operator[]( int i )
        {
            vec< N, Type > result = this->data[ i ];
            return result;
        }

        inline mat< N, M, Type > transpose( void )
        {
            mat< N, M, Type > result;
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    result.data[ j ][ i ] = data[ i ][ j ];
            return result;
        }

        inline mat< N, M, Type > T( void )  // alias of transpose
        {
            return this->transpose();
        }

        inline vec< N, Type > get_row( unsigned int i )
        {
            return this->operator[]( i );
        }

        inline vec< M, Type > get_col( unsigned int i )
        {
            vec< M, Type > result;
            for ( unsigned int j = 0; j < M; ++j )
                result.data[ j ] = data[ j ][ i ];
            return result;
        }
        inline vec< N, Type > get_row( int i )
        {
            return this->operator[]( i );
        }

        inline vec< M, Type > get_col( int i )
        {
            vec< M, Type > result;
            for ( unsigned int j = 0; j < M; ++j )
                result.data[ j ] = data[ j ][ i ];
            return result;
        }

        inline bool operator==( const mat< M, N, Type >& rhs ) const
        {
            if ( M != rhs.row || N != rhs.col )
            {
                WARN( "For operator==: the size of the two matrices are not equal, treat them are  "
                      "unequal." );
                return false;
            }
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    if ( fabs( data[ i ][ j ] - rhs.data[ i ][ j ] ) > 1e-6 )
                        return false;
            return true;
        }
        inline bool operator!=( const mat< M, N, Type >& rhs ) const
        {
            return !( this->operator==( rhs ) );
        }

        inline void operator=( const mat< M, N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = rhs.data[ i ][ j ];
        }
        inline void operator=( Type& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] = rhs;
        }

        inline void operator+=( const mat< M, N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] += rhs.data[ i ][ j ];
        }
        inline void operator+=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] += rhs;
        }

        inline void operator-=( const mat< M, N, Type >& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] -= rhs.data[ i ][ j ];
        }
        inline void operator-=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] -= rhs;
        }

        inline mat< M, N, Type > operator*( const Type& rhs )
        {
            mat< M, N, Type > result( *this );
            result *= rhs;
            return result;
        }
        inline void operator*=( const Type& rhs )
        {
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    this->data[ i ][ j ] *= rhs;
        }
        // the pointwise multiplication
        inline mat< M, N, Type > operator*( const mat< M, N, Type >& rhs )
        {
            mat< M, N, Type > result( *this );
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    result.data[ i ][ j ] *= rhs.data[ i ][ j ];
            return result;
        }
        // the ordinary matrix multiplication
        template < unsigned int K >
        inline mat< M, K, Type > operator*( const mat< N, K, Type >& rhs )
        {
            mat< M, K, Type > result;
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < K; ++j )
                    for ( unsigned int k = 0; k < N; ++k )
                        result.data[ i ][ j ] += data[ i ][ k ] * rhs.data[ k ][ j ];
            return result;
        }

        inline mat< M, N, Type > operator/( const Type& rhs )
        {
            mat< M, N, Type > result( *this );
            result /= rhs;
            return result;
        }
        // the pointwise division
        inline mat< M, N, Type > division( const mat< M, N, Type >& rhs )
        {
            mat< M, N, Type > result( *this );
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    result.data[ i ][ j ] /= rhs.data[ i ][ j ];
            return result;
        }
        // the ordinary matrix division: A / B = A * B^-1
        inline mat< M, N, Type > operator/( const mat< N, N, Type >& rhs )
        {
            mat< N, N, Type > inverse = rhs.inverse();
            return ( *this ) * inverse;
        }

        mat< M, N, Type > inverse( void )
        {
            if ( M != N )
                ERROR( "The inverse is only defined for square matrices." );
            if ( M == 0 )
                return mat< M, N, Type >();
            // calculate the inverse of a matrix by gsl
            gsl_matrix* gsl_mat = gsl_matrix_alloc( M, N );
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    gsl_matrix_set( gsl_mat, i, j, data[ i ][ j ] );
            gsl_permutation* p = gsl_permutation_alloc( M );
            int              s;
            gsl_linalg_LU_decomp( gsl_mat, p, &s );
            gsl_matrix* inverse = gsl_matrix_alloc( M, N );
            gsl_linalg_LU_invert( gsl_mat, p, inverse );
            mat< M, N, Type > result;
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    result.data[ i ][ j ] = gsl_matrix_get( inverse, i, j );
            gsl_permutation_free( p );
            gsl_matrix_free( gsl_mat );
            gsl_matrix_free( inverse );
            return result;
        }

        Type determinant( void )
        {
            // calculate the determinant of a matrix by gsl
            if ( M != N )
                ERROR( "The determinant is only defined for square matrices." );
            if ( M == 0 )
                return 0;
            gsl_matrix* gsl_mat = gsl_matrix_alloc( M, N );
            for ( unsigned int i = 0; i < M; ++i )
                for ( unsigned int j = 0; j < N; ++j )
                    gsl_matrix_set( gsl_mat, i, j, data[ i ][ j ] );
            gsl_permutation* p = gsl_permutation_alloc( M );
            int              s;
            gsl_linalg_LU_decomp( gsl_mat, p, &s );
            Type result = gsl_linalg_LU_det( gsl_mat, s );
            gsl_permutation_free( p );
            gsl_matrix_free( gsl_mat );
            return result;
        }

        Type det( void )  // alias of determinant
        {
            return this->determinant();
        }
    };

    template < unsigned int M, unsigned int N, typename Type >
    mat< M, N, Type > tensor_product( const vec< M, Type >& mat_A, const vec< N, Type >& mat_B )
    {
        mat< M, N, Type > result;
        for ( unsigned int i = 0; i < M; ++i )
            for ( unsigned int j = 0; j < N; ++j )
                result.data[ i ][ j ] = mat_A.data[ i ] * mat_B.data[ j ];
        return result;
    };

    enum stats_method { mean, median, std, max, min, sum, count };

    template < unsigned int Array_Len, unsigned int Bin_Num, typename Type >
    vec< Bin_Num, Type > bin1d( double ( &coord )[ Array_Len ], double ( &data )[ Array_Len ],
                                double lower_bound, double upper_bound, stats_method method )
    {
        if ( Bin_Num == 0 )
            ERROR( "The number of bins should be larger than 0." );
        vec< Bin_Num, Type > bins_1d( 0 );  // the final result

        // the index of the data points in each bin
        unsigned int bins[ Bin_Num ][ Array_Len ] = { 0 };
        // the counts of the data points in each bin
        unsigned int counts[ Bin_Num ] = { 0 };

        double range = upper_bound - lower_bound;

        // iterate over the data points
        for ( unsigned int i = 0; i < Array_Len; ++i )
        {
            // the index of the bin
            unsigned int bin_index =
                ( unsigned int )( ( coord[ i ] - lower_bound ) / range * Bin_Num );
            if ( bin_index == Bin_Num )
                bin_index = Bin_Num - 1;  // avoid the overflow at the upper bound

            bins[ bin_index ][ counts[ bin_index ]++ ] = i;
        }

        if ( method == stats_method::count )
        {
            for ( unsigned int i = 0; i < Bin_Num; ++i )
                bins_1d.data[ i ] = ( Type )counts[ i ];
        }
        else
        {
            double* data_points[ Bin_Num ];

            unsigned int i = 0, j = 0;
            for ( i = 0; i < Bin_Num; ++i )
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
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = nan( "" );
                    else
                        bins_1d.data[ i ] = ( Type )*std::min_element(
                            data_points[ i ], data_points[ i ] + counts[ i ] );
                }
                break;
            }
            case stats_method::max: {
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = nan( "" );
                    else
                        bins_1d.data[ i ] = ( Type )*std::max_element(
                            data_points[ i ], data_points[ i ] + counts[ i ] );
                }
                break;
            }
            case stats_method::mean: {
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = nan( "" );
                    else
                        bins_1d.data[ i ] =
                            ( Type )std::accumulate( data_points[ i ],
                                                     data_points[ i ] + counts[ i ], 0.0 )
                            / counts[ i ];
                }
                break;
            }
            case stats_method::median: {
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = nan( "" );
                    else
                    {
                        std::sort( data_points[ i ], data_points[ i ] + counts[ i ] );
                        if ( counts[ i ] % 2 == 0 )
                            bins_1d.data[ i ] = ( Type )( data_points[ i ][ counts[ i ] / 2 - 1 ]
                                                          + data_points[ i ][ counts[ i ] / 2 ] )
                                                / 2;
                        else
                            bins_1d.data[ i ] = ( Type )data_points[ i ][ counts[ i ] / 2 ];
                    }
                }
                break;
            }
            case stats_method::sum: {
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = 0;
                    else
                        bins_1d.data[ i ] = ( Type )std::accumulate(
                            data_points[ i ], data_points[ i ] + counts[ i ], 0.0 );
                }
                break;
            }
            case stats_method::std: {
                for ( i = 0; i < Bin_Num; ++i )
                {
                    if ( counts[ i ] == 0 )
                        bins_1d.data[ i ] = nan( "" );
                    else
                    {
                        double mean =
                            std::accumulate( data_points[ i ], data_points[ i ] + counts[ i ], 0.0 )
                            / counts[ i ];
                        double sum = 0;
                        for ( j = 0; j < counts[ i ]; ++j )
                            sum +=
                                ( data_points[ i ][ j ] - mean ) * ( data_points[ i ][ j ] - mean );
                        bins_1d.data[ i ] = ( Type )sqrt( sum / counts[ i ] );
                    }
                }
                break;
            }
            default:  // never reach here
                break;
            }
            for ( i = 0; i < Bin_Num; ++i )
                if ( counts[ i ] > 0 )
                    delete[] data_points[ i ];
        }
        return bins_1d;
    };

    template < unsigned int Array_Len, unsigned int Bin_Numx, unsigned int Bin_Numy, typename Type >
    mat< Bin_Numx, Bin_Numy, Type >
    bin2d( double ( &coord_x )[ Array_Len ], double ( &coord_y )[ Array_Len ],
           double ( &data )[ Array_Len ], double lower_bound_x, double upper_bound_x,
           double lower_bound_y, double upper_bound_y, stats_method method )
    {
        if ( Bin_Numx == 0 || Bin_Numy == 0 )
            ERROR( "The number of bins should be larger than 0." );

        mat< Bin_Numx, Bin_Numy, Type > bins_2d( 0 );  // the final result

        // the index of the data points in each bin
        unsigned int bins[ Bin_Numx ][ Bin_Numy ][ Array_Len ] = { 0 };
        // the counts of the data points in each bin
        unsigned int counts[ Bin_Numx ][ Bin_Numy ] = { 0 };

        double range_x = upper_bound_x - lower_bound_x;
        double range_y = upper_bound_y - lower_bound_y;

        // iterate over the data points
        for ( unsigned int i = 0; i < Array_Len; ++i )
        {
            // the index of the bin
            unsigned int bin_index_x =
                ( unsigned int )( ( coord_x[ i ] - lower_bound_x ) / range_x * Bin_Numx );
            unsigned int bin_index_y =
                ( unsigned int )( ( coord_y[ i ] - lower_bound_y ) / range_y * Bin_Numy );
            if ( bin_index_x == Bin_Numx )
                bin_index_x = Bin_Numx - 1;  // avoid the overflow at the upper bound
            if ( bin_index_y == Bin_Numy )
                bin_index_y = Bin_Numy - 1;  // avoid the overflow at the upper bound

            bins[ bin_index_x ][ bin_index_y ][ counts[ bin_index_x ][ bin_index_y ]++ ] = i;
        }

        if ( method == stats_method::count )
        {
            for ( unsigned int i = 0; i < Bin_Numx; ++i )
                for ( unsigned int j = 0; j < Bin_Numy; ++j )
                    bins_2d.data[ i ][ j ] = ( Type )counts[ i ][ j ];
        }
        else
        {
            double* data_points[ Bin_Numx ][ Bin_Numy ];

            unsigned int i = 0, j = 0, k = 0;
            for ( i = 0; i < Bin_Numx; ++i )
                for ( j = 0; j < Bin_Numy; ++j )
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
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = nan( "" );
                        else
                            bins_2d.data[ i ][ j ] = ( Type )*std::min_element(
                                data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ] );
                    }
                break;
            }
            case stats_method::max: {
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = nan( "" );
                        else
                            bins_2d.data[ i ][ j ] = ( Type )*std::max_element(
                                data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ] );
                    }
                break;
            }
            case stats_method::mean: {
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = nan( "" );
                        else
                            bins_2d.data[ i ][ j ] =
                                ( Type )std::accumulate( data_points[ i ][ j ],
                                                         data_points[ i ][ j ] + counts[ i ][ j ],
                                                         0.0 )
                                / counts[ i ][ j ];
                    }
                break;
            }
            case stats_method::median: {
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = nan( "" );
                        else
                        {
                            std::sort( data_points[ i ][ j ],
                                       data_points[ i ][ j ] + counts[ i ][ j ] );
                            if ( counts[ i ][ j ] % 2 == 0 )
                                bins_2d.data[ i ][ j ] =
                                    ( Type )( data_points[ i ][ j ][ counts[ i ][ j ] / 2 - 1 ]
                                              + data_points[ i ][ j ][ counts[ i ][ j ] / 2 ] )
                                    / 2;
                            else
                                bins_2d.data[ i ][ j ] =
                                    ( Type )data_points[ i ][ j ][ counts[ i ][ j ] / 2 ];
                        }
                    }
                break;
            }
            case stats_method::sum: {
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = 0;
                        else
                            bins_2d.data[ i ][ j ] = ( Type )std::accumulate(
                                data_points[ i ][ j ], data_points[ i ][ j ] + counts[ i ][ j ],
                                0.0 );
                    }
                break;
            }
            case stats_method::std: {
                for ( i = 0; i < Bin_Numx; ++i )
                    for ( j = 0; j < Bin_Numy; ++j )
                    {
                        if ( counts[ i ][ j ] == 0 )
                            bins_2d.data[ i ][ j ] = nan( "" );
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
                            bins_2d.data[ i ][ j ] = ( Type )sqrt( sum / counts[ i ][ j ] );
                        }
                    }
                break;
            }
            default:  // never reach here
                break;
            }
            for ( i = 0; i < Bin_Numx; ++i )
                for ( j = 0; j < Bin_Numy; ++j )
                    if ( counts[ i ][ j ] > 0 )
                        delete[] data_points[ i ][ j ];
        }
        return bins_2d;
    }
}  // namespace analysis
}  // namespace galotfa

#ifdef debug_utils
namespace unit_test {
int test_in_spheroid( void );
int test_in_box( void );
int test_in_cylinder( void );
int test_vec( void );
int test_mat( void );
int test_bin1d( void );
int test_bin2d( void );
}  // namespace unit_test
#endif
#endif
