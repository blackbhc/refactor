#ifndef __GALOTFA_WRITER_CPP__
#define __GALOTFA_WRITER_CPP__
#include "writer.h"
#include <hdf5.h>
#include <stdio.h>
#include <unistd.h>

namespace galotfa {
hid_t create_h5( std::string path_to_file )
{
    hid_t file_id = -1;
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
    file_id = H5Fcreate( path_to_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
    return file_id;
}
}  // namespace galotfa

#ifdef debug_output
namespace unit_test {
int test_create_h5()
{
    // test it can create a file
    std::string testfile = "test1.h5";
    hid_t       file_id  = galotfa::create_h5( testfile );
    H5Fclose( file_id );
    bool create_success = access( testfile.c_str(), F_OK ) == 0;
    if ( create_success )
        remove( testfile.c_str() );

    // test it can create a file with suffix
    std::string testfile2 = "test2.h5";
    FILE*       fp        = fopen( testfile2.c_str(), "w" );
    fclose( fp );
    file_id = galotfa::create_h5( testfile2 );
    H5Fclose( file_id );
    bool create_another_success = access( ( testfile2 + "-1" ).c_str(), F_OK ) == 0;
    if ( create_another_success )
        remove( ( testfile2 + "-1" ).c_str() );
    remove( testfile2.c_str() );

    bool success = create_success && create_another_success;

    CHECK_RETURN( success );
}
}  // namespace unit_test
#endif
#endif
