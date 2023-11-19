// The particle level analysis functions
#ifndef GALOTFA_ANALYSIS_PARTICLE_H
#define GALOTFA_ANALYSIS_PARTICLE_H
namespace galotfa {
namespace analysis {
    int angular_momentum( int part_num, double masses[], double coords[][ 3 ], double vels[][ 3 ],
                          double ( &center )[ 3 ], double** angular_momentum );
}
}  // namespace galotfa

#ifdef debug_particle
#include "../tools/prompt.h"
#include <math.h>
namespace unit_test {
int test_angular_momentum();
}  // namespace unit_test
#endif
#endif
