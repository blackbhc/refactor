#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>

namespace galotfa {
monitor::monitor( void )
{
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

    galotfa::ini_parser ini( "./galotfa.ini" );
    this->para   = new galotfa::para( ini );
    this->engine = new galotfa::calculator( *( this->para ) );
    this->engine->start();
    if ( this->is_root() )
        this->create_writers();
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

    if ( this->writers.size() > 0 )
    {
        for ( auto writer : this->writers )
        {
            delete writer;
            writer = nullptr;
        }
        this->writers.clear();
    }

    int  initialized = 0;
    auto status      = MPI_Initialized( &initialized );
    if ( status != MPI_SUCCESS )
    {
        WARN( "Failed to check MPI initialization status when exit the monitor of galotfa." );
    }
    else if ( !initialized )
        MPI_Init( NULL, NULL );
}

int monitor::create_writers()
{
    // NOTE: this function will access the hdf5 files, so it should be called by the root process
    // TODO: create the writers (a hdf5 file) and its nodes (group and dataset) based on the ini
    // parameter file

    // TEST: create a test writer
    auto writer = new galotfa::writer( "test_output" );
    this->writers.push_back( writer );
    hdf5::size_info size_info = { H5T_NATIVE_DOUBLE, 1, { 3 } };
    writer->create_dataset( "test_group/test_dataset", size_info );
    return 0;
}

int monitor::save()
{
    // this function mock you press a button to save the data on the monitor dashboard
    int return_code = 0;
    if ( this->is_root() )
    {
        auto datas = this->engine->feedback();
        // TEST: it has 1 element to a double[3] array

        return_code =
            this->writers[ 0 ]->push( ( double* )datas[ 0 ], 3, "/test_group/test_dataset" );

        if ( return_code != 0 )
            WARN( "Failed to push data to the writer." );
    }

    MPI_Bcast( &return_code, 1, MPI_INT, 0, MPI_COMM_WORLD );
    // make all the MPI processes return the same value
    return return_code;
}

}  // namespace galotfa
#endif
