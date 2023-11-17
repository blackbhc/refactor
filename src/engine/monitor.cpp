#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace galotfa {
monitor::monitor( void )
{
    // check and ensure MPI is initialized
    auto status = MPI_Initialized( &this->mpi_init_before_galotfa );
    if ( status != MPI_SUCCESS )
    {
        ERROR( "Failed to check MPI initialization status." );
    }
    else if ( !this->mpi_init_before_galotfa )
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
        this->calc = new galotfa::calculator( this->para );
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
    // TODO: call the post analysis module of the calculator
    if ( this->calc != nullptr )
    {
        delete this->calc;
        this->calc = nullptr;
    }

    if ( this->para != nullptr )
    {
        delete this->para;
        this->para = nullptr;
    }

    // free the writers pointers
    if ( this->writers.model_writers.size() > 0 )
        for ( auto& writer : this->writers.model_writers )
            if ( writer != nullptr )
            {
                delete writer;
                writer = nullptr;
            }

    if ( this->writers.group_writers.size() > 0 )
        for ( auto& writer : this->writers.group_writers )
            if ( writer != nullptr )
            {
                delete writer;
                writer = nullptr;
            }

    if ( this->writers.particle_writer != nullptr )
    {
        delete this->writers.particle_writer;
        this->writers.particle_writer = nullptr;
    }

    if ( this->writers.orbit_writer != nullptr )
    {
        delete this->writers.orbit_writer;
        this->writers.orbit_writer = nullptr;
    }

    if ( !this->mpi_init_before_galotfa )
        MPI_Finalize();
}

int monitor::create_writers()
{
    // initialized the vector of the writers

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
            this->writers.model_writers.push_back( writer );
        }
    }
    else
    {
        std::string file = this->para->glb_output_dir + "/" + this->para->md_filename;

        galotfa::writer* writer = new galotfa::writer( file.c_str() );
        this->writers.model_writers.push_back( writer );
    }

    if ( this->para->ptc_switch_on )
    {
        std::string      file         = this->para->glb_output_dir + "/" + this->para->ptc_filename;
        galotfa::writer* writer       = new galotfa::writer( file.c_str() );
        this->writers.particle_writer = writer;
    }

    if ( this->para->orb_switch_on )
    {
        std::string      file      = this->para->glb_output_dir + "/" + this->para->orb_filename;
        galotfa::writer* writer    = new galotfa::writer( file.c_str() );
        this->writers.orbit_writer = writer;
    }
    // TODO: the file of group analysis
}

int monitor::save()
{
    // this function mock you press a button to save the data on the monitor dashboard
    // so it should only be called by the run_with() function
    int return_code = 0;
    if ( this->is_root() && this->need_ana() )
    {
        galotfa::analysis_result* res = this->calc->feedback();  // get the analysis results
        int                       i   = 0;  // index of the target model analysis sets
        if ( this->need_ana_model() )
            for ( auto& single_model : this->writers.model_writers )
            {
                single_model->push< double >( &this->time, 1, "/Times" );

                if ( this->para->pre_recenter )
                    single_model->push< double >( res->system_center, 3, "/Center" );
                if ( this->para->md_bar_major_axis )
                    single_model->push< double >( &res->bar_marjor_axis[ i ], 1, "/Bar/MajorAxis" );
                if ( this->para->md_sbar )
                    single_model->push< double >( &res->s_bar[ i ], 1, "/Bar/SBar" );
                if ( this->para->md_sbuckle )
                    single_model->push< double >( &res->s_buckle[ i ], 1, "/Bar/SBuckle" );
                if ( this->para->md_an.size() > 0 )
                    for ( auto& m : this->para->md_an )
                    {
                        double real = res->Ans[ m ][ i ].real();
                        double imag = res->Ans[ m ][ i ].real();
                        single_model->push< double >( &real, 1,
                                                      "/Bar/A" + std::to_string( m ) + "(real)" );
                        single_model->push< double >( &imag, 1,
                                                      "/Bar/A" + std::to_string( m ) + "(imag)" );
                    }
                if ( this->para->md_bar_radius )
                    single_model->push< double >( &res->bar_radius[ i ], 1, "/Bar/Length" );

                if ( this->para->md_image )
                {
                    unsigned long binnum = this->para->md_image_bins;
                    for ( auto& color : this->para->md_colors )
                    {
                        if ( color == "number_density" )
                        {
                            single_model->push< double >( res->images[ 0 ][ 0 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(xy)" );
                            single_model->push< double >( res->images[ 0 ][ 1 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(xz)" );
                            single_model->push< double >( res->images[ 0 ][ 2 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(yz)" );
                        }
                        else if ( color == "surface_density" )
                        {
                            single_model->push< double >( res->images[ 1 ][ 0 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(xy)" );
                            single_model->push< double >( res->images[ 1 ][ 1 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(xz)" );
                            single_model->push< double >( res->images[ 1 ][ 2 ][ i ],
                                                          binnum * binnum,
                                                          "/Image/" + color + "(yz)" );
                        }
                        else if ( color == "mean_velocity" )
                        {
                            for ( int n = 0; n < 3; ++n )
                            {
                                single_model->push< double >(
                                    res->images[ 2 + n ][ 0 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(xy)" );
                                single_model->push< double >(
                                    res->images[ 2 + n ][ 1 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(yz)" );
                                single_model->push< double >(
                                    res->images[ 2 + n ][ 2 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(yz)" );
                            }
                        }
                        else  // ( color == "velocity_dispersion" )
                        {
                            for ( int n = 0; n < 3; ++n )
                            {
                                single_model->push< double >(
                                    res->images[ 5 + n ][ 0 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(xy)" );
                                single_model->push< double >(
                                    res->images[ 5 + n ][ 1 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(yz)" );
                                single_model->push< double >(
                                    res->images[ 5 + n ][ 2 ][ i ], binnum * binnum,
                                    "/Image/" + color + "_axis_" + std::to_string( n + 1 )
                                        + "(yz)" );
                            }
                        }
                    }
                }
                if ( this->para->md_dispersion_tensor )
                {
                    unsigned long binnum = this->para->md_image_bins;
                    unsigned long binnum_third =
                        ( unsigned long )this->para->md_image_bins * this->para->md_axis_ratio;
                    single_model->push< double >( res->dispersion_tensor[ i ],
                                                  binnum * binnum * binnum_third * 3 * 3,
                                                  "/DispersionTensor", 5 );
                }
                if ( this->para->md_inertia_tensor )
                {
                    single_model->push< double >( res->inertia_tensor[ i ], 9, "/InertiaTensor" );
                }
                ++i;
            }
    }

    MPI_Bcast( &return_code, 1, MPI_INT, 0, MPI_COMM_WORLD );
    // make all the MPI processes return the same value
    return return_code;
}


inline void monitor::create_model_file_datasets()
{
    // this function should be called only when the model file is enabled
    // again, it should be called by the root process
    for ( auto& single_model : this->writers.model_writers )
    {
        // the size of the datasets
        galotfa::hdf5::size_info single_scaler_info = { H5T_NATIVE_DOUBLE,
                                                        1,
                                                        { 1 } };  // for scalers
        // for 3D vectors
        galotfa::hdf5::size_info single_vector_info = { H5T_NATIVE_DOUBLE, 1, { 3 } };
        // for image matrix
        size_t binnum       = ( size_t )this->para->md_image_bins;
        size_t binnum_third = ( size_t )this->para->md_image_bins * this->para->md_axis_ratio;
        galotfa::hdf5::size_info image_info = { H5T_NATIVE_DOUBLE, 2, { binnum, binnum } };
        // for tensor
        // 5 dimensions: x, y, z, (i, j) of the tensor
        galotfa::hdf5::size_info dispersion_tensor_info = {
            H5T_NATIVE_DOUBLE, 5, { binnum, binnum, binnum_third, 3, 3 }
        };
        galotfa::hdf5::size_info inertia_tensor_info = { H5T_NATIVE_DOUBLE, 2, { 3, 3 } };

        single_model->create_dataset( "/Times", single_scaler_info );
        if ( this->para->pre_recenter )
            single_model->create_dataset( "/Center", single_vector_info );
        if ( this->para->md_bar_major_axis )
            single_model->create_dataset( "/Bar/MajorAxis", single_scaler_info );
        if ( this->para->md_bar_radius )
            single_model->create_dataset( "/Bar/Length", single_scaler_info );
        if ( this->para->md_sbar )
            single_model->create_dataset( "/Bar/SBar", single_scaler_info );
        if ( this->para->md_sbuckle )
            single_model->create_dataset( "/Bar/SBuckle", single_scaler_info );
        if ( this->para->md_an.size() > 0 )
            for ( auto& m : this->para->md_an )
            {
                single_model->create_dataset( "/Bar/A" + std::to_string( m ) + "(real)",
                                              single_scaler_info );
                single_model->create_dataset( "/Bar/A" + std::to_string( m ) + "(imag)",
                                              single_scaler_info );
            }
        if ( this->para->md_image )
        {
            for ( auto& color : this->para->md_colors )
            {
                if ( color == "number_density" )
                {
                    single_model->create_dataset( "/Image/" + color + "(xy)", image_info );
                    single_model->create_dataset( "/Image/" + color + "(xz)", image_info );
                    single_model->create_dataset( "/Image/" + color + "(yz)", image_info );
                }
                else if ( color == "surface_density" )
                {
                    single_model->create_dataset( "/Image/" + color + "(xy)", image_info );
                    single_model->create_dataset( "/Image/" + color + "(xz)", image_info );
                    single_model->create_dataset( "/Image/" + color + "(yz)", image_info );
                }
                else if ( color == "mean_velocity" )
                {
                    for ( int n = 0; n < 3; ++n )
                    {
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(xy)",
                                                      image_info );
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(xz)",
                                                      image_info );
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(yz)",
                                                      image_info );
                    }
                }
                else  // ( color == "velocity_dispersion" )
                {
                    for ( int n = 0; n < 3; ++n )
                    {
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(xy)",
                                                      image_info );
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(xz)",
                                                      image_info );
                        single_model->create_dataset( "/Image/" + color + "_axis_"
                                                          + std::to_string( n + 1 ) + "(yz)",
                                                      image_info );
                    }
                }
            }
        }

        if ( this->para->md_dispersion_tensor )
            single_model->create_dataset( "/DispersionTensor", dispersion_tensor_info, 5 );
        if ( this->para->md_inertia_tensor )
            single_model->create_dataset( "/InertiaTensor", inertia_tensor_info );
        // use a smaller chunk size to avoid the memory error
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
            this->writers.particle_writer->create_dataset( prefix + "/Circularity", scalers_info );
        }
        if ( this->para->ptc_circularity_3d )
        {
            this->writers.particle_writer->create_dataset( prefix + "/Circularity3D",
                                                           scalers_info );
        }
        if ( this->para->ptc_rg )
        {
            this->writers.particle_writer->create_dataset( prefix + "/Rg", scalers_info );
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
        this->writers.orbit_writer->create_dataset(
            "/Particle" + std::to_string( target_id ) + "/Position",
            single_vector_info );  // create the dataset for each particle
        this->writers.orbit_writer->create_dataset(
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

    this->time = time;

    if ( this->need_ana() )
        this->extractor( particle_number, types, particle_ids,
                         coordinates );  // extract the target particles

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

    inject_data no_tracer;
    int         return_code = this->save();
    if ( return_code != 0 )
    {
        WARN( "Failed to save analysis results to the output files." );
        return return_code;
    }

    if ( this->need_ana() )
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
inline bool monitor::need_ana() const
{
    return need_ana_model() || need_ana_particle() || need_log_orbit() || need_ana_group();
}

void monitor::release_once() const
{
    if ( this->para->md_switch_on )
        if ( need_ana_model() )
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
        if ( need_ana_particle() )
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
        if ( need_log_orbit() )
        {
            delete[] this->id_for_orbit;
            this->id_for_orbit   = nullptr;
            this->part_num_orbit = 0;
        }

    /* if ( this->id_for_group != nullptr )
        if ( need_ana_group() )
        {
            delete[] this->id_for_group;
            this->id_for_group = nullptr;
        } */
}

void monitor::extractor( int& partnum_total, int types[], int ids[],
                         double coordinates[][ 3 ] ) const
{
    // NOTE: the pre-process data will always be extracted, so the extractor should be called
    // only when need_ana() is true

    // HACK: the extract of target particles for pre-process is implemented in the calculator
    if ( this->need_ana_model() )
    {
        for ( auto& ids : this->id_for_model )
            ids = new int[ partnum_total ];
    }
    if ( this->need_ana_particle() )
    {
        for ( auto& ids : this->id_for_particle )
            ids = new int[ partnum_total ];
    }
    if ( this->need_log_orbit() )
    {
        this->id_for_orbit = new int[ this->orbit_part_num ];
    }

    /* if ( this->need_ana_group() )
    {
        this->id_for_group = new unsigned long[ partnum_total ];
    } */

    static int    i = 0;  // a static temporary variable
    static size_t j = 0;  // a static temporary variable
    static size_t k = 0;  // a static temporary variable
    // i: index for iterating all the particles
    // j: index for iterating all the target sets
    // k: index for iterating all the members in each target set
    for ( i = 0; i < partnum_total; ++i )
    {
        // extract the target particles for model analysis
        if ( this->need_ana_model() )
            if ( this->calc->is_target_of_md( types[ i ], coordinates[ i ][ 0 ],
                                              coordinates[ i ][ 1 ], coordinates[ i ][ 2 ] ) )
            {
                for ( j = 0; j < this->para->md_target_sets.size(); ++j )
                {
                    for ( k = 0; k < this->para->md_target_sets[ j ].size(); ++k )
                        if ( types[ i ] == this->para->md_target_sets[ j ][ k ] )
                        {
                            this->id_for_model[ j ][ this->part_num_model[ j ]++ ] = i;
                            break;
                        }
                }
            }

        // extract the target particles for particle analysis
        if ( this->need_ana_particle() )
        {
            for ( size_t j = 0; j < this->para->ptc_particle_types.size(); ++j )
            {
                if ( types[ i ] == this->para->ptc_particle_types[ j ] )
                {
                    this->id_for_particle[ j ][ this->part_num_particle[ j ]++ ] = i;
                    break;
                }
            }
        }

        // extract the target particles for orbit analysis
        if ( this->need_log_orbit() )
        {
            for ( int j = 0; j < this->orbit_part_num; ++j )
            {
                if ( ids[ i ] == this->orbit_log_ids[ j ] )
                {
                    this->id_for_orbit[ this->part_num_orbit++ ] = i;
                    break;
                }
            }
        }
    }
}

inline int monitor::inject_data call_without_tracer const
{
    ( void )time;  // avoid the warning of unused variable
    ( void )particle_ids;
    // This function should be called before the increment of the step counter
    if ( need_ana() )
        this->calc->call_pre_module( particle_number, types, masses, coordinates );
    if ( need_ana_model() )
        this->calc->call_md_module( masses, coordinates, velocities, this->id_for_model,
                                    this->part_num_model );
    if ( need_ana_particle() )
        this->calc->call_ptc_module();
    if ( need_log_orbit() )
        this->calc->call_orb_module();
    if ( need_ana_group() )
        this->calc->call_grp_module();
    return 0;
}


}  // namespace galotfa
#endif
