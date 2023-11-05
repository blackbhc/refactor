#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace galotfa {
monitor::monitor( void )
{
    // check and ensure MPI is initialized
    int  initialized = 0;
    auto status      = MPI_Initialized( &initialized );
    if ( status != MPI_SUCCESS )
    {
        ERROR( "Failed to check MPI initialization status." );
    }
    else if ( !initialized )
        MPI_Init( NULL, NULL );

    // get the rank and size of the MPI process
    MPI_Comm_rank( MPI_COMM_WORLD, &( this->galotfa_rank ) );
    MPI_Comm_size( MPI_COMM_WORLD, &( this->galotfa_size ) );

    // read the configuration file
    galotfa::ini_parser ini( "./galotfa.ini" );
    this->para = new galotfa::para( ini );
    if ( this->para->glb_switch_on )
    {
        this->init();  // create the output directory and the output files
        // create and start the virtual engine's calculator
        this->engine = new galotfa::calculator( *( this->para ) );
        this->engine->start();
    }
}

void monitor::init()
{
    if ( this->is_root() )
    {
        auto out_dir = this->para->glb_output_dir.c_str();
        if ( access( out_dir, F_OK ) != 0 )
        {
            INFO( "The output directory does not exist, create it." );
            int status = mkdir( out_dir, 0755 );
            if ( status != 0 )
            {
                ERROR( "Failed to create the output directory: %s", out_dir );
            }
        }
        this->create_writers();
    }
}

monitor::~monitor()
{
    if ( this->engine->is_active() )
        this->engine->stop();

    if ( this->para != nullptr )
    {
        delete this->para;
        this->para = nullptr;
    }

    if ( this->engine != nullptr )
    {
        delete this->engine;
        this->engine = nullptr;
    }

    // free the writers pointers
    for ( auto& writer_of_single_set : this->writers )
        for ( auto& w : writer_of_single_set )
            if ( w != nullptr )
            {
                delete w;
                w = nullptr;
            }
    this->writers.clear();

    // free the orbit writer pointer
    if ( this->orbit_writer != nullptr )
    {
        delete this->orbit_writer;
        this->orbit_writer = nullptr;
    }


    int  initialized = 0;
    auto status      = MPI_Initialized( &initialized );
    if ( status != MPI_SUCCESS )
    {
        WARN( "Failed to check MPI initialization status when exit the monitor of galotfa." );
    }
    else if ( initialized )
        MPI_Finalize();
}

int monitor::create_writers()
{
    // create the files for each analysis set
    this->create_files();

    // create the datasets in each file
    if ( this->para->md_switch_on )
        this->create_model_file_datasets();
    if ( this->para->ptc_switch_on )
        this->create_particle_file_datasets();
    if ( this->para->orb_switch_on )
        this->create_orbit_file_datasets();
    if ( this->para->grp_switch_on )
        this->create_group_file_datasets();
    if ( this->para->post_switch_on )
        this->create_post_file_datasets();
    return 0;
}

inline void monitor::create_files()
{
    // NOTE: this function will access the hdf5 files, so it should be called by the root process
    // Besides, it should be called only when galotfa is enabled (para->glb_switch_on == true)

    // create the files
    if ( !this->para->glb_multiple )
    // single analysis set case
    {
        std::string file1 = this->para->glb_output_dir + "/" + this->para->md_filename;
        std::string file2 = this->para->glb_output_dir + "/" + this->para->ptc_filename;
        std::string file3 = this->para->glb_output_dir + "/" + this->para->grp_filename;
        std::string file4 = this->para->glb_output_dir + "/" + this->para->post_filename;

        galotfa::writer *writer1 = nullptr, *writer2 = nullptr, *writer3 = nullptr,
                        *writer4 = nullptr;

        if ( this->para->md_switch_on )
            writer1 = new galotfa::writer( file1.c_str() );
        if ( this->para->ptc_switch_on )
            writer2 = new galotfa::writer( file2.c_str() );
        if ( this->para->grp_switch_on )
            writer3 = new galotfa::writer( file3.c_str() );
        if ( this->para->post_switch_on )
            writer4 = new galotfa::writer( file4.c_str() );
        this->writers.push_back( vector< galotfa::writer* >{ writer1, writer2, writer3, writer4 } );
    }
    else
    {
        for ( size_t i = 0; i < this->para->glb_target_sets.size(); ++i )
        {
            std::string prefix = "set" + std::to_string( i + 1 ) + "_";

            std::string file1 = this->para->glb_output_dir + "/" + prefix + this->para->md_filename;
            std::string file2 =
                this->para->glb_output_dir + "/" + prefix + this->para->ptc_filename;
            std::string file3 =
                this->para->glb_output_dir + "/" + prefix + this->para->grp_filename;
            std::string file4 =
                this->para->glb_output_dir + "/" + prefix + this->para->post_filename;

            galotfa::writer *writer1 = nullptr, *writer2 = nullptr, *writer3 = nullptr,
                            *writer4 = nullptr;

            if ( this->para->md_switch_on )
                writer1 = new galotfa::writer( file1.c_str() );
            if ( this->para->ptc_switch_on )
                writer2 = new galotfa::writer( file2.c_str() );
            if ( this->para->grp_switch_on )
                writer3 = new galotfa::writer( file3.c_str() );
            if ( this->para->post_switch_on )
                writer4 = new galotfa::writer( file4.c_str() );
            this->writers.push_back(
                vector< galotfa::writer* >{ writer1, writer2, writer3, writer4 } );
        }
    }

    if ( this->para->orb_switch_on )
    {
        std::string      file   = this->para->glb_output_dir + "/" + this->para->orb_filename;
        galotfa::writer* writer = new galotfa::writer( file.c_str() );
        this->orbit_writer      = writer;
    }
}

int monitor::save()
{
    // this function mock you press a button to save the data on the monitor dashboard
    // so it should only be called by the run_with() function
    int return_code = 0;
    if ( this->is_root() )
    {
        // auto datas = this->engine->feedback();
        //
        // return_code =
        //     this->writers[ 0 ]->push( ( double* )datas[ 0 ], 3, "/test_group/test_dataset" );

        if ( return_code != 0 )
            WARN( "Failed to push data to the writer." );
    }

    MPI_Bcast( &return_code, 1, MPI_INT, 0, MPI_COMM_WORLD );
    // make all the MPI processes return the same value
    return return_code;
}


inline void monitor::create_model_file_datasets()
{
    // this function should be called only when the model file is enabled
    // again, it should be called by the root process
    // TODO: specify different target analysis sets (and target particle types) for different
    // analysis level
    for ( auto& writers_of_single_set : this->writers )
    {
        // the size of the datasets
        galotfa::hdf5::size_info single_scaler_info = { H5T_NATIVE_INT, 1, { 1 } };  // for scalers
        // for 3D vectors
        galotfa::hdf5::size_info single_vector_info = { H5T_NATIVE_INT, 1, { 3 } };
        // for image matrix
        size_t                   binnum     = ( size_t )this->para->md_image_bins;
        galotfa::hdf5::size_info image_info = { H5T_NATIVE_INT, 2, { binnum, binnum } };
        // for tensor
        galotfa::hdf5::size_info tensor_info = { H5T_NATIVE_INT, 4, { binnum, binnum, 3, 3 } };

        writers_of_single_set[ 0 ]->create_dataset( "/Times", single_scaler_info );
        if ( this->para->pre_recenter )
            writers_of_single_set[ 0 ]->create_dataset( "/Center", single_vector_info );
        if ( this->para->md_image )
        {
            writers_of_single_set[ 0 ]->create_dataset( "/Image/Size", image_info );
            for ( auto& color : this->para->md_colors )
                writers_of_single_set[ 0 ]->create_dataset( "/Image/" + color, single_vector_info );
        }
        if ( this->para->md_bar_major_axis )
            writers_of_single_set[ 0 ]->create_dataset( "/Bar/MajorAxis", single_vector_info );
        if ( this->para->md_bar_length )
            writers_of_single_set[ 0 ]->create_dataset( "/Bar/Length", single_scaler_info );
        if ( this->para->md_sbar )
            writers_of_single_set[ 0 ]->create_dataset( "/Bar/SBar", single_scaler_info );
        if ( this->para->md_sbuckle )
            writers_of_single_set[ 0 ]->create_dataset( "/Bar/SBuckle", single_scaler_info );
        if ( this->para->md_am.size() > 0 )
            for ( auto& m : this->para->md_am )
                writers_of_single_set[ 0 ]->create_dataset( "/Bar/A" + std::to_string( m ),
                                                            single_scaler_info );
    }
}

inline void monitor::create_particle_file_datasets()
{
    // this function should be called only when the particle file is enabled
    // again, it should be called by the root process
    for ( auto& writers_of_single_set : this->writers )
    {
        // writers_of_single_set[ 1 ]->create_dataset( "/Times", single_scaler_info );
        // TODO: get the runtime quantitiy: the particle number
        // TODO: for the above feature, try to extract the target particles in the monitor class
    }
}

inline void monitor::create_orbit_file_datasets()
{
    // this function should be called only when the orbit file is enabled
    // again, it should be called by the root process
    FILE* idfile = fopen( this->para->orb_idfile.c_str(), "r" );
    if ( idfile == nullptr )
        ERROR( "The file %s does not exist!\n", this->para->orb_idfile.c_str() );

    struct stat* st = new struct stat;
    st->st_size     = 0;
    stat( this->para->orb_idfile.c_str(), st );
    try
    {
        this->check_filesize( st->st_size );
    }
    catch ( const std::exception& e )
    {
        delete st;
        fclose( idfile );
        ERROR( "%s", e.what() );
    }

    char buffer[ st->st_size + 1 ];
    buffer[ st->st_size ] = '\0';
    char* p_buffer        = buffer;
    fread( p_buffer, ( size_t )st->st_size, 1, idfile );
    std::string           id_string = buffer;
    vector< std::string > ids       = galotfa::string::split( id_string, " \t\n" );
    for ( auto& id_str : ids )
    {
        this->particle_ids.push_back( std::stoul( id_str ) );
    }

    for ( auto& target_id : this->particle_ids )
    {
        // TODO: support more available orbit types, such as recentered, aligned and corotating
        galotfa::hdf5::size_info single_vector_info = { H5T_NATIVE_DOUBLE, 1, { 3 } };
        // this->orbit_writer->create_group( "/Orbit" + std::to_string( target_id ) );
        this->orbit_writer->create_dataset(
            "/Particle" + std::to_string( target_id ) + "/Position",
            single_vector_info );  // create the dataset for each particle
        this->orbit_writer->create_dataset(
            "/Particle" + std::to_string( target_id ) + "/Velocity",
            single_vector_info );  // create the dataset for each particle
    }

    delete st;
    fclose( idfile );
}

inline void monitor::check_filesize( long int size ) const
// The same as ini_parser::check_filesize()
{
    if ( size == 0 )
    {
        ERROR( "The size of the id list file is 0, most common case: the file was not found." );
    }
    else if ( size >= 1024 * 1024 * 5 && size < 1024 * 1024 * 1024 )
    {
        WARN( "The id list file is too big: size = %ld MB.", size / 1024 / 1024 );
    }
    else if ( size >= 1024 * 1024 * 1024 )
    {
        ERROR( " The id list file is too big: size = %ld MB\nPlease check whether the your file is "
               "correct!",
               size / 1024 / 1024 );
    }
}

inline void monitor::create_group_file_datasets()
{
    // this function should be called only when the group file is enabled
    // again, it should be called by the root process
    ;
}

inline void monitor::create_post_file_datasets()
{
    // this function should be called only when the post file is enabled
    // again, it should be called by the root process
    ;
}

}  // namespace galotfa
#endif
