#ifndef GALOTFA_MONITOR_CPP
#define GALOTFA_MONITOR_CPP
#include "monitor.h"
#include <hdf5.h>

// I don't want to write this verbose argument list again and again ...
// In summary, the argument list is:
// id, type, mass, coordinate, velocity, time, particle_number
// and an optional potential tracer type
/* (The following macro is defined in the header file calculator.h)
#define call_without_tracer                                                            \
    ( unsigned long( &particle_ids )[], unsigned int( &types )[], double( &masses )[], \
      double( &coordiantes )[][ 3 ], double( &velocities )[][ 3 ], double( &times )[], \
      unsigned long& particle_number )

#define call_with_tracer                                                                          \
    ( unsigned long& pot_tracer_type, unsigned long( &particle_ids )[], unsigned int( &types )[], \
      double( &masses )[], double( &coordiantes )[][ 3 ], double( &velocities )[][ 3 ],           \
      double( &times )[], unsigned long& particle_number )
*/
#define no_tracer ( particle_ids, types, masses, coordiantes, velocities, times, particle_number )
#define has_tracer ( particle_ids, types, masses, coordiantes, velocities, times, particle_number )


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

// data API without potential tracer
inline int monitor::run_with call_without_tracer
{
    push_data no_tracer;
    this->save();
    return 0;
}

// data API with potential tracer
inline int monitor::run_with call_with_tracer
{
    push_data has_tracer;
    return 0;
}

// the push data API without potential tracer
inline int monitor::push_data call_without_tracer const
{
    this->engine->recv_data no_tracer;
    return 0;
};

// the push data API with potential tracer
inline int monitor::push_data call_with_tracer const
{
    this->engine->recv_data has_tracer;
    return 0;
}

int monitor::create_writers()
{
    // TODO: create the writers (a hdf5 file) and its nodes (group and dataset) based on the ini
    // parameter file

    // TEST: create a test writer
    auto writer = new galotfa::writer( "test_writer" );
    this->writers.push_back( writer );
    writer->create_group( "test_group" );
    hdf5::size_info size_info = { H5T_NATIVE_DOUBLE, 1, { 1 } };
    writer->create_dataset( "test_dataset", size_info );
    return 0;
}

int monitor::save()
{
    // this function mock you press a button to save the data on the monitor dashboard
    auto datas = this->engine->feedback();
    // TEST: it has 1 element to 1 double[3] array

    this->writers[ 0 ]->push( ( double* )datas[ 0 ], 3, "/test_group/test_dataset" );

    return 0;
}

}  // namespace galotfa
#endif
