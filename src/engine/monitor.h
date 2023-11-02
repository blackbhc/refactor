// This header define the main monitor of the on-the-fly analysis
#ifndef GALOTFA_MONITOR_H
#define GALOTFA_MONITOR_H
#include "../output/writer.h"
#include "../parameter/ini-parser.h"
#include "../parameter/para.h"
#include "calculator.h"
#include <vector>

namespace galotfa {
class monitor
{
    // private members
private:
    galotfa::para*                  para   = nullptr;  // pointer to the parameter class
    galotfa::calculator*            engine = nullptr;
    std::vector< galotfa::writer* > writers;  // the writers
    // pointer to the analysis engine
    // private methods
private:
    // push the data to the analysis engine
    inline int push_data call_without_tracer const;
    // the push data API with potential tracer
    inline int push_data call_with_tracer const;
    int                  create_writers();  // create the writers
    int                  save( void );      // write the data to the output files

    // public methods
public:
    monitor( void );

    ~monitor();
    // interface of the simulation data: without potential tracer
    inline int run_with call_without_tracer;

    // interface of the simulation data: with potential tracer
    inline int run_with call_with_tracer;
};
}  // namespace galotfa
#endif
