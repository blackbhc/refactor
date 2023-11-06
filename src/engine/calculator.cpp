#ifndef GALOTFA_CALCULATOR_CPP
#define GALOTFA_CALCULATOR_CPP
#include "calculator.h"
namespace galotfa {

calculator::calculator( galotfa::para& parameter ) : para( parameter )
{
    // TEST: allocate the output data pointer
}

inline int shuffle()
{
    // TODO: to be implemented

    // TODO: shuffle the flag status
    return 0;
}

int calculator::call_pre_module() const
{
    INFO( "Mock the behavior of pre-process module" );
    return 0;
}

int calculator::call_md_module() const
{
    INFO( "Mock the behavior of model analysis module" );
    return 0;
}

int calculator::call_ptc_module() const
{
    INFO( "Mock the behavior of particle analysis module" );
    return 0;
}

int calculator::call_orb_module() const
{
    INFO( "Mock the behavior of orbital curve log module" );
    return 0;
}

int calculator::call_grp_module() const
{
    INFO( "Mock the behavior of group analysis module" );
    return 0;
}

int calculator::call_post_module() const
{
    INFO( "Mock the behavior of post analysis module" );
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
