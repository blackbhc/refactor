// The model level analysis part
#ifndef GALOTFA_ANALYSIS_MODEL_H
#define GALOTFA_ANALYSIS_MODEL_H
#include "utils.h"
#include <complex>
#include <math.h>
#include <string>
using std::complex;
namespace galotfa {
namespace analysis {
    complex< double > An( unsigned int array_len, double mass[], double x[], double y[],
                          unsigned int order );

    double s_bar( unsigned int array_len, double mass[], double x[], double y[] );

    double s_buckle( unsigned int array_len, double mass[], double x[], double y[], double z[] );

    double bar_major_axis( unsigned int array_len, double mass[], double x[], double y[] );

    double bar_length( unsigned int array_len, double mass[], double x[], double y[] );

    int dispersion_tensor( unsigned int array_len, double x[], double y[], double z[], double vx[],
                           double vy[], double vz[], double lower_bound_x, double upper_bound_x,
                           double lower_bound_y, double upper_bound_y, double lower_bound_z,
                           double upper_bound_z, unsigned int num_bins_x, unsigned int num_bins_y,
                           unsigned int num_bins_z, double* tensor );

}  // namespace analysis
}  // namespace galotfa
#endif
