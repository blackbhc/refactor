#include "mpi.h"  // must include mpi.h before galotfa.h, to make prompt.h work correctly
#ifndef GALOTFA_H_INCLUDED
#define GALOTFA_H_INCLUDED
extern "C" {
void galotfa_with_pot_tracer( unsigned long pot_tracer_type, unsigned long particle_ids[],
                              unsigned long types[], double masses[], double coordiantes[][ 3 ],
                              double velocities[][ 3 ], double times,
                              unsigned long particle_number );

void galotfa_without_pot_tracer( unsigned long particle_ids[], unsigned long types[],
                                 double masses[], double coordiantes[][ 3 ],
                                 double velocities[][ 3 ], double times,
                                 unsigned long particle_number );
}
#endif
