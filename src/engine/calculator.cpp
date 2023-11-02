#ifndef GALOTFA_CALCULATOR_CPP
#define GALOTFA_CALCULATOR_CPP
#include "calculator.h"
namespace galotfa {
calculator::calculator()
{
    ;
}

calculator::~calculator()
{
    if ( this->active )
        this->stop();
}

int calculator::start() const
{
    // TODO: allocate some resources at here
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

int calculator::run()
{
    ;
    return 0;
}

}  // namespace galotfa
#endif
