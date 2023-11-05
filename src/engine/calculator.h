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

// I don't want to write this verbose argument list again and again ...
// In summary, the argument list is:
// id, type, mass, coordinate, velocity, time, particle_number
// and an optional potential tracer type

namespace galotfa {
class calculator
{
    // private members
private:
    // TODO: add the data container: pointers of the sim data, and dynamic arrays of the analysis
    // results
    mutable bool   active = false;         // whether the analysis engine is active
    galotfa::para& para;                   // the parameter object
    double*        output_data = nullptr;  // TEST: the output data

    // private methods
private:
    // the analysis wrappers: call the analysis modules, and restore the results
    int        pre_process() const;
    int        model() const;
    int        particle() const;
    int        orbit_curve() const;
    int        group() const;
    int        post() const;
    inline int shuffle();

public:
    calculator( galotfa::para& parameter );
    ~calculator();
    inline bool is_active() const
    {
        return this->active;
    }
    int        start() const;
    int        stop() const;
    inline int run_once() const;  // a wrapper of the analysis modules
    // receive data without potential tracer
    int recv_data( void ) const;
    // receive data with potential tracer
    int                  recv_data_without_tracer( void ) const;
    std::vector< void* > feedback() const;
    bool                 is_target_of_pre() const;
    bool                 is_target_of_md() const;
};

}  // namespace galotfa
#endif
