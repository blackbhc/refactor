// This header define the main monitor of the on-the-fly analysis
#ifndef GALOTFA_MONITOR_H
#define GALOTFA_MONITOR_H
#include "../output/writer.h"
#include "../parameter/ini_parser.h"
#include "../parameter/para.h"
#include "calculator.h"
#include <mpi.h>
#include <vector>

using std::vector;

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
    unsigned long long   step   = 0;  // the current step of the simulation/analysis
    // array of pointers to the writers: 5 possible output files
    // model, particle, orbit, group, post
    // nested vector of the data writers to be written: due to there may be multiple analysis sets
    vector< vector< galotfa::writer* > > vec_of_writers;
    int                                  galotfa_rank;  // the global rank of the MPI process
    int                                  galotfa_size;  // the global size of the MPI process
    vector< int >                        include_particle_types;
    vector< vector< int >* >             classifications;
    vector< unsigned long >              particle_ids;  // the particle ids for orbital curve log

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
    int         create_writers();              // create the writers
    inline void create_files();                // create the output files
    inline void create_model_file_datasets();  // create the datasets in the model file
    void        create_particle_file_datasets(
               vector< unsigned long >& particle_ana_nums );  // create the datasets in the particle file
    inline void create_orbit_file_datasets();  // create the datasets in the orbit file
    inline void create_group_file_datasets();  // create the datasets in the group file
    inline void create_post_file_datasets();   // create the datasets in the post file
    int         save( void );                  // write the data to the output files
    inline void init();
    inline void check_filesize( long int size ) const;

    // public methods
public:
    monitor( void );

    ~monitor();
    // interface of the simulation data: without potential tracer
    inline int run_with call_without_tracer
    {
        ++this->step;
        if ( this->step == 1 )
        {
            // TODO: extract the target particles at here!
            vector< unsigned long > particle_ana_nums( this->para->ptc_particle_types.size(), 1 );
            this->create_particle_file_datasets( particle_ana_nums );
        }

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
};
}  // namespace galotfa
#endif
