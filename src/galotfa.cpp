#ifndef GALOTFA_CPP_INCLUDED
#define GALOTFA_CPP_INCLUDED
#include "galotfa.h"
#include "engine/monitor.h"
#ifdef GALOTFA_HEADER_ONLY
#include "../engine/calculator.cpp"
#include "../engine/monitor.cpp"
#include "../output/writer.cpp"
#include "../parameter/ini_parser.cpp"
#include "../parameter/para.cpp"
#include "../tools/prompt.cpp"
#include "../tools/string.cpp"
#endif

extern "C" {
void galotfa_without_pot_tracer( int particle_ids[], int types[], double masses[],
                                 double coordiantes[][ 3 ], double velocities[][ 3 ], double time,
                                 int particle_number )
{
    static galotfa::monitor otf_monitor;

    int run_failed = otf_monitor.run_with( particle_ids, types, masses, coordiantes, velocities,
                                           time, particle_number );
    if ( run_failed )
        WARN( "Failed to run galotfa at some steps!" );
    return;
}

void galotfa_with_pot_tracer( int pot_tracer_type, int particle_ids[], int types[], double masses[],
                              double coordiantes[][ 3 ], double velocities[][ 3 ], double time,
                              int particle_number )

{
    // TODO: to be implemented
    ( void )pot_tracer_type;
    static galotfa::monitor otf_monitor;

    int run_failed = otf_monitor.run_with( particle_ids, types, masses, coordiantes, velocities,
                                           time, particle_number );
    if ( run_failed )
        WARN( "Failed to run galotfa at some steps!" );
    return;
}
}
#endif
