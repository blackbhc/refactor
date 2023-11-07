#ifndef GALOTFA_ANALYSIS_UNTILS_H
#define GALOTFA_ANALYSIS_UNTILS_H
namespace galotfa {
namespace analysis {
    bool in_spheroid( double& coordx, double& coordy, double& coordz, double& size, double& ratio );

    bool in_box( double& coordx, double& coordy, double& coordz, double& size, double& ratio );

    bool in_cylinder( double& coordx, double& coordy, double& coordz, double& size, double& ratio );
}  // namespace analysis
}  // namespace galotfa

#ifdef debug_utils
namespace unit_test {
int test_in_spheroid( void );
int test_in_box( void );
int test_in_cylinder( void );
}  // namespace unit_test
#endif
#endif
