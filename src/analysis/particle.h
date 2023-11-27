// The particle level analysis functions
#ifndef GALOTFA_ANALYSIS_PARTICLE_H
#define GALOTFA_ANALYSIS_PARTICLE_H
namespace galotfa {
namespace analysis {
    int angular_momentum( int part_num, double masses[], double coords[][ 3 ], double vels[][ 3 ],
                          double ( &center )[ 3 ], double** angular_momentum );
    int circularity( int part_num, double coords[][ 3 ], double vels[][ 3 ],
                     double ( &center )[ 3 ], double circularity[] );
    int circularity_3d( int part_num, double coords[][ 3 ], double vels[][ 3 ],
                        double ( &center )[ 3 ], double circularity_3d[] );
}  // namespace analysis
}  // namespace galotfa

#ifdef debug_particle
#include "../tools/prompt.h"
#include <math.h>
namespace unit_test {
int test_angular_momentum();
int test_circularity();
int test_circularity_3d();
}  // namespace unit_test
#endif
#endif
