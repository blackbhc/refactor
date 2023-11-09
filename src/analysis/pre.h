// The pre-process part
#ifndef GALOTFA_ANALYSIS_PRE_H
#define GALOTFA_ANALYSIS_PRE_H
namespace galotfa {
namespace analysis {
    int center_of_mass( unsigned long part_num, double masses[], double coords[][ 3 ],
                        double ( &center )[ 3 ] );

    // calculate the center of mass of the given array of particles

    int most_dense_pixel( unsigned long part_num, double coords[][ 3 ], double lower_bound_x,
                          double upper_bound_x, double lower_bound_y, double upper_bound_y,
                          double lower_bound_z, double upper_bound_z, unsigned int bin_num_x,
                          unsigned int bin_num_y, unsigned int bin_num_z, double ( &center )[ 3 ] );
    // calculate the most dense pixel of the given array of particles
}  // namespace analysis
}  // namespace galotfa


#ifdef debug_analysis
namespace unit_test {
int test_center_of_mass();
}
#endif
#endif
