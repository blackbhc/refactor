// This header define the main manager of the on-the-fly analysis
#ifndef GALOTFA_MANAGER_H
#define GALOTFA_MANAGER_H
#include "../output/writer.h"
#include "../parameter/ini-parser.h"
#include "../parameter/para.h"
#include "calculator.h"

namespace galotfa {
class manager
{
    // private members
private:
    galotfa::para*       para   = nullptr;  // pointer to the parameter class
    galotfa::calculator* engine = nullptr;
    // pointer to the analysis engine
    // private methods
private:
    int push_data( void );  // push the data to the analysis engine
    int collect( void );    // collect the data from the analysis engine
    int write( void );      // write the data to the output files

    // public methods
public:
    manager( void )
    {
        galotfa::ini_parser ini( "./galotfa.ini" );
        this->para   = new galotfa::para( ini );
        this->engine = new galotfa::calculator();
    }

    ~manager()
    {
        if ( this->para != nullptr )
        {
            delete this->para;
            this->para = nullptr;
        }
        if ( this->engine != nullptr )
        {
            delete this->engine;
            this->engine = nullptr;
        }
    }
    // interface of the simulation data: all array
    int sim_data( unsigned long particle_ids[], unsigned int types[], double masses[],
                  double coordiantes[][ 3 ], double velocities[][ 3 ], double times[],
                  unsigned long particle_number ) const;

    int sim_data( unsigned long pot_tracer_type, unsigned long particle_ids[], unsigned int types[],
                  double masses[], double coordiantes[][ 3 ], double velocities[][ 3 ],
                  double times[], unsigned long particle_number ) const;
};
}  // namespace galotfa
#endif
