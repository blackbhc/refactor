// This header file define the analysis engine class, which will call the individual analysis
// modules
#ifndef GALOTFA_ANALYSIS_ENGINE_H
#define GALOTFA_ANALYSIS_ENGINE_H
// include the prompt header and parameter header
#include "../parameter/para.h"
#include "../tools/prompt.h"
// include the analysis modules
#include "../analysis/group.h"
#include "../analysis/model.h"
#include "../analysis/orbit_curve.h"
#include "../analysis/particle.h"
#include "../analysis/post.h"
#include "../analysis/pre.h"
// include the vector
#include <vector>
using std::vector;

// I don't want to write this verbose argument list again and again ...
// In summary, the argument list is:
// id, type, mass, coordinate, velocity, time, particle_number
// and an optional potential tracer type
#define md_args                                                             \
    ( double masses[], double coordiantes[][ 3 ], double velocities[][ 3 ], \
      unsigned long& particle_number, vector< unsigned long* >& id_for_pre, \
      vector< unsigned long >& part_num_pre )

namespace galotfa {

struct analysis_result
{
    double* system_center = nullptr;
};


class calculator
{
    // private members
private:
    // TODO: add the data container: pointers of the sim data, and dynamic arrays of the analysis
    // results
    galotfa::para* para;                   // the parameter object
    mutable double system_center[ 3 ];     // the container of the system center
    double         convergence_threshold;  // the convergence threshold
    enum method { center_of_mass, most_dense_pixel, most_bound_particle };  // the recenter method
    enum region_shape { sphere, cylinder, box };  // the recenter region shape
    method          recenter_method;              // the recenter method
    region_shape    recenter_region_shape;        // the recenter region shape
    region_shape    model_region_shape;
    analysis_result ptrs_of_results;

    // private methods
private:
    // the analysis wrappers: call the analysis modules, and restore the results
    inline int  shuffle();
    inline bool in_recenter_region() const;

public:
    calculator( galotfa::para* parameter );
    ~calculator() = default;
    galotfa::analysis_result feedback() const;
    bool                     is_target_of_pre( unsigned long& type, double& coordx, double& coordy,
                                               double& coordz ) const;
    bool                     is_target_of_md( unsigned long& type, double& coordx, double& coordy,
                                              double& coordz ) const;
    // the apis between the analysis engine and the real analysis codes
    // TODO: the version with the potential tracer for the following methods
    int call_pre_module( unsigned long types[], double masses[], double coordinates[][ 3 ],
                         unsigned long& particle_num ) const;
    int call_md_module md_args const;
    int                call_ptc_module() const;
    int                call_orb_module() const;
    int                call_grp_module() const;
    int                call_post_module() const;
};

}  // namespace galotfa
#endif
