#ifndef __GALOTFA_WRITER_CPP__
#define __GALOTFA_WRITER_CPP__
#include "writer.h"
#include "../tools/prompt.h"
#include "../tools/string.h"
#ifdef DO_UNIT_TEST
#include "../tools/string.cpp"
#endif
#include <hdf5.h>
#include <unistd.h>

namespace galotfa {
writer::writer( std::string path_to_file )
{
#ifndef debug_output  // if not in debug mode: create file
    this->create_file( path_to_file );
#else  // if in debug mode: do nothing
    ( void )path_to_file;
#endif
}

writer::~writer( void )
{
#ifndef debug_output  // if not in debug mode: close file
    for ( auto& property_handle : this->property_handles )
        H5Pclose( property_handle );
    for ( auto& dataset_handle : this->dataset_handles )
        H5Dclose( dataset_handle );
    for ( auto& dataspace_handle : this->dataspace_handles )
        H5Sclose( dataspace_handle );
    for ( auto& group_handle : this->group_handles )
        H5Gclose( group_handle );
    H5Fclose( this->file_id );
#endif
}

int writer::create_file( std::string path_to_file )
{
    try
    {
        if ( access( path_to_file.c_str(), F_OK ) == 0 )
        // if file exists
        {
            // add suffix to file name to avoid overwriting
            int i = 1;
            while ( access( ( path_to_file + "-" + std::to_string( i ) ).c_str(), F_OK ) == 0 )
            {
                ++i;
            }
            path_to_file += "-" + std::to_string( i );
        }
        this->file_id = H5Fcreate( path_to_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
        return 0;
    }
    catch ( ... )
    {
        WARN( "Failed to create file: %s", path_to_file.c_str() );
        return 1;
    }
}


int writer::create_group( std::string name )
{
    return 0;
}


#ifdef debug_output
int writer::test_create_file()
{
    // test it can create a file
    std::string testfile    = "test1.h5";
    int         return_code = this->create_file( testfile );
    hid_t       file_id     = this->file_id;
    H5Fclose( file_id );
    bool create_success = access( testfile.c_str(), F_OK ) == 0 && return_code == 0;
    if ( create_success )
        remove( testfile.c_str() );

    // test it can create a file with suffix
    std::string testfile2 = "test2.h5";
    FILE*       fp        = fopen( testfile2.c_str(), "w" );
    fclose( fp );
    return_code = this->create_file( testfile2 );
    file_id     = this->file_id;
    H5Fclose( file_id );
    bool create_another_success =
        access( ( testfile2 + "-1" ).c_str(), F_OK ) == 0 && return_code == 0;
    if ( create_another_success )
        remove( ( testfile2 + "-1" ).c_str() );
    remove( testfile2.c_str() );

    bool success = create_success && create_another_success;

    CHECK_RETURN( success );
}
#endif
}  // namespace galotfa
#endif
