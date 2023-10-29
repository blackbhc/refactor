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

#define OTF_STACK_STEP_SIZE = 1000
// the size of the stack for on-the-fly writing

namespace galotfa {

namespace hdf5 {
    node::node( hid_t id, NodeType type )
    {
        if ( type != NodeType::file )
            ERROR( "The 2 arguments constructor can only create root node for the file handle!" );
        this->self = id;
        this->type = type;
    }

    node::node( node* parent, hid_t id, NodeType type )
    {
        if ( type == NodeType::file )
            ERROR( "The 3 arguments constructor can not create root node for the file handle!" );
        this->self   = id;
        this->type   = type;
        this->parent = parent;
        parent->add_child( this );
    }

    void node::add_child( node* child )
    {
        this->children.push_back( child );
    }

    void node::close( void )
    {
        println( "There are %lld children", ( long long )this->children.size() );
        for ( auto& child : this->children )
            child->close();
        this->children.clear();

        // if the attribute, property or dataspace is not created, the handle is -1
        // if created, close it and set the handle to -1
        if ( this->attr != -1 )
        {
            H5Aclose( this->attr );
            this->attr = -1;
        }
        if ( this->prop != -1 )
        {
            H5Pclose( this->prop );
            this->prop = -1;
        }
        if ( this->space != -1 )
        {
            H5Sclose( this->space );
            this->space = -1;
        }
        if ( this->self != -1 )
            switch ( this->type )
            {
            case NodeType::file:
                H5Fclose( this->self );
                this->self = -1;
                break;
            case NodeType::group:
                H5Gclose( this->self );
                this->self = -1;
                break;
            case NodeType::dataset:
                H5Dclose( this->self );
                this->self = -1;
                break;
            default:
                WARN( "The closeed galotfa::hdf5::node target is uninitialized!" );
            }
    };

}  // namespace hdf5


writer::writer( std::string path_to_file )
{
#ifndef debug_output  // if not in debug mode: create file
    this->create_file( path_to_file );
#else  // if in debug mode: do nothing
    ( void )path_to_file;  // avoid unused variable warning
#endif
}

writer::~writer( void )
{
#ifndef debug_output  // if not in debug mode: close file
    // TODO: flush the buffer before closing the file
    nodes.at( "/" ).close();
#endif
}

int writer::create_file( std::string path_to_file )
{
    hid_t file_id = this->open_file( path_to_file );
    // check if the root node exists, this should never happen in normal cases
    if ( this->nodes.find( "/" ) != this->nodes.end() )
        ERROR( "The root node already exists!" );

    // insert the root node
    this->nodes.insert(
        std::pair< std::string, hdf5::node >( "/", hdf5::node( file_id, hdf5::NodeType::file ) ) );
    return 0;
}

inline hid_t writer::open_file( std::string path_to_file )
// split this part for
// 1. convenience of unit test and debug
// 2. clean code in the public API
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
        hid_t file_id = H5Fcreate( path_to_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
        return file_id;
    }
    catch ( ... )
    {
        WARN( "Failed to create file: %s", path_to_file.c_str() );
        return -1;
    }
}

int writer::create_group( std::string group_name )
{
    auto strings =
        galotfa::string::split( group_name, "/" );  // split the path into a string vector
    if ( strings.size() == 0 )
    {
        WARN( "The path to creat a hdf5 group is empty!" );
        return 1;  // if the path is empty, do nothing
    }
#ifdef debug_output
    // check whether the root node exists
    if ( this->nodes.find( "/" ) == this->nodes.end() )
    {
        WARN(
            "The root node does not exist, create it automatically with a filename=test.hdf5..." );
        this->create_file( "test.hdf5" );
    }
#endif
    std::string parent_path = "/";
    for ( size_t i = 0; i < strings.size(); ++i )
    {
        std::string this_path;
        if ( i == 0 )
            this_path = parent_path + strings[ i ];
        else
            this_path = parent_path + "/" + strings[ i ];

        if ( this->nodes.find( this_path ) != this->nodes.end() )  // if the link exists
        {
            if ( i == strings.size() - 1 )  // if be the last element, warn and return
            {
                WARN( "The group or dataset %s already exists!", group_name.c_str() );
                return 1;
            }
            else  // if not the last element, go to the next element
            {
                continue;
            }
        }
        else
        {
            // create the group
            hid_t group_id =
                H5Gcreate2( this->nodes.at( parent_path ).get_id(), strings[ i ].c_str(),
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
            // insert the node
            this->nodes.insert( std::pair< std::string, hdf5::node >(
                this_path, hdf5::node( &( this->nodes.at( parent_path ) ), group_id,
                                       hdf5::NodeType::group ) ) );
        }
        parent_path = this_path;
    }
    return 0;
}

int writer::create_dataset( std::string dataset_name, hdf5::data_info info )
{
    auto strings =
        galotfa::string::split( dataset_name, "/" );  // split the path into a string vector
    if ( strings.size() == 0 )
    {
        WARN( "The path to create a hdf5 dataset is empty!" );
        return 1;  // if the path is empty, do nothing
    }

#ifdef debug_output
    if ( this->nodes.find( "/" ) == this->nodes.end() )  // check whether the root node exists
    {
        WARN(
            "The root node does not exist, create it automatically with a filename=test.hdf5..." );
        this->create_file( "test.hdf5" );
    }
#endif

    // get the parent path
    std::string parent_path = "/";
    for ( size_t i = 0; i < strings.size() - 1; ++i )
    {
        if ( i == 0 )
            parent_path += strings[ i ];
        else
            parent_path += "/" + strings[ i ];
    }
    // check whether the dataset exists
    if ( this->nodes.find( parent_path + "/" + strings.back() ) != this->nodes.end() )
    {
        WARN( "The dataset %s already exists!", dataset_name.c_str() );
        return 1;
    }
    // check whether the parent node exists, if not, create it
    else if ( this->nodes.find( parent_path ) == this->nodes.end() )
        this->create_group( parent_path );

    // create the dataset
    hid_t data_id = open_dataset( this->nodes.at( parent_path ).get_id(), strings.back(), info );
    // insert the node
    this->nodes.insert( std::pair< std::string, hdf5::node >(
        parent_path + "/" + strings.back(),
        hdf5::node( &( this->nodes.at( parent_path ) ), data_id, hdf5::NodeType::dataset ) ) );
    return 0;
}

inline hid_t writer::open_dataset( hid_t group_id, std::string dataset, hdf5::data_info info )
{
    ( void )info;
    hid_t data_id = H5Dcreate2( group_id, dataset.c_str(), H5T_NATIVE_DOUBLE,
                                H5Screate( H5S_SCALAR ), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    // TODO: create the dataset according to the data_info
    return data_id;
}



#ifdef debug_output
int writer::test_node( void )
{
    // TODO: to be completed
    println( "Testing hdf5::node ..." );
    println( "Testing hdf5::node::node( hid_t id, NodeType type ) with non file type, it should "
             "raise a error ..." );
    try
    {
        auto node = new hdf5::node( -1, hdf5::NodeType::group );
        ( void )node;  // avoid unused variable warning
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise error as expected: %s", e.what() );
    }
    catch ( std::exception& e )
    {
        println( "It raise error unexpectedly: %s", e.what() );
        return 2;
    }

    hdf5::node node1( -1, hdf5::NodeType::file );
    bool       success = false;

    println( "Testing hdf5::node::node( node* parent_node, hid_t id, NodeType type ) with group "
             "type, it should not "
             "raise a error ..." );
    try
    {
        galotfa::hdf5::node node2( &node1, -1, hdf5::NodeType::group );
        success = ( node2.get_parent() == &node1 );
    }
    catch ( std::exception& e )
    {
        println( "It raise error unexpectedly: %s", e.what() );
        return 2;
    }

    println( "Testing hdf5::node::node( node* parent_node, hid_t id, NodeType type ) with file "
             "type, it should "
             "raise a error ..." );
    try
    {
        galotfa::hdf5::node node2( &node1, -1, hdf5::NodeType::file );
        success = ( node2.get_parent() == &node1 );
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise error as expected: %s", e.what() );
        success = success && true;
    }
    catch ( std::exception& e )
    {
        println( "It raise error unexpectedly: %s", e.what() );
        return 2;
    }

    CHECK_RETURN( success );
}

int writer::test_open_file()
{
    println( "Testing writer::open_file(std::string path_to_file) ..." );
    // test it can create a file
    std::string testfile = "test1.h5";
    hid_t       file_id  = this->open_file( testfile );
    H5Fclose( file_id );
    bool create_success = access( testfile.c_str(), F_OK ) == 0;
    if ( create_success )
        remove( testfile.c_str() );

    // test it can create a file with suffix
    std::string testfile2 = "test2.h5";
    FILE*       fp        = fopen( testfile2.c_str(), "w" );
    fclose( fp );
    file_id = this->open_file( testfile2 );
    H5Fclose( file_id );
    bool create_another_success = access( ( testfile2 + "-1" ).c_str(), F_OK ) == 0;
    if ( create_another_success )
        remove( ( testfile2 + "-1" ).c_str() );
    remove( testfile2.c_str() );

    bool success = create_success && create_another_success;

    CHECK_RETURN( success );
}

int writer::test_create_close()
{
    println(
        "Testing writer::create_file(std::string path_to_file) and writer::~writer(void) ..." );
    int  return_code    = this->create_file( "test.hdf5" );
    bool create_success = access( "test.hdf5", F_OK ) == 0 && return_code == 0;
    println( "All nodes:" );
    for ( auto& node : this->nodes )
        println( "node: %s", node.first.c_str() );
    bool insert_node_success = this->nodes.at( "/" ).is_root();

    nodes.at( "/" ).close();
    bool  close_success = true;
    hid_t file_id       = H5Fopen( "test.hdf5", H5F_ACC_RDONLY, H5P_DEFAULT );
    if ( file_id < 0 )
        close_success = false;
    H5Fclose( file_id );
    remove( "test.hdf5" );
    bool success = create_success && insert_node_success && close_success;
    if ( success )
        remove( "test.hdf5" );
    nodes.clear();
    CHECK_RETURN( success );
}

int writer::test_create_group()
{
    println( "Testing writer::create_group(std::string group_name) ..." );
    this->create_file( "test.hdf5" );  // create a test file

    // test it can create a root group
    int create_failure = this->create_group( "group1" );
    if ( create_failure )
        CHECK_RETURN( false );
    // close the file
    for ( auto& node : this->nodes )
        println( "node: [%s]", node.first.c_str() );
    println( "Reach here 1" );
    auto& node = this->nodes.at( "/group1" );
    println( "Reach here 2" );
    node.close();
    node = this->nodes.at( "/" );
    println( "Reach here 3" );
    node.close();
    // nodes.clear();
    println( "Reach here 4" );
    try
    {
        // open the file and check the group exists
        hid_t file_id  = H5Fopen( "test.hdf5", H5F_ACC_RDONLY, H5P_DEFAULT );
        hid_t group_id = H5Gopen2( file_id, "group1", H5P_DEFAULT );

        // close the file
        H5Gclose( group_id );
        H5Fclose( file_id );
    }
    catch ( ... )
    {
        CHECK_RETURN( false );
    }

    // // test it can create a subgroup
    // this->create_file( "test.hdf5" );  // create a test file
    // create_failure = this->create_group( "group1/group2" );
    // if ( create_failure )
    //     CHECK_RETURN( false );
    // // close the file
    // nodes.at( "/" ).close();
    // try
    // {
    //     // open the file and check the group exists
    //     hid_t file_id  = H5Fopen( "test.hdf5", H5F_ACC_RDONLY, H5P_DEFAULT );
    //     hid_t group_id = H5Gopen2( file_id, "group1/group2", H5P_DEFAULT );
    //
    //     // close the file
    //     H5Gclose( group_id );
    //     H5Fclose( file_id );
    // }
    // catch ( ... )
    // {
    //     CHECK_RETURN( false );
    // }
    //
    // if ( access( "test.hdf5", F_OK ) == 0 )
    //     remove( "test.hdf5" );
    //
    CHECK_RETURN( true );
}

#endif
}  // namespace galotfa
#endif
