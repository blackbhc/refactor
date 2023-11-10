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
#define call_without_tracer                                                 \
    ( unsigned long particle_ids[], unsigned long types[], double masses[], \
      double coordinates[][ 3 ], double velocities[][ 3 ], double& time,    \
      unsigned long particle_number )

#define call_with_tracer                                                                  \
    ( unsigned long pot_tracer_type, unsigned long particle_ids[], unsigned long types[], \
      double masses[], double coordinates[][ 3 ], double velocities[][ 3 ], double& time, \
      unsigned long particle_number )

#define no_tracer ( particle_ids, types, masses, coordinates, velocities, time, particle_number )
#define has_tracer ( particle_ids, types, masses, coordinates, velocities, time, particle_number )
// TODO: use conditional compilation to deal the caller with potential tracer

namespace galotfa {

struct writer_integration
{
    vector< galotfa::writer* > model_writers;
    galotfa::writer*           particle_writer = nullptr;
    vector< galotfa::writer* > group_writers;
    galotfa::writer*           orbit_writer = nullptr;
};

class monitor
{
    // private members
private:
    int                  mpi_init_before_galotfa = 0;
    galotfa::para*       para                    = nullptr;  // pointer to the parameter class
    galotfa::calculator* calc                    = nullptr;
    unsigned long long   step = 0;  // the current step of the simulation/analysis
    // array of pointers to the writers: 5 possible output files
    // model, particle, orbit, group, post
    // nested vector of the data writers to be written: due to there may be multiple analysis
    // sets
    writer_integration       writers;
    int                      galotfa_rank;  // the global rank of the MPI process
    int                      galotfa_size;  // the global size of the MPI process
    vector< int >            include_particle_types;
    vector< vector< int >* > classifications;
    unsigned long orbit_part_num = 0;  // the number of target particles for orbital curve log
    vector< unsigned long >          orbit_log_ids;  // the particle ids for orbital curve log
    mutable vector< unsigned long* > id_for_model;   // similar but for model analysis section
    mutable vector< unsigned long >  part_num_model;
    mutable vector< unsigned long* > id_for_particle;  // simlar but for particle analysis
    mutable vector< unsigned long >  part_num_particle;
    mutable unsigned long*           id_for_orbit   = nullptr;  // similar but for orbital curve log
    mutable unsigned long            part_num_orbit = 0;
    // mutable unsigned long*           id_for_group   = nullptr;  // similar but for group
    // analysis mutable unsigned long            part_num_group = 0;

    // pointer to the analysis engine
    // private methods
private:
    // push the data to the analysis engine
    inline bool is_root() const
    {
        return this->galotfa_rank == 0;
    }

    // the push data API with potential tracer
    inline int push_data call_without_tracer const;
    inline int push_data call_with_tracer const;
    int                  create_writers();              // create the writers
    inline void          create_files();                // create the output files
    inline void          create_model_file_datasets();  // create the datasets in the model file
    void                 create_particle_file_datasets(
                        vector< unsigned long >& particle_ana_nums );  // create the datasets in the particle file
    inline void create_orbit_file_datasets();  // create the datasets in the orbit file
    inline void create_group_file_datasets();  // create the datasets in the group file
    inline void create_post_file_datasets();   // create the datasets in the post file
    int         save( void );                  // write the data to the output files
    inline void init();
    inline void check_filesize( long int size ) const;
    // extract the target particles from the simulation data
    void        extractor( unsigned long& partnum_total, unsigned long types[], unsigned long ids[],
                           double coordinates[][ 3 ] ) const;
    void        release_once() const;  // release the resource alloacted in extractor()
    inline bool need_ana_model() const;
    inline bool need_ana_particle() const;
    inline bool need_log_orbit() const;
    inline bool need_ana_group() const;
    inline bool need_ana_post() const;
    inline bool need_ana() const;

    // public methods
public:
    monitor( void );
    ~monitor();
    // interface of the simulation data: without potential tracer
    int run_with call_without_tracer;
    inline void  post_analysis();  // TODO: to be implemented
};
}  // namespace galotfa
#endif
