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
#include <complex>
#include <vector>
using std::complex;
using std::vector;

// I don't want to write this verbose argument list again and again ...
// In summary, the argument list is:
// id, type, mass, coordinate, velocity, time, particle_number
// and an optional potential tracer type
#define md_args                                                             \
    ( double masses[], double coordinates[][ 3 ], double velocities[][ 3 ], \
      vector< int* >& id_for_md, vector< int >& part_num_md )

namespace galotfa {

struct analysis_result
{
    // pre-process part
    double* system_center = nullptr;
    // model part, all support multiple analysis sets
    vector< double >            bar_marjor_axis;  // by argument of A2
    vector< double >            s_bar;
    vector< double >            s_buckle;
    vector< complex< double > > Ans[ 7 ];  // the An of each order, from 0 to 6
    // First dimension: the order of the An, from 0 to 6
    // Second dimension: the pointer to the data
    vector< double >  bar_length;  // TODO: to be implemented with multiple method
    vector< double* > images[ 8 ][ 3 ];
    // First dimension are over possible images colors: number_density, surface_density,
    // mean_velocity axis1, mean_velocity axis2, mean_velocity axis3, velocity_dispersion axis1,
    // velocity_dispersion axis2, velocity_dispersion axis3 dispersion
    // their indexes are 0, 1, 2, 3, 4, 5, 6, 7
    // Second dimension: the three projections, x-y, x-z, y-z
    // Third dimension: the pointer to the data
    vector< double* > dispersion_tensor;  // the pointer to the dispersion tensor
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
    method                recenter_method;        // the recenter method
    region_shape          recenter_region_shape;  // the recenter region shape
    region_shape          model_region_shape;
    analysis_result*      ptrs_of_results;
    vector< std::string > colors;  // I dont't know why I need this, but if I don't use c copy,
                                   // the call of this->para->model_colors will dump core

    // private methods
private:
    // the analysis wrappers: call the analysis modules, and restore the results
    inline bool in_recenter_region() const;
    void        setup_res();

public:
    calculator( galotfa::para* parameter );
    ~calculator();
    galotfa::analysis_result* feedback() const;
    bool is_target_of_pre( int& type, double& coordx, double& coordy, double& coordz ) const;
    bool is_target_of_md( int& type, double& coordx, double& coordy, double& coordz ) const;
    // the apis between the analysis engine and the real analysis codes
    // TODO: the version with the potential tracer for the following methods
    int                call_pre_module( int& particle_num, int types[], double masses[],
                                        double coordinates[][ 3 ] ) const;
    int call_md_module md_args const;
    int                call_ptc_module() const;
    int                call_orb_module() const;
    int                call_grp_module() const;
    int                call_post_module() const;
};

}  // namespace galotfa
#endif
