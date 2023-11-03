#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>

namespace galotfa {
monitor::monitor( void )
{
    galotfa::ini_parser ini( "./galotfa.ini" );
    this->para   = new galotfa::para( ini );
    this->engine = new galotfa::calculator( *( this->para ) );
    this->engine->start();
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
}

int monitor::create_writers()
{
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
    auto datas = this->engine->feedback();
    // TEST: it has 1 element to a double[3] array

    int return_code =
        this->writers[ 0 ]->push( ( double* )datas[ 0 ], 3, "/test_group/test_dataset" );

    if ( return_code != 0 )
    {
        WARN( "Failed to push data to the writer." );
        return return_code;
    }

    return 0;
}

}  // namespace galotfa
#endif