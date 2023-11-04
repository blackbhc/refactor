#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>
#include <sys/stat.h>
#include <unistd.h>

namespace galotfa {
monitor::monitor( void )
{
    // set the writers pointers to nullptr
    for ( int i = 0; i < 5; ++i )
        this->writers[ i ] = nullptr;

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
    for ( int i = 0; i < 5; ++i )
    {
        if ( this->writers[ i ] != nullptr )
        {
            delete this->writers[ i ];
            this->writers[ i ] = nullptr;
        }
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
    // NOTE: this function will access the hdf5 files, so it should be called by the root process
    // Besides, it should be called only when galotfa is enabled (para->glb_switch_on == true)

    // TODO: create the datasets at the same time
    if ( this->para->md_switch_on )
    {
        auto file          = this->para->glb_output_dir + "/" + this->para->md_filename;
        auto writer        = new galotfa::writer( file.c_str() );
        this->writers[ 0 ] = writer;
        this->create_model_file_datasets();
    }
    if ( this->para->ptc_switch_on )
    {
        auto file          = this->para->glb_output_dir + "/" + this->para->ptc_filename;
        auto writer        = new galotfa::writer( file.c_str() );
        this->writers[ 1 ] = writer;
    }
    if ( this->para->orb_switch_on )
    {
        auto file          = this->para->glb_output_dir + "/" + this->para->orb_filename;
        auto writer        = new galotfa::writer( file.c_str() );
        this->writers[ 2 ] = writer;
    }
    if ( this->para->grp_switch_on )
    {
        auto file          = this->para->glb_output_dir + "/" + this->para->grp_filename;
        auto writer        = new galotfa::writer( file.c_str() );
        this->writers[ 3 ] = writer;
    }
    if ( this->para->post_switch_on )
    {
        auto file          = this->para->glb_output_dir + "/" + this->para->post_filename;
        auto writer        = new galotfa::writer( file.c_str() );
        this->writers[ 4 ] = writer;
    }

    return 0;
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
    // TODO: create datasets based on the enabled target analysis sets

    ;
}

}  // namespace galotfa
#endif
