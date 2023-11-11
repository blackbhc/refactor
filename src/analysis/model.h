// The model level analysis part
#ifndef GALOTFA_ANALYSIS_MODEL_H
#define GALOTFA_ANALYSIS_MODEL_H
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

}  // namespace analysis
}  // namespace galotfa
#endif
