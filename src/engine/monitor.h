// This header define the main monitor of the on-the-fly analysis
#ifndef GALOTFA_MONITOR_H
#define GALOTFA_MONITOR_H
#include "../output/writer.h"
#include "../parameter/ini_parser.h"
#include "../parameter/para.h"
#include "calculator.h"
#include <mpi.h>
#include <vector>

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
class monitor
{
    // private members
private:
    galotfa::para*       para   = nullptr;  // pointer to the parameter class
    galotfa::calculator* engine = nullptr;
    // array of pointers to the writers: 5 possible output files
    // model, particle, orbit, group, post
    // TODO: may by create a separate writer for each possible group classfiication in the future?
    galotfa::writer* writers[ 5 ];
    int              galotfa_rank;  // the global rank of the MPI process
    int              galotfa_size;  // the global size of the MPI process
    // pointer to the analysis engine
    // private methods
private:
    // push the data to the analysis engine
    inline bool is_root() const
    {
        return this->galotfa_rank == 0;
    }
    inline int push_data call_without_tracer const
    {
        this->engine->recv_data no_tracer;
        return 0;
    };
    // the push data API with potential tracer
    inline int push_data call_with_tracer const
    {
        this->engine->recv_data has_tracer;
        return 0;
    };
    int         create_writers();  // create the writers
    int         save( void );      // write the data to the output files
    inline void init();

    // public methods
public:
    monitor( void );

    ~monitor();
    // interface of the simulation data: without potential tracer
    inline int run_with call_without_tracer
    {
        if ( this->para->glb_switch_on )
        {
            push_data no_tracer;
            int       return_code = this->save();
            if ( return_code != 0 )
            {
                WARN( "Failed to save the data to the output files." );
                return return_code;
            }
        }
        return 0;
    };


    // interface of the simulation data: with potential tracer
    inline int run_with call_with_tracer
    {
        push_data has_tracer;
        return 0;
    };
};
}  // namespace galotfa
#endif
