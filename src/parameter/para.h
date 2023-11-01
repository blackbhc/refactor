// The file define the default value of the parameters
#ifndef __GALOTFA_DEFAULT_H__
#define __GALOTFA_DEFAULT_H__
#include "ini-parser.h"
#include <string>
#include <vector>
namespace galotfa {
struct para
{
    // the switchs of each part
    bool gb_switch = true, md_switch = false, ptc_switch = false, orb_switch = false,
         grp_switch = false, post_switch = false;
    // the file/path of each part
    std::string gb_output_dir = "./otfoutput", md_filename = "model", ptc_filename = "particle",
                orb_filename = "orbit", grp_filename = "group", post_filename = "post";
    // the analysis periods
    int md_period = 10, ptc_period = 10000, orb_period = 1, grp_period = 10;

    // other global parameters
    std::string        gb_convergence_type = "absolute", gb_sim_type = "galaxy";
    int                gb_pot_tracer, gb_max_iter                    = 25;
    std::vector< int > gb_particle_types;
    double             gb_convergence_threshold = 0.001, gb_equal_threshold = 1e-10;

    // pre section parameters
    bool        pre_recenter = true, pre_align_bar = true;
    double      pre_region_size = 20, pre_ratio = 1;
    std::string pre_region_shape = "cylinder", pre_recenter_method = "density";

    // other model section parameters
    bool md_image = false, md_bar_major_axis = false, md_bar_length = false, md_sbar = false,
         md_sbuckle = false, md_inertia_tensor = false;
    int                        md_image_bins  = 100;
    double                     md_region_size = 20, md_ratio = 1;
    std::string                md_region_shape = "cylinder";
    std::vector< int >         md_An;
    std::vector< std::string > md_colors;

    // other particle section parameters
    bool ptc_circularity = false, ptc_circularity_3d = false, ptc_rg = false, ptc_freq = false;

    // other orbit section parameters
    std::string orb_idfile;

    // other group section parameters
    bool                       grp_vmg = false, grp_rmg = false, grp_ellipticity = false;
    std::vector< std::string > grp_group_types;

    // other post section parameters
    bool post_pattern_speed = false;

    // constructor and destructor
    para( ini_parser& parser );
    ~para() = default;
#ifdef debug_parameter
    int test_print();
#endif
};

}  // namespace galotfa
#endif
