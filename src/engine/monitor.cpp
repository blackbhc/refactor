#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>
#include <mpi.h>
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
        // create and start the virtual calc's calculator
        this->calc = new galotfa::calculator( *( this->para ) );
        this->calc->start();
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
    // extend the size of the members
    this->id_for_pre.resize( this->para->pre_recenter_anchors.size(), nullptr );
    this->part_num_pre.resize( this->para->pre_recenter_anchors.size(), 0 );
    if ( this->para->md_switch_on )
    {
        this->id_for_model.resize( this->para->md_target_sets.size(), nullptr );
        this->part_num_model.resize( this->para->md_target_sets.size(), 0 );
    }
    if ( this->para->ptc_switch_on )
    {
        this->id_for_particle.resize( this->para->ptc_particle_types.size(), nullptr );
        this->part_num_particle.resize( this->para->ptc_particle_types.size(), 0 );
    }
}

monitor::~monitor()
{
    if ( this->calc->is_active() )
        this->calc->stop();

    if ( this->para != nullptr )
    {
        delete this->para;
        this->para = nullptr;
    }

    if ( this->calc != nullptr )
    {
        delete this->calc;
        this->calc = nullptr;
    }

    // free the writers pointers
    for ( auto& writers_of_single_section : this->vec_of_writers )
        for ( auto& writer : writers_of_single_section )
            if ( writer != nullptr )
            {
                delete writer;
                writer = nullptr;
            }
    this->vec_of_writers.clear();

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
    // initialized the vector of the writers
    for ( int i = 0; i < 5; ++i )
    {
        this->vec_of_writers.push_back( vector< galotfa::writer* >{} );
    }

    // create the files for each analysis set
    this->create_files();

    // create the datasets in each file
    if ( this->para->md_switch_on )
        this->create_model_file_datasets();
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

    if ( this->para->md_multiple )
    {
        for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
        {
            std::string prefix = "set" + std::to_string( i + 1 ) + "_";

            std::string file = this->para->glb_output_dir + "/" + prefix + this->para->md_filename;

            galotfa::writer* writer = new galotfa::writer( file.c_str() );
            this->vec_of_writers[ 0 ].push_back( writer );
        }
    }
    else
    {

        std::string file = this->para->glb_output_dir + "/" + this->para->md_filename;

        galotfa::writer* writer = new galotfa::writer( file.c_str() );
        this->vec_of_writers[ 0 ].push_back( writer );
    }

    if ( this->para->ptc_switch_on )
    {
        std::string      file   = this->para->glb_output_dir + "/" + this->para->ptc_filename;
        galotfa::writer* writer = new galotfa::writer( file.c_str() );
        this->vec_of_writers[ 1 ].push_back( writer );
    }

    if ( this->para->orb_switch_on )
    {
        std::string      file   = this->para->glb_output_dir + "/" + this->para->orb_filename;
        galotfa::writer* writer = new galotfa::writer( file.c_str() );
        this->vec_of_writers[ 2 ].push_back( writer );
    }
}

int monitor::save()
{
    // this function mock you press a button to save the data on the monitor dashboard
    // so it should only be called by the run_with() function
    int return_code = 0;
    if ( this->is_root() )
    {
        // TODO: sort the particle ids in ascending order before writing

        double datas[ 1 ] = { 0.1 };
        return_code = this->vec_of_writers[ 0 ][ 0 ]->push( ( double* )datas, 1, "/Bar/SBar" );

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
    for ( auto& writer_of_single_set : this->vec_of_writers[ 0 ] )
    {
        // the size of the datasets
        galotfa::hdf5::size_info single_scaler_info = { H5T_NATIVE_DOUBLE,
                                                        1,
                                                        { 1 } };  // for scalers
        // for 3D vectors
        galotfa::hdf5::size_info single_vector_info = { H5T_NATIVE_DOUBLE, 1, { 3 } };
        // for image matrix
        size_t                   binnum     = ( size_t )this->para->md_image_bins;
        galotfa::hdf5::size_info image_info = { H5T_NATIVE_DOUBLE, 2, { binnum, binnum } };
        // for tensor
        galotfa::hdf5::size_info tensor_info = { H5T_NATIVE_DOUBLE, 4, { binnum, binnum, 3, 3 } };

        writer_of_single_set->create_dataset( "/Times", single_scaler_info );
        if ( this->para->pre_recenter )
            writer_of_single_set->create_dataset( "/Center", single_vector_info );
        if ( this->para->md_image )
        {
            writer_of_single_set->create_dataset( "/Image/Size", image_info );
            for ( auto& color : this->para->md_colors )
                writer_of_single_set->create_dataset( "/Image/" + color, single_vector_info );
        }
        if ( this->para->md_bar_major_axis )
            writer_of_single_set->create_dataset( "/Bar/MajorAxis", single_vector_info );
        if ( this->para->md_bar_length )
            writer_of_single_set->create_dataset( "/Bar/Length", single_scaler_info );
        if ( this->para->md_sbar )
            writer_of_single_set->create_dataset( "/Bar/SBar", single_scaler_info );
        if ( this->para->md_sbuckle )
            writer_of_single_set->create_dataset( "/Bar/SBuckle", single_scaler_info );
        if ( this->para->md_am.size() > 0 )
            for ( auto& m : this->para->md_am )
                writer_of_single_set->create_dataset( "/Bar/A" + std::to_string( m ),
                                                      single_scaler_info );
    }
}

void monitor::create_particle_file_datasets( vector< unsigned long >& particle_ana_nums )
{
    // this function should be called only when the particle file is enabled
    // again, it should be called by the root process
    // NOTE: due to the particle number of target particels is unknown, so the datasets should be
    // created in the first call of `run_with(...)`
    for ( size_t i = 0; i < this->para->ptc_particle_types.size(); ++i )
    {
        if ( particle_ana_nums[ i ] == 0 )  // if there is no target particle, just ignore it
            continue;

        galotfa::hdf5::size_info scalers_info = { H5T_NATIVE_DOUBLE,
                                                  1,
                                                  { particle_ana_nums[ i ] } };
        std::string prefix = "/PartType" + std::to_string( this->para->ptc_particle_types[ i ] );
        if ( this->para->ptc_circularity )
        {
            this->vec_of_writers[ 1 ][ 0 ]->create_dataset( prefix + "/Circularity", scalers_info );
        }
        if ( this->para->ptc_circularity_3d )
        {
            this->vec_of_writers[ 1 ][ 0 ]->create_dataset( prefix + "/Circularity3D",
                                                            scalers_info );
        }
        if ( this->para->ptc_rg )
        {
            this->vec_of_writers[ 1 ][ 0 ]->create_dataset( prefix + "/Rg", scalers_info );
        }
        // TODO: the orbital frequency is not supported yet
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
        this->orbit_log_ids.push_back( std::stoul( id_str ) );
    }

    this->orbit_part_num = ids.size();
    for ( auto& target_id : this->orbit_log_ids )
    {
        // TODO: support more available orbit types, such as recentered, aligned and corotating
        galotfa::hdf5::size_info single_vector_info = { H5T_NATIVE_DOUBLE, 1, { 3 } };
        // this->orbit_writer->create_group( "/Orbit" + std::to_string( target_id ) );
        this->vec_of_writers[ 2 ][ 0 ]->create_dataset(
            "/Particle" + std::to_string( target_id ) + "/Position",
            single_vector_info );  // create the dataset for each particle
        this->vec_of_writers[ 2 ][ 0 ]->create_dataset(
            "/Particle" + std::to_string( target_id ) + "/Velocity",
            single_vector_info );  // create the dataset for each particle
        // TODO: add an attribute in each dataset to indicate the particle id
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
    // TODO: implement this function
    ;
}

inline void monitor::create_post_file_datasets()
{
    // this function should be called only when the post file is enabled
    // again, it should be called by the root process
    // TODO: implement this function
    ;
}

int monitor::run_with call_without_tracer
{
    if ( !this->para->glb_switch_on )  // if galotfa is disabled, just return 0
        return 0;

    if ( this->need_extract() )
        this->extractor( particle_number, types, particle_ids );  // extract the target particles

    if ( this->step == 0 && this->para->ptc_switch_on )
    {
        // create the particle file's datasets at the first step
        auto                    ptc_target_type_num = this->para->ptc_particle_types.size();
        vector< unsigned long > particle_ana_nums( ptc_target_type_num, 0 );
        for ( size_t i = 0; i < ptc_target_type_num; ++i )
            particle_ana_nums[ i ] = this->part_num_particle[ i ];
        MPI_Allreduce( MPI_IN_PLACE, particle_ana_nums.data(), ptc_target_type_num,
                       MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD );
        // sum the number of target particles in all the MPI processes
        if ( this->is_root() )
            this->create_particle_file_datasets( particle_ana_nums );  // create the datasets
    }

    push_data no_tracer;
    int       return_code = this->save();
    if ( return_code != 0 )
    {
        WARN( "Failed to save the data to the output files." );
        return return_code;
    }

    if ( this->need_extract() )
        this->release_once();  // release the memory allocated in extractor()

    ++this->step;
    return 0;
}

inline bool monitor::need_ana_model() const
{
    return this->para->md_switch_on && this->step % this->para->md_period == 0;
}
inline bool monitor::need_ana_particle() const
{
    return this->para->ptc_switch_on && this->step % this->para->ptc_period == 0;
}
inline bool monitor::need_log_orbit() const
{
    return this->para->orb_switch_on && this->step % this->para->orb_period == 0;
}
inline bool monitor::need_ana_group() const
{
    return this->para->grp_switch_on && this->step % this->para->grp_period == 0;
}
inline bool monitor::need_extract() const
{
    return need_ana_model() || need_ana_particle() || need_log_orbit() || need_ana_group();
}

void monitor::release_once() const
{
    if ( need_ana_model() )
        for ( size_t i = 0; i < this->id_for_pre.size(); ++i )
        {
            if ( this->id_for_pre[ i ] != nullptr )
            {
                delete[] this->id_for_pre[ i ];
                this->id_for_pre[ i ] = nullptr;
            }
            this->part_num_pre[ i ] = 0;
        }

    if ( this->para->md_switch_on )
    {
        for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
        {
            if ( this->id_for_model[ i ] != nullptr )
            {
                delete[] this->id_for_model[ i ];
                this->id_for_model[ i ] = nullptr;
            }
            this->part_num_model[ i ] = 0;
        }
    }
    if ( this->para->ptc_switch_on )
    {
        for ( size_t i = 0; i < this->para->ptc_particle_types.size(); ++i )
        {
            if ( this->id_for_particle[ i ] != nullptr )
            {
                delete[] this->id_for_particle[ i ];
                this->id_for_particle[ i ] = nullptr;
            }
            this->part_num_particle[ i ] = 0;
        }
    }

    if ( this->id_for_orbit != nullptr )
    {
        delete[] this->id_for_orbit;
        this->id_for_orbit   = nullptr;
        this->part_num_orbit = 0;
    }

    /* if ( this->id_for_group != nullptr )
    {
        delete[] this->id_for_group;
        this->id_for_group = nullptr;
    } */
}

void monitor::extractor( unsigned long& partnum_total, unsigned long types[],
                         unsigned long ids[] ) const
{
    static unsigned long i = 0;  // a static temporary variable
    for ( auto& ids : this->id_for_pre )
        ids = new unsigned long[ partnum_total ];
    // NOTE: the pre-process data will always be extracted, so the extractor should be called
    // only when need_extract() is true

    if ( this->need_ana_model() )
    {
        for ( auto& ids : this->id_for_model )
            ids = new unsigned long[ partnum_total ];
    }
    if ( this->need_ana_particle() )
    {
        for ( auto& ids : this->id_for_particle )
            ids = new unsigned long[ partnum_total ];
    }
    if ( this->need_log_orbit() )
    {
        this->id_for_orbit = new unsigned long[ this->orbit_part_num ];
    }

    /* if ( this->need_ana_group() )
    {
        this->id_for_group = new unsigned long[ partnum_total ];
    } */

    static size_t j = 0;  // a static temporary variable
    static size_t k = 0;  // a static temporary variable
    for ( i = 0; i < partnum_total; ++i )
    {
        // i: index for iterating all the particles
        // j: index for iterating all the target sets
        // k: index for iterating all the members in each target set
        if ( this->calc->is_target_of_pre() )
        {
            for ( size_t j = 0; j < this->para->pre_recenter_anchors.size(); ++j )
            {
                if ( types[ i ] == this->para->pre_recenter_anchors[ j ] )
                {
                    this->id_for_pre[ j ][ this->part_num_pre[ j ]++ ] = i;
                }
            }
        }

        if ( this->need_ana_model() )
            if ( this->calc->is_target_of_md() )
            {
                for ( size_t j = 0; j < this->para->md_target_sets.size(); ++j )
                {
                    for ( size_t k = 0; k < this->para->md_target_sets[ j ].size(); ++k )
                        if ( types[ i ] == this->para->md_target_sets[ j ][ k ] )
                            this->id_for_model[ j ][ this->part_num_model[ j ]++ ] = i;
                }
            }

        if ( this->need_ana_particle() )
        {
            for ( size_t j = 0; j < this->para->ptc_particle_types.size(); ++j )
            {
                if ( types[ i ] == this->para->ptc_particle_types[ j ] )
                    this->id_for_particle[ j ][ this->part_num_particle[ j ]++ ] = i;
            }
        }

        if ( this->need_log_orbit() )
        {
            for ( size_t j = 0; j < this->orbit_part_num; ++j )
            {
                if ( ids[ i ] == this->orbit_log_ids[ j ] )
                    this->id_for_orbit[ this->part_num_orbit++ ] = i;
            }
        }
    }
}

}  // namespace galotfa
#endif
