#ifndef __GALOTFA_WRITER_H__
#define __GALOTFA_WRITER_H__
#include <hdf5.h>
#include <list>
#include <string>
#include <unordered_map>
namespace galotfa {
enum class H5DataType { scalar, vector, matrix, tensor };

class writer
{
    // public members
public:
    // private members
private:
    // list of handles: to be closed in destructor, for group, dataset, dataspace, property
    std::list< hid_t >                       group_handles;
    std::list< hid_t >                       dataset_handles;
    std::list< hid_t >                       dataspace_handles;
    std::list< hid_t >                       property_handles;
    hid_t                                    file_id = 0;
    std::unordered_map< std::string, hid_t > groups;
    std::unordered_map< std::string, hid_t > datasets;



    // public methods
public:
    writer( std::string path_to_file );
    ~writer( void );
    int   create_file( std::string path_to_file );
    int   create_group( std::string name );
    int   create_dataset( hid_t parent_id, std::string name );
    hid_t operator[]( std::string name );  // return a hdf5 handle from the map
#ifdef debug_output
    int test_create_file( void );
#endif

    // private methods
private:
};

}  // namespace galotfa
#endif
