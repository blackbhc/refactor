#ifndef GALOTFA_WRITER_CPP
#define GALOTFA_WRITER_CPP
#include "writer.h"
#include "../tools/prompt.h"
#include "../tools/string.h"
#ifdef DO_UNIT_TEST
#include "../tools/string.cpp"
#endif
#include <hdf5.h>
#include <unistd.h>

namespace galotfa {

namespace hdf5 {
    node::node( hid_t id, NodeType nodetype )
    {
        if ( nodetype != NodeType::file )
            ERROR( "The 2 arguments constructor can only create root node for the file handle!" );
        this->self = id;
        this->type = nodetype;
    }

    node::node( node* parent_ptr, hid_t id, NodeType nodetype )
    {
        if ( nodetype == NodeType::file )
            ERROR( "The 3 arguments constructor can not create root node for the file handle!" );
        this->self   = id;
        this->type   = nodetype;
        this->parent = parent_ptr;
        parent_ptr->add_child( this );
    }

    node::node( node&& other ) noexcept
    {
        hdf5::swap( *this, other );
        shuffle( other );
    }

    node::~node( void )
    {
        this->close();
    }
    inline void node::add_child( node* child )
    {
        this->children.push_back( child );
    }
    inline void node::remove_from_parent()
    {
        if ( this->parent != nullptr )  // only when the node has a parent
        {
            for ( size_t i = 0; i < this->parent->children.size(); ++i )
            {
                if ( this->parent->children.at( i ) == this )
                {
                    this->parent->children.erase( this->parent->children.begin() + ( long )i );
                    break;
                }
            }
        }
    }

    void node::close( void )
    {
        for ( auto& child : this->children )
            child->close();
        this->remove_from_parent();
        this->children.clear();

        // if the attribute, property or dataspace is not created, the handle is 0
        // if created, close it
        if ( this->prop != 0 )
            H5Pclose( this->prop );
        if ( this->dataspace != 0 )
            H5Sclose( this->dataspace );
        if ( this->memspace != 0 )
            H5Sclose( this->memspace );
        if ( this->self != 0 )
            switch ( this->type )
            {
            case NodeType::file:
                H5Fclose( this->self );
                break;
            case NodeType::group:
                H5Gclose( this->self );
                break;
            case NodeType::dataset:
                H5Dclose( this->self );
                if ( this->info != nullptr )
                {
                    delete this->info;
                    this->info = nullptr;
                }
                break;
            default:
                if ( this->type != NodeType::uninitialized )
                {
                    WARN( "Get a garbage value node: %ld at %p", this->self, ( void* )this );
                }
                else
                {
                    WARN(
                        "The closed galotfa::hdf5::node target (hid = %ld) at %p is uninitialized!",
                        this->self, ( void* )this );
                }
            }
        // shuffle the members to uninitialized state
        shuffle( *this );
    }

    inline void shuffle( node& node )
    {
        node.self      = 0;
        node.prop      = 0;
        node.dataspace = 0;
        node.memspace  = 0;
        node.type      = NodeType::uninitialized;
        node.parent    = nullptr;
        node.children.clear();
        node.dim_ext.clear();
        node.info = nullptr;
    };

    // dangerous: this should be used only for move constructor
    inline void swap( node& lhs, node& rhs )  // a friend function to swap the members
    {
        std::swap( lhs.self, rhs.self );
        std::swap( lhs.prop, rhs.prop );
        std::swap( lhs.parent, rhs.parent );
        std::swap( lhs.dataspace, rhs.dataspace );
        std::swap( lhs.memspace, rhs.memspace );
        std::swap( lhs.type, rhs.type );
        std::swap( lhs.children, rhs.children );
        std::swap( lhs.dim_ext, rhs.dim_ext );
        if ( lhs.is_dataset() || rhs.is_dataset() )
            std::swap( lhs.info, rhs.info );
    };

}  // namespace hdf5


writer::writer( std::string path_to_file )
{
#ifndef debug_output  // if not in debug mode: create file
    this->create_file( path_to_file );
    this->filename = path_to_file;
#else  // if in debug mode: do nothing
    ( void )path_to_file;  // avoid unused variable warning
#endif
}

inline void writer::clean_nodes( void )
{
    for ( auto& node : this->nodes )
    {
        if ( node.second != nullptr )
        {
            delete node.second;
            node.second = nullptr;
        }
    }
    nodes.clear();
}

writer::~writer( void )
{
    // TODO: flush the buffer before closing the file
    clean_nodes();
    this->stack_counter.clear();
}

int writer::create_file( std::string path_to_file )
{
    hid_t file_id = this->open_file( path_to_file );
    if ( file_id == 0 )
    {
        ERROR( "Failed to create file: %s", path_to_file.c_str() );
        return 1;
    }
    // check if the root node exists, this should never happen in normal cases
    if ( this->nodes.find( "/" ) != this->nodes.end() )
    {
        H5Fclose( file_id );
        ERROR( "The root node already exists!" );
    }

    // insert the root node
    auto root = new galotfa::hdf5::node( file_id, hdf5::NodeType::file );
    this->nodes.insert( std::pair< std::string, hdf5::node* >( "/", root ) );
    return 0;
}

inline hid_t writer::open_file( std::string path_to_file )
// sperate this part for
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
        return 0;
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
        INFO(
            "The root node does not exist, create it automatically with a filename=test.hdf5..." );
        this->create_file( "test.hdf5" );
    }
#endif
    std::string parent_path = "/";
    for ( size_t i = 0; i < strings.size(); ++i )
    {
        std::string this_path;
        this_path = parent_path + "/" + strings[ i ];
        if ( i == 0 )
            this_path.erase( 0, 1 );  // remove the additional first "/"

        if ( this->nodes.find( this_path ) != this->nodes.end() )  // if the link exists
        {
            if ( i == strings.size() - 1 )  // if be the last element, warn and return
            {
                WARN( "The group or dataset %s is already in use in file %s!", group_name.c_str(),
                      this->filename.c_str() );
                return 1;
            }
            else  // if not the last element, go to the next element
            {
                parent_path = this_path;  // update the parent path
                continue;
            }
        }
        else
        {
            // create the group
            hid_t group_id =
                H5Gcreate2( this->nodes.at( parent_path )->get_hid(), strings[ i ].c_str(),
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
            // insert the node
            auto group_node_ptr =
                new galotfa::hdf5::node( this->nodes.at( parent_path ), group_id,
                                         hdf5::NodeType::group );  // create a new node
            auto pair = std::pair< std::string, hdf5::node* >(
                this_path,
                group_node_ptr );  // create a pair of key-value to insert into the nodes map
            this->nodes.insert( pair );
            parent_path = this_path;
        }
    }
    return 0;
}

int writer::create_dataset( std::string dataset_name, hdf5::size_info& info )
{
    // first check the size of the size_info is consistent
    if ( ( size_t )info.rank != info.dims.size() )
        ERROR( "The rank (%d) != the size of the dims vector (%lu)!\n", info.rank,
               info.dims.size() );

    auto strings =
        galotfa::string::split( dataset_name, "/" );  // split the path into a string vector
    if ( strings.size() == 0 )
    {
        WARN( "The path of the creating hdf5 dataset is empty!" );
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
    std::string parent_path = "";
    for ( size_t i = 0; i < strings.size() - 1; ++i )
        parent_path += "/" + strings[ i ];

    // check whether there are group/dataset with the same name
    if ( this->nodes.find( parent_path + "/" + strings.back() ) != this->nodes.end() )
    {
        WARN( "The name %s is already in use in file %s!", dataset_name.c_str(),
              this->filename.c_str() );
        return 1;
    }
    // check whether the parent node exists, if not, create it
    else if ( this->nodes.find( parent_path ) == this->nodes.end() && parent_path != "" )
        this->create_group( parent_path );

    // create the dataset
    if ( parent_path == "" )
        parent_path = "/";  // avoid the empty string
    auto data_node_ptr = create_datanode( *this->nodes.at( parent_path ), strings.back(), info );
    // insert the node
    this->nodes.insert( std::pair< std::string, galotfa::hdf5::node* >(
        parent_path + "/" + strings.back(), data_node_ptr ) );
    return 0;
}

inline hdf5::node* writer::create_datanode( hdf5::node& parent, std::string& dataset,
                                            hdf5::size_info& info )
{
    // check the size of the size_info is consistent: done in the caller-function create_file(
    // ...)
    /* head of the caller:
    // first check the size of the size_info is consistent
    if ( ( size_t )info.rank != info.dims.size() )
        ERROR( "The rank != the size of the dims vector!\n" );
    */

    // check whther the parent is a group or file
    if ( !parent.is_group() && !parent.is_file() )
        ERROR( "Try to create a dataset in a non-group or root node!" );

    // set the chunk size and compression at here
    hsize_t chunk_dims[ info.rank + 1 ];
    hsize_t max_dims[ info.rank + 1 ];
    hsize_t data_dims[ info.rank + 1 ];  // only for the data space creation
    for ( size_t i = 0; i < info.rank; ++i )
        data_dims[ i + 1 ] = max_dims[ i + 1 ] = chunk_dims[ i + 1 ] = info.dims[ i ];
    data_dims[ 0 ]  = 0;
    chunk_dims[ 0 ] = VIRTUAL_STACK_SIZE;
    max_dims[ 0 ]   = H5S_UNLIMITED;

    // create property list and set chunk and compression
    hid_t  prop_list = H5Pcreate( H5P_DATASET_CREATE );
    herr_t status    = H5Pset_chunk( prop_list, ( int )info.rank + 1, chunk_dims );
    status           = H5Pset_deflate( prop_list, 6 );
    if ( status < 0 )
        ERROR( "Failed to set deflate!" );

    // create the zero-size dataspace
    hid_t dataspace = H5Screate_simple( ( int )info.rank + 1, data_dims, max_dims );

    hid_t dataset_id = H5Dcreate2( parent.get_hid(), dataset.c_str(), info.data_type, dataspace,
                                   H5P_DEFAULT, prop_list, H5P_DEFAULT );
    // create the dataset with 0 size data
    auto datanode_ptr = new galotfa::hdf5::node( &parent, dataset_id, hdf5::NodeType::dataset );
    datanode_ptr->set_hid( dataset_id );
    datanode_ptr->set_property( prop_list );
    datanode_ptr->set_dataspace( dataspace );
    data_dims[ 0 ] = 1;
    hid_t memspace = H5Screate_simple( ( int )info.rank + 1, data_dims, NULL );
    datanode_ptr->set_memspace( memspace );
    std::vector< hsize_t > dim_ext( data_dims, data_dims + ( int )info.rank + 1 );
    datanode_ptr->set_dim_ext( dim_ext );
    datanode_ptr->set_size_info( info );
    return datanode_ptr;
}

template < typename T > int writer::push( T* ptr, unsigned long len, std::string dataset_name )
{
    // check whether the dataset exists
    if ( this->nodes.find( dataset_name ) == this->nodes.end() )
    {
        WARN( "Try to push data into unexist dataset: %s", dataset_name.c_str() );
        return 1;
    }
    else if ( !this->nodes.at( dataset_name )->is_dataset() )
    {
        WARN( "The target is not a dataset: %s", dataset_name.c_str() );
        return 1;
    }
    else
    {
        unsigned int target_len = 1;
        auto         dims       = this->nodes.at( dataset_name )->get_dim_ext();
        for ( size_t i = 1; i < dims.size(); ++i )
            target_len *= dims[ i ];
        if ( target_len != len )
        {
            WARN( "The length of the target dataset different (%d) is different from the input "
                  "(%lu)!",
                  target_len, len );
            return 1;
        }
    }

    if ( this->stack_counter.find( dataset_name ) == this->stack_counter.end() )
    {
        stack_counter[ dataset_name ] = 1;
        // if it is the first push, initialize the stack counter
    }

    /* // get the filespace
    hid_t dataset   = this->nodes.at( dataset_name ).get_hid();
    hid_t filespace = H5Dget_space( dataset );

    // extend the dataset
    // get the data info from the node
    hdf5::size_info* info    = this->nodes.at( dataset_name ).get_size_info();
    auto&            datadim = info->dims;
    hsize_t          new_sizes[ datadim.size() + 1 ];
    hsize_t          hyperslab[ datadim.size() + 1 ];
    hyperslab[ 0 ] = 1;
    new_sizes[ 0 ] = ( size_t )stack_counter[ dataset_name ];
    for ( size_t i = 0; i < datadim.size(); ++i )
        hyperslab[ i + 1 ] = new_sizes[ i + 1 ] = datadim[ i ];
    // herr_t status = H5Dset_extent( dataset, new_sizes ); */

    // select the hyperslab, namely a subset of the dataset
    hdf5::size_info* info       = this->nodes.at( dataset_name )->get_size_info();
    auto             dims       = this->nodes.at( dataset_name )->get_dim_ext();
    hid_t            dataset_id = this->nodes.at( dataset_name )->get_hid();
    auto             rank       = dims.size();
    hsize_t*         offset     = new hsize_t[ rank ]();
    offset[ 0 ]                 = stack_counter[ dataset_name ] - 1;
    hid_t memspace              = this->nodes.at( dataset_name )->get_memspace();

    dims[ 0 ]     = stack_counter[ dataset_name ];
    herr_t status = H5Dset_extent( dataset_id, dims.data() ); /* extend the dataset */
    dims[ 0 ]     = 1;  // reset the first dimension to 1 for hyperslab selection

    // get file space
    hid_t filespace = H5Dget_space( dataset_id );
    // get the hyperslab, namely a subset of the dataset
    status = H5Sselect_hyperslab( filespace, H5S_SELECT_SET, offset, NULL, dims.data(), NULL );
    // call the write function, its only a API without flush buffer
    status = H5Dwrite( dataset_id, info->data_type, memspace, filespace, H5P_DEFAULT, ptr );
    H5Sclose( filespace );

    delete[] offset;

    // flush the buffer if the stack is "full"
    if ( stack_counter[ dataset_name ] % VIRTUAL_STACK_SIZE == 0 )
        H5Dflush( this->nodes.at( dataset_name )->get_hid() );

    ++stack_counter[ dataset_name ];
    if ( status < 0 )
    {
        WARN( "Failed to push data into dataset: %s", dataset_name.c_str() );
        return 1;
    }
    return 0;
}

// ensure the template function is instantiated
template int writer::push< int >( int* ptr, unsigned long len, std::string dataset_name );
template int writer::push< double >( double* ptr, unsigned long len, std::string dataset_name );
template int writer::push< unsigned int >( unsigned int* ptr, unsigned long len,
                                           std::string dataset_name );

#ifdef debug_output
int writer::test_node( void )
{
    println( "Testing hdf5::node ..." );
    println( "The size of hdf5::node is %lu", sizeof( hdf5::node ) );
    println( "Testing hdf5::node::node( hid_t id, NodeType type ) with non file type, it should "
             "raise an error ..." );
    try
    {
        auto node = new hdf5::node( 0, hdf5::NodeType::group );
        ( void )node;  // avoid unused variable warning
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise an error as expected: %s", e.what() );
    }
    catch ( std::exception& e )
    {
        println( "It raise an error unexpectedly: %s", e.what() );
        return 2;
    }

    hdf5::node node1( 0, hdf5::NodeType::file );
    bool       success = false;

    println( "Testing hdf5::node::node( node* parent_node, hid_t id, NodeType type ) with group "
             "type, it should not "
             "raise an error ..." );
    try
    {
        galotfa::hdf5::node node2( &node1, 0, hdf5::NodeType::group );
        success = ( node2.get_parent() == &node1 );
    }
    catch ( std::exception& e )
    {
        println( "It raise an error unexpectedly: %s", e.what() );
        return 2;
    }

    println( "Testing hdf5::node::node( node* parent_node, hid_t id, NodeType type ) with file "
             "type, it should "
             "raise an error ..." );
    try
    {
        galotfa::hdf5::node node2( &node1, 0, hdf5::NodeType::file );
        success = ( node2.get_parent() == &node1 );
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise an error as expected: %s", e.what() );
        success = success && true;
    }
    catch ( std::exception& e )
    {
        println( "It raise an error unexpectedly: %s", e.what() );
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
    {
        remove( testfile.c_str() );
    }
    else
    {
        CHECK_RETURN( false );
    }

    // test it can create a file with suffix
    std::string testfile2 = "test2.h5";
    FILE*       fp        = fopen( testfile2.c_str(), "w" );
    fclose( fp );
    file_id = this->open_file( testfile2 );
    H5Fclose( file_id );
    bool create_another_success = access( ( testfile2 + "-1" ).c_str(), F_OK ) == 0;
    if ( create_another_success )
    {
        remove( ( testfile2 + "0" ).c_str() );
    }
    else
    {
        CHECK_RETURN( false );
    }
    remove( testfile2.c_str() );


    CHECK_RETURN( true );
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
    bool insert_node_success = this->nodes.at( "/" )->is_root();
    clean_nodes();  // clean the nodes vector

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
    std::string testfile   = "test.hdf5";
    std::string testgroup1 = "/group1";
    std::string testgroup2 = "/group1/group2/group3";

    // test it can create a file and the "/" node
    println( "Testing it can create a root group \"/\" ..." );
    // ensure the file does not exist
    if ( access( "test.hdf5", F_OK ) == 0 )
        remove( "test.hdf5" );

    int create_file_failure = this->create_file( testfile );  // create a test file
    clean_nodes();

    if ( create_file_failure )
        CHECK_RETURN( false );
    try
    {
        // open the file and check the group exists
        hid_t file_id = H5Fopen( testfile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        H5Fclose( file_id );
        remove( testfile.c_str() );  // clean up
    }
    catch ( ... )
    {
        WARN( "The file does not exist!" );
        CHECK_RETURN( false );
    }
    // check the nodes vector is empty
    if ( !nodes.empty() )
    {
        WARN( "The nodes vector is not empty!" );
        CHECK_RETURN( false );
    }

    // test the group can be created
    println( "Testing it can create a first level group ..." );
    create_file_failure      = this->create_file( testfile );  // create a test file
    int create_group_failure = this->create_group( testgroup1 );
    clean_nodes();
    // check the nodes vector is empty
    if ( !nodes.empty() )
    {
        WARN( "The nodes vector is not empty!" );
        CHECK_RETURN( false );
    }
    if ( create_file_failure || create_group_failure )
        CHECK_RETURN( false );
    try
    {
        // open the file and check the group exists
        hid_t file_id  = H5Fopen( testfile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        hid_t group_id = H5Gopen2( file_id, testgroup1.c_str(), H5P_DEFAULT );

        // close the file
        H5Gclose( group_id );
        H5Fclose( file_id );
        remove( testfile.c_str() );  // clean up
    }
    catch ( std::exception& e )
    {
        WARN( "The group does not exist! Error: %s", e.what() );
        CHECK_RETURN( false );
    }


    // test it can create groups with multiple levels
    println( "Testing it can create groups with multiple levels ..." );
    create_file_failure  = this->create_file( testfile );  // create a test file
    create_group_failure = this->create_group( testgroup2 );
    clean_nodes();
    // check the nodes vector is empty
    if ( !nodes.empty() )
    {
        WARN( "The nodes vector is not empty!" );
        CHECK_RETURN( false );
    }
    if ( create_file_failure || create_group_failure )
        CHECK_RETURN( false );
    try
    {
        hid_t file_id  = H5Fopen( testfile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        hid_t group_id = H5Gopen2( file_id, testgroup2.c_str(), H5P_DEFAULT );  // open the group
        H5Gclose( group_id );
        H5Fclose( file_id );
        // remove( testfile.c_str() );  // clean up
    }
    catch ( const std::exception& e )
    {
        WARN( "Test failed with error: %s", e.what() );
        CHECK_RETURN( false );
    }

    CHECK_RETURN( true );
}

int writer::test_create_dataset( void )
{
    println( "Testing writer::create_dataset(std::string dataset_name, hdf5::size_info info) ..." );

    std::string testfile  = "test.hdf5";
    std::string testset1  = "/data";
    std::string testset2  = "/group/data";
    std::string testgroup = "/group/data";  // for error test

    hdf5::size_info info{ H5T_NATIVE_INT, 1, { 1 } };  // create a size_info object

    // ensure the test file does not exist
    if ( access( testfile.c_str(), F_OK ) == 0 )
        remove( testfile.c_str() );

    int create_file_failure = this->create_file( testfile );  // create the test file
    if ( create_file_failure )
        CHECK_RETURN( false );

    try
    {
        println( "Testing it can create a dataset in \"/\" ..." );
        int create_fail = this->create_dataset( testset1, info );
        if ( create_fail )
            CHECK_RETURN( false );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }
    // clean up
    clean_nodes();

    try
    {
        hid_t file_id    = H5Fopen( testfile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        hid_t dataset_id = H5Dopen2( file_id, testset1.c_str(), H5P_DEFAULT );  // open the dataset
        H5Dclose( dataset_id );
        H5Fclose( file_id );
        remove( testfile.c_str() );  // clean up
    }
    catch ( std::exception& e )
    {
        WARN( "The dataset does not exist! Error: %s", e.what() );
        CHECK_RETURN( false );
    }

    try
    {
        // test it can create a dataset in a group
        println( "Testing it can create a dataset in a group ..." );
        int create_fail = this->create_file( testfile );
        create_fail += this->create_dataset( testset2, info );
        if ( create_fail )
            CHECK_RETURN( false );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }
    // clean up
    clean_nodes();

    try
    {
        // read the file and check the dataset exists
        hid_t file_id    = H5Fopen( testfile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        hid_t group_id   = H5Gopen2( file_id, "/group", H5P_DEFAULT );          // open the group
        hid_t dataset_id = H5Dopen2( file_id, testset2.c_str(), H5P_DEFAULT );  // open the dataset
        H5Dclose( dataset_id );
        H5Gclose( group_id );
        H5Fclose( file_id );
        remove( testfile.c_str() );  // clean up
    }
    catch ( std::exception& e )
    {
        WARN( "The dataset does not exist! Error: %s", e.what() );
        CHECK_RETURN( false );
    }

    println( "Test wrong path to create dataset, it should raise a Warning ..." );
    try
    {
        int create_fail = this->create_file( testfile );
        create_fail += this->create_group( testgroup );
        create_fail += this->create_dataset( testset2, info );
        if ( create_fail != 1 )
            CHECK_RETURN( false );
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise an error as expected: %s", e.what() );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }
    // clean up
    clean_nodes();

    CHECK_RETURN( true );
}

int writer::test_push( void )
{
    println( "Testing writer::push(T* ptr, unsigned int len, std::string dataset_name) ..." );
    std::string testfile = "test.hdf5";
    std::string testset2 = "/group/data";

    // ensure the test file does not exist
    if ( access( testfile.c_str(), F_OK ) == 0 )
        remove( testfile.c_str() );
    // ensure the nodes is clean
    nodes.clear();
    // create the test file and test dataset
    int create_failure = this->create_file( testfile );  // create the test file
    create_failure += this->create_group( "/group" );

    hdf5::size_info info{ H5T_NATIVE_DOUBLE, 1, { 3 } };  // create a size_info object
    create_failure += this->create_dataset( "/group/data", info );
    if ( create_failure )
        CHECK_RETURN( false );

    // push data one-by-one
    println( "Testing it can push data one by one ..." );
    std::vector< double > vec{ 3, 4, 5 };
    try
    {
        int push_failure = 0;
        for ( int i = 0; i < 7; ++i )
        {
            push_failure += this->push( vec.data(), vec.size(), testset2 );
            for ( auto& v : vec )
                v += 1;
        }
        if ( push_failure )
            CHECK_RETURN( false );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        remove( testfile.c_str() );
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }
    try
    {
        println( "Try to push into a non-exist dataset, it should raise a warning ..." );
        int push_failure = this->push( vec.data(), vec.size(), "/group/data2" );
        if ( push_failure != 1 )
            CHECK_RETURN( false );
    }
    catch ( std::runtime_error& e )
    {
        println( "It raise an error as expected: %s", e.what() );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        remove( testfile.c_str() );
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }

    // clean up
    clean_nodes();
    remove( testfile.c_str() );

    // test it can push to different datasets
    println( "Testing it can push to different datasets ..." );
    create_failure = this->create_file( testfile );  // create the test file
    // create test info
    hdf5::size_info info_scalar{ H5T_NATIVE_INT, 1, { 1 } };        // 1 int scalar
    hdf5::size_info info_vector{ H5T_NATIVE_DOUBLE, 2, { 1, 3 } };  // 1x3 double vector
    hdf5::size_info info_matrix{ H5T_NATIVE_DOUBLE, 2, { 4, 4 } };  // 4x4 double matrix
    create_failure += this->create_dataset( "/group/scalar", info_scalar );
    create_failure += this->create_dataset( "/group/com", info_vector );
    create_failure += this->create_dataset( "/group/image", info_matrix );
    int steps = 10;  // mock 10 synchronization steps
    // create the mock analysis results
    std::vector< int >    step( 1 );
    std::vector< double > com{ 3.1, 4.1, 5.9 };
    std::vector< double > image( 16, 1.1 );
    try
    {
        int push_failure = 0;
        for ( int i = 0; i < 5; ++i )
        {
            step[ 0 ] += i;
            push_failure += this->push( step.data(), step.size(), "/group/scalar" );
        }  // push 5 steps
        if ( push_failure )
            CHECK_RETURN( false );

        for ( int i = 0; i < steps; ++i )
        {
            step[ 0 ] += i;
            com[ 0 ] += i;
            com[ 1 ] += i;
            com[ 2 ] += i;
            for ( auto& pixel : image )
                pixel *= ( double )( i + 0.1 );
            push_failure += this->push( com.data(), com.size(), "/group/com" );
            push_failure += this->push( image.data(), image.size(), "/group/image" );
        }
        if ( push_failure )
            CHECK_RETURN( false );
    }
    catch ( std::exception& e )
    {
        clean_nodes();
        remove( testfile.c_str() );
        WARN( "Encounter unexpected error: %s", e.what() );
        CHECK_RETURN( false );
    }

    clean_nodes();
    // remove( testfile.c_str() );
    CHECK_RETURN( true );
}
#endif
}  // namespace galotfa
#endif
