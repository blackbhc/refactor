#include "ini-parser.h"
#include "../tools/prompt.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

namespace galotfa {
ini_parser::ini_parser( const char* file_name )
{
    this->filename = file_name;
    this->read( this->filename.c_str() );
}

void ini_parser::read( const char* filename )
{
    struct stat* st = new struct stat;
    stat( filename, st );
    this->check_filesize( st->st_size );

    // // read the file into one buffer
    // FILE* fp = fopen( filename, "rb" );
    // if ( fp == NULL )
    //     ERROR( "The file %s does not exist!\n", filename );
    //
    // char* buffer = new char[ st->st_size ];
    // fread( buffer, st->st_size, 1, fp );
    // // close the file
    // fclose( fp );
    //
    // char* token = strsep( &buffer, "\n" );
    // while ( token != NULL )
    // {
    //     // While there are tokens in "string"
    //     if ( *token == '\0' )
    //         ;
    //     else
    //     {
    //         std::string str( token );
    //         str.erase( 0, str.find_first_not_of( " " ) );
    //         if ( str[ 0 ] != '#' )
    //             println( " %s\n", str.c_str() );
    //     }
    //     // Get next token:
    //     token = strsep( &buffer, "\n" );
    // }
    //
    // delete[] buffer;
    delete st;
    // buffer = nullptr;
    st = nullptr;
}

void ini_parser::check_filesize( long int size )
{
    if ( size == 0 )
    {
        ERROR( "The ini file is empty." );
    }
    else if ( size >= 1024 * 1024 * 5 && size < 1024 * 1024 * 1024 )
    {
        WARN( "The ini file is too big: size = %ld MB.", size / 1024 / 1024 );
    }
    else if ( size >= 1024 * 1024 * 1024 )
    {
        ERROR( " The ini file is too big: size = %ld MB\nCheck whether the file is correct!",
               size / 1024 / 1024 );
    }
}

#ifdef debug_parameter
int ini_parser::test_check_size_works()
{
    println( "Testing ini_parser::check_filesize() ..." );
    try
    {
        println( "Check whethet it can distinguish a file with size 0:" );
        try
        {
            check_filesize( 0 );
        }
        catch ( std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s.", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!" );
            return 1;
        }

        try
        {
            println( "Check whethet it can distinguish a file with size 1GB:" );
            check_filesize( 1024 * 1024 * 1024 );
        }
        catch ( const std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s.", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!\n" );
            return 1;
        }

        try
        {
            println( "Check whethet it can distinguish a file with size 5MB:" );
            check_filesize( 1024 * 1024 * 5 );
        }
        catch ( const std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s.", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!\n" );
            return 1;
        }

        return 0;
    }
    catch ( const std::exception& e )
    {
        println( "It failed unexpectedly, error message: %s.", e.what() );
        return 1;
    }
}
#endif

}  // namespace galotfa
