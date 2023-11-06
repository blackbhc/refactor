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
    galotfa::para& para;  // the parameter object

    // private methods
private:
    // the analysis wrappers: call the analysis modules, and restore the results
    inline int shuffle();

public:
    calculator( galotfa::para& parameter );
    ~calculator() = default;
    std::vector< void* > feedback() const;
    bool                 is_target_of_pre() const;
    bool                 is_target_of_md() const;
    // the apis between the analysis engine and the real analysis codes
    int call_pre_module() const;
    int call_md_module() const;
    int call_ptc_module() const;
    int call_orb_module() const;
    int call_grp_module() const;
    int call_post_module() const;
};

}  // namespace galotfa
#endif
