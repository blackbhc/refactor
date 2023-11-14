#include "mpi.h"  // must include mpi.h before galotfa.h, to make prompt.h work correctly
#ifndef GALOTFA_H_INCLUDED
#define GALOTFA_H_INCLUDED
extern "C" {
void galotfa_with_pot_tracer( int pot_tracer_type, int particle_ids[], int types[], double masses[],
                              double coordiantes[][ 3 ], double velocities[][ 3 ], double times,
                              int particle_number );

void galotfa_without_pot_tracer( int particle_ids[], int types[], double masses[],
                                 double coordiantes[][ 3 ], double velocities[][ 3 ], double times,
                                 int particle_number );
}
#endif
