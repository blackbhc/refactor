#ifndef __GALOTFA_WRITER_H__
#define __GALOTFA_WRITER_H__
#include <algorithm>
#include <hdf5.h>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>


namespace galotfa {

namespace hdf5 {
    enum class NodeType { group, dataset, file, uninitialized };
    class node
    // basic node for hdf5: group and dataset,
    // this class is used to organize the resources: dataspace, property and attribute
    {
        // private members
    private:
        hid_t    self = -1;
        NodeType type = NodeType::uninitialized;
        // node*                parent   = nullptr;
        std::vector< node* > children;
        node*                parent = nullptr;
        hid_t                attr   = -1;  // attribute
        hid_t                prop   = -1;  // property
        hid_t                space  = -1;  // dataspace
        // public methods
    public:
        node( hid_t id, NodeType type );
        node( node* paraent, hid_t id, NodeType type );
        node( node&& other ) noexcept;
        ~node( void );
        void         add_child( node* child );
        inline hid_t get_id( void ) const
        {
            return self;
        }
        void        close( void );
        inline bool is_root( void ) const
        {
            return ( this->parent == nullptr ) && ( this->type == NodeType::file );
        }
        inline bool is_file( void ) const
        {
            return type == NodeType::file;
        }
        inline bool is_group( void ) const
        {
            return type == NodeType::group;
        }
        inline bool is_dataset( void ) const
        {
            return type == NodeType::dataset;
        }
#ifdef debug_output
        node* get_parent( void ) const
        {
            return parent;
        }
#endif
    private:
        friend inline void shuffle( node& node );  // a friend function to shuffle the members
        friend inline void swap( node& lhs,
                                 node& rhs );  // a friend function to swap the children
    };
    // due to the node class is a cleaner for the hdf5 handle, its unit test is in the writer class

    // HACK: two dangerous functions, use with caution
    // if they are used, the node will be in an uninitialized state
    // so there may be memory leak if the node is not closed properly
    inline void
    shuffle( node& node );  // a friend function to shuffle the members to uninitialized state
    inline void swap( node& lhs, node& rhs );  // a friend function to swap the members


    struct data_info
    {
        unsigned int rank;
        hsize_t*     dims;
    };

}  // namespace hdf5

class writer
{
    // public members
public:
    // private members
private:
    std::unordered_map< std::string, hdf5::node > nodes = {};

    // public methods
public:
    writer( std::string path_to_file );
    ~writer( void );
    int create_file( std::string path_to_file );
    int create_group( std::string group_name );
    int create_dataset( std::string dataset_name, hdf5::data_info info );
    int add_attribute( std::string node_name, std::string attr_name, hid_t type, void* data );
    int push( void* ptr, std::string dataset_name );
#ifdef debug_output
    int test_open_file( void );
    int test_node( void );
    int test_create_close( void );
    int test_create_group( void );
    int test_create_dataset( void );
#endif
    // private methods
private:
    // the nake open functions, without any check, internal use only
    inline hid_t open_file( std::string path_to_file );
    inline hid_t open_dataset( hid_t group_id, std::string dataset, hdf5::data_info info );
};

}  // namespace galotfa
#endif
