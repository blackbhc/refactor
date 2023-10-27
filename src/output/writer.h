#ifndef __GALOTFA_WRITER_H__
#define __GALOTFA_WRITER_H__
#include <hdf5.h>
#include <string>
namespace galotfa {
hid_t create_h5( std::string path_to_file );
}  // namespace galotfa
#endif


#ifdef debug_output
namespace unit_test {
int test_create_h5( void );
}  // namespace unit_test
#endif
