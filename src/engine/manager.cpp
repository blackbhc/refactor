#ifndef GALOTFA_MANAGER_CPP
#define GALOTFA_MANAGER_CPP
#include "manager.h"
namespace galotfa {
// data API with potential tracer
int manager::sim_data( unsigned long pot_tracer_type, unsigned long particle_ids[],
                       unsigned int types[], double masses[], double coordiantes[][ 3 ],
                       double velocities[][ 3 ], double times[],
                       unsigned long particle_number ) const
{
    ;
    return 0;
}

// data API without potential tracer
int manager::sim_data( unsigned long particle_ids[], unsigned int types[], double masses[],
                       double coordiantes[][ 3 ], double velocities[][ 3 ], double times[],
                       unsigned long particle_number ) const
{
    ;
    return 0;
}

int manager::push_data()
{
    ;
    return 0;
}

int manager::collect()
{
    ;
    return 0;
}

int manager::write()
{
    ;
    return 0;
}

}  // namespace galotfa
#endif
