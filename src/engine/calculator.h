// This header file define the analysis engine class, which will call the individual analysis
// modules
#ifndef GALOTFA_ANALYSIS_ENGINE_H
#define GALOTFA_ANALYSIS_ENGINE_H
// include the analysis modules
#include "../analysis/group.h"
#include "../analysis/model.h"
#include "../analysis/orbit_curve.h"
#include "../analysis/particle.h"
#include "../analysis/post.h"
#include "../analysis/pre.h"

namespace galotfa {
class calculator
{
    // private members
private:
    mutable bool active = false;  // whether the analysis engine is active
    // public members
public:
    // private methods
private:
    // public methods
    int run();

public:
    calculator();
    ~calculator();
    int start() const;
    int stop() const;
};

}  // namespace galotfa
#endif
