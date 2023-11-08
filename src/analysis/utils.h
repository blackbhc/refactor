#ifndef GALOTFA_ANALYSIS_UNTILS_H
#define GALOTFA_ANALYSIS_UNTILS_H
#include "../tools/prompt.h"
#include <initializer_list>
#include <math.h>
namespace galotfa {
namespace analysis {
    bool in_spheroid( double ( &pos )[ 3 ], double& size, double& ratio );

    bool in_box( double ( &pos )[ 3 ], double& size, double& ratio );

    bool in_cylinder( double ( &pos )[ 3 ], double& size, double& ratio );

    double norm( double ( &vec )[ 3 ] );

    // the vector class: support ordinary operations
    template < unsigned int N, typename T > struct vec
    {
        T data[ N ] = { 0 };

        inline T norm( void )
        {
            T sum = 0;
            for ( unsigned int i = 0; i < N; ++i )
                sum += data[ i ] * data[ i ];
            return sqrt( sum );
        }

        void normalize( void )
        {
            if ( this->norm() > 0 )
                ( *this ) /= norm();
        }

        inline unsigned int length( void )
        {
            return N;
        }

        vec( void ) {}

        // the constructor from an initializer list
        vec( std::initializer_list< T > list )
        {
            if ( list.size() != N )
                ERROR( "The size of the initializer list is not equal to the dimension of the "
                       "vector." );
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = *( list.begin() + i );
        }

        // the constructor from an array
        vec( T ( &array )[ N ] )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = array[ i ];
        }

        // the constructor from an non-initialised array
        vec( T number )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = number;
        }

        // the copy constructor
        vec( const vec< N, T >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                data[ i ] = rhs.data[ i ];
        }


        inline const T& operator[]( unsigned int i ) const
        {
            return data[ i ];
        }

        inline vec< N, T >& operator=( const T ( &array )[ N ] )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = array[ i ];
            return *this;
        }
        inline vec< N, T >& operator=( const vec< N, T >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] = rhs.data[ i ];
            return *this;
        }

        inline vec< N, T > operator+( const vec< N, T >& rhs ) const
        {
            vec< N, T > result;
            for ( unsigned int i = 0; i < N; ++i )
                result.data[ i ] = data[ i ] + rhs.data[ i ];
            return result;
        }
        inline vec< N, T >& operator+=( const vec< N, T >& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] += rhs.data[ i ];
            return *this;
        }
        inline vec< N, T >& operator+=( const T& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] += rhs;
            return *this;
        }

        inline vec< N, T > operator-( const vec< N, T >& rhs ) const
        {
            vec< N, T > result;
            for ( unsigned int i = 0; i < N; ++i )
                result.data[ i ] = data[ i ] - rhs.data[ i ];
            return result;
        }
        inline vec< N, T >& operator-=( const T& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] -= rhs;
            return *this;
        }

        inline T operator*( const vec< N, T >& rhs ) const
        {
            T result = 0;
            for ( unsigned int i = 0; i < N; ++i )
                result += data[ i ] * rhs.data[ i ];
            return result;
        }
        inline vec< N, T >& operator*=( const T& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] *= rhs;
            return *this;
        }

        inline vec< N, T > operator/( const T& rhs ) const
        {
            vec< N, T > result( *this );
            result /= rhs;
            return result;
        }
        inline vec< N, T >& operator/=( const T& rhs )
        {
            for ( unsigned int i = 0; i < N; ++i )
                this->data[ i ] /= rhs;
            return *this;
        }

        inline bool operator==( const vec< N, T >& rhs ) const
        {
            auto error = this->operator-( rhs );
            return error.norm() < 1e-12;
            // the numerical error is 1e-12
        }
        inline bool operator!=( const vec< N, T >& rhs ) const
        {
            return !( this->operator==( rhs ) );
        }
    };
}  // namespace analysis
}  // namespace galotfa

#ifdef debug_utils
namespace unit_test {
int test_in_spheroid( void );
int test_in_box( void );
int test_in_cylinder( void );
int test_vec( void );
}  // namespace unit_test
#endif
#endif
