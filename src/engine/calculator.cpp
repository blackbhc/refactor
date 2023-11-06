#ifndef GALOTFA_CALCULATOR_CPP
#define GALOTFA_CALCULATOR_CPP
#include "calculator.h"
namespace galotfa {

calculator::calculator( galotfa::para& parameter ) : para( parameter )
{
    // TEST: allocate the output data pointer
}

calculator::~calculator()
{
    if ( this->active )
        this->stop();
}

int calculator::start() const
{
    // TODO: allocate some resources at here: large enough containers of the analysis results
    // TODO: set up some flags at here
    this->active = true;
    return 0;
}

int calculator::stop() const
{
    // TODO: release some resources at here
    // TODO: shuffle some flags at here
    this->active = false;
    return 0;
}

inline int shuffle()
{
    // TODO: to be implemented

    // TODO: shuffle the flag status
    return 0;
}

// receive data without potential tracer
int calculator::recv_data() const
{
    // TODO: to be implemented
    if ( !this->active )
        ERROR( "The analysis engine is not active!" );

    this->run_once();
    return 0;
}

// receive data with potential tracer
int calculator::recv_data_without_tracer() const
{
    // TODO: to be implemented
    if ( !this->active )
        ERROR( "The analysis engine is not active!" );

    this->run_once();
    return 0;
}

int calculator::pre_process() const
{
    // TODO: to be implemented
    ;
    return 0;
}

int calculator::model() const
{
    // TODO: to be implemented
    ;
    return 0;
}

int calculator::particle() const
{
    // TODO: to be implemented
    ;
    return 0;
}

int calculator::orbit_curve() const
{
    // TODO: to be implemented
    ;
    return 0;
}

int calculator::group() const
{
    // TODO: to be implemented
    ;
    return 0;
}

int calculator::post() const
{
    // TODO: to be implemented
    ;
    return 0;
}

inline int calculator::run_once() const
{
    static unsigned long long counter = 0;
    // TODO: call the analysis modules only at the steps specified by the parameter

    this->pre_process();
    this->model();
    this->particle();
    this->orbit_curve();
    this->group();
    this->post();

    ++counter;
    return 0;
}

std::vector< void* > calculator::feedback() const
{
    std::vector< void* > data_ptrs;
    return data_ptrs;
}

bool calculator::is_target_of_pre() const
{
    return true;
}
bool calculator::is_target_of_md() const
{
    return true;
}

}  // namespace galotfa
#endif
