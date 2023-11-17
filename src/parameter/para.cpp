#ifndef GALOTFA_DEFAULT_CPP
#define GALOTFA_DEFAULT_CPP
#include "para.h"
#include "../tools/prompt.h"
#include "../tools/string.h"
#include "ini_parser.h"
#ifdef debug_parameter
#include "ini_parser.cpp"
#endif

// here type is not the internal types but name of APIs of the ini_parser:
// get_bool, get_int, get_double, get_str, get_ints, get_doubles, get_strs
// e.g. update( glb, switch, Global, bool ) will be
// glb_switch_on = parser.get_bool( "Global", "switch" );
#define update( sec_alias, para, sec, type )                                                       \
    {                                                                                              \
        if ( parser.has( #sec, #para ) )                                                           \
            sec_alias##_##para = parser.get_##type( #sec, #para );                                 \
        else                                                                                       \
            INFO( "The parameter %s_%s is not given, use the default value.", #sec_alias, #para ); \
    }


// usage: if statement is true, print the warning message and let return_code += 1
#define IF_THEN_WARN( statement, ... ) \
    if ( statement )                   \
    {                                  \
        WARN( __VA_ARGS__ );           \
        return_code += 1;              \
    }

// similar, but for the iteratable object, and the judge should be callable function that return a
// bool
#define IF_ONE_THEN_WARN( iteratable, judge, ... ) \
    for ( auto& item : iteratable )                \
    {                                              \
        if ( judge( item ) )                       \
        {                                          \
            WARN( __VA_ARGS__ );                   \
            return_code += 1;                      \
            break;                                 \
        }                                          \
    }

namespace galotfa {
para::para( ini_parser& parser )
{
    // update the parameters from the ini_parser
    // emm ... stupid hard code
    // I know it's ugly, but choose this way for 2 reason:
    // R1. To support the parameters with possible multiple values, it should be a vector, but if
    // there is only 1 value, the ini_parser has no way to know it's a 1-element vector or a single
    // value.
    // PERF: R2. it's really ugly, but "results-oreinted" programming has the fast running speed ...

#ifndef debug_parameter
    // Global section
    update( this->glb, switch_on, Global, bool );
    if ( this->glb_switch_on )
    {
        INFO( "The global analysis is enabled." );
    }
    update( glb, output_dir, Global, str );
    update( glb, convergence_type, Global, str );
    update( glb, convergence_threshold, Global, double );
    update( glb, max_iter, Global, int );
    update( glb, equal_threshold, Global, double );
    update( glb, sim_type, Global, str );
    update( glb, pot_tracer, Global, int );

    // Pre section
    update( pre, recenter, Pre, bool );
    update( pre, recenter_anchors, Pre, ints );
    update( pre, region_shape, Pre, str );
    update( pre, axis_ratio, Pre, double );
    update( pre, region_size, Pre, double );
    update( pre, recenter_method, Pre, str );

    // Model section
    update( md, switch_on, Model, bool );
    update( md, filename, Model, str );
    update( md, period, Model, int );
    update( md, particle_types, Model, ints );
    update( md, multiple, Model, bool );
    update( md, classification, Model, strs );
    update( md, align_bar, Model, bool );
    update( md, region_shape, Model, str );
    update( md, axis_ratio, Model, double );
    update( md, region_size, Model, double );
    update( md, image, Model, bool );
    update( md, image_bins, Model, int );
    update( md, colors, Model, strs );
    update( md, bar_major_axis, Model, bool );
    update( md, bar_radius, Model, bool );
    update( md, deg, Model, double );
    update( md, percentage, Model, double );
    update( md, sbar, Model, bool );
    update( md, bar_threshold, Model, double );
    update( md, sbuckle, Model, bool );
    update( md, an, Model, ints );
    update( md, inertia_tensor, Model, bool );
    update( md, dispersion_tensor, Model, bool );

    // Particle section
    update( ptc, switch_on, Particle, bool );
    update( ptc, filename, Particle, str );
    update( ptc, period, Particle, int );
    update( ptc, particle_types, Particle, ints );
    update( ptc, circularity, Particle, bool );
    update( ptc, circularity_3d, Particle, bool );
    update( ptc, rg, Particle, bool );
    update( ptc, freq, Particle, bool );

    // orbit section
    update( orb, switch_on, Orbit, bool );
    update( orb, filename, Orbit, str );
    update( orb, period, Orbit, int );
    update( orb, idfile, Orbit, str );

    // Group section
    update( grp, switch_on, Group, bool );
    update( grp, filename, Group, str );
    update( grp, period, Group, int );
    update( grp, group_types, Group, strs );
    update( grp, ellipticity, Group, bool );
    update( grp, rmg, Group, bool );
    update( grp, vmg, Group, bool );

    // Post section
    update( post, switch_on, Post, bool );
    update( post, filename, Post, str );
    update( post, pattern_speed, Post, bool );

    // parser the analysis sets if the multiple sets analysis is enabled
    if ( this->md_multiple )
    {
        int return_code = this->target_sets_parser();
        if ( return_code != 0 )
        {
            ERROR( "Failed to parse the target sets, please check your ini file." );
        }
        else
        {
            INFO( "Get %d sets for analysis.", ( int )this->md_target_sets.size() );
            int counter = 0;
            for ( auto& subset : this->md_target_sets )
            {
                counter += 1;
                std::string subset_str = "";
                for ( auto& type : subset )
                    subset_str += std::to_string( type ) + "&";
                subset_str.pop_back();
                INFO( "Subset %d: %s", counter, subset_str.c_str() );
            }
        }
    }
    else
    {
        INFO( "Treating all the particle types as a single set for model analysis." );
        this->md_target_sets.push_back( this->md_particle_types );
        // push this into the target sets, to make the API more convenient
    }

    // check the dependencies between the parameters
    int return_code = this->check();
    if ( return_code != 0 )
    {
        ERROR( "There are conflicts between the parameters specified in the galotfa.ini file."
               "\nPlease check and modify your ini "
               "parameter file, may be there are typos such as an initial lowercase section "
               "name." );
    }
#else
    ( void )parser;  // avoid the warning of unused variable
#endif
}

inline int para::target_sets_parser()
{
    try
    {
        for ( std::string& subset : this->md_classification )
        {
            auto               substrs = galotfa::string::split( subset, "&\"" );
            std::vector< int > subset_types;
            for ( auto& str : substrs )
                subset_types.push_back( std::stoi( str ) );
            this->md_target_sets.push_back( subset_types );
        }
    }
    catch ( std::exception& e )
    {
        WARN( "Get unexpected runtime error: %s", e.what() );
        return 1;
    }
    return 0;
}

int para::check( void )
{
    // check the dependencies between the parameters
    int return_code = 0;
    if ( !this->glb_switch_on )
    {
        WARN( "Galotfa is disabled, no any analysis will be performed." );
        return return_code;
    }

    if ( this->glb_switch_on )  // check the global and pre-process section
    {
        // global part
        // use a local file macro: IF_THEN_WARN( statement, ... )
        IF_THEN_WARN( !this->md_switch_on && !this->ptc_switch_on && !this->orb_switch_on
                          && !this->grp_switch_on && !this->post_switch_on,
                      "On-the-fly analysis by galotfa is enabled, but no any analysis part is "
                      "activated." )

        IF_THEN_WARN( this->glb_convergence_type != "relative"
                          && this->glb_convergence_type != "absolute",
                      "The convergence type is unknown: %s."
                      "\nSupported value: relative or absolute",
                      this->glb_convergence_type.c_str() );

        IF_THEN_WARN( this->glb_convergence_threshold <= 0,
                      "The convergence threshold is non-positive, which is not allowed." )

        IF_THEN_WARN( this->glb_max_iter <= 0,
                      "The maximum iteration times is non-positive, which is not allowed." );

        IF_THEN_WARN( this->glb_equal_threshold <= 0,
                      "The equal threshold is non-positive, which is not allowed." );

        IF_THEN_WARN( this->glb_sim_type != "galaxy" && this->glb_sim_type != "cluster",
                      "The simulation type is unknown: %s."
                      "\nSupported value: galaxy, cluster, cosmology or cosmology_zoom_in.",
                      this->glb_sim_type.c_str() );

#ifdef GALOTFA_ENABLE_POT_TRACER
        IF_THEN_WARN( this->glb_pot_tracer == -10086,
                      "The potential tracer feature is enabled but the potential tracer's "
                      "particle type is not specified." );
#endif
        // pre-process part
        if ( this->pre_recenter )
        {
            IF_THEN_WARN( this->pre_recenter_anchors.size() == 0,
                          "Recenter system before anlysis is enabled, but the  recenter_anchor is "
                          "not given." );

            IF_THEN_WARN( this->pre_region_shape != "sphere" && this->pre_region_shape != "cylinder"
                              && this->pre_region_shape != "box",
                          "The region shape for pre-process is not supported, given: %s."
                          "\nSupported value: sphere, cylinder or box.",
                          this->pre_region_shape.c_str() );

            IF_THEN_WARN(
                this->pre_region_size <= 0,
                "The region size for pre-process region is non-positive, which is not allowed." )

            IF_THEN_WARN(
                this->pre_axis_ratio <= 0,
                "The axis ratio for pre-process region is non-positive, which is not allowed." )

            IF_THEN_WARN( this->pre_recenter_method != "com"
                              && this->pre_recenter_method != "density"
                              && this->pre_recenter_method != "potential",
                          "The recenter method is unknown: %s."
                          "\nSupported value: com, density or potential",
                          this->pre_recenter_method.c_str() );

            if ( this->pre_recenter_method == "potential" )
            {
                WARN( "The recenter method by potential is not supported yet. Use com instead." );
                this->pre_recenter_method = "com";
            }
        }
    }

    // check the model section
    if ( this->md_switch_on )
    {
        IF_THEN_WARN( this->md_align_bar && !this->pre_recenter,
                      "Try to align the x axis parallel to the bar major axis, "
                      "but the recenter is not activated." );

        // if need to align the bar major axis, then ensure the major axis is enabled
        if ( this->md_align_bar )
        {
            this->md_bar_major_axis = true;
            this->md_sbar           = true;
        }

        IF_THEN_WARN( this->md_period <= 0,
                      "The period of the model analysis is non-positive, which is not allowed." );

        IF_THEN_WARN( this->md_particle_types.size() == 0,
                      "Model level analysis is enabled, but the target particle types for model "
                      "analysis are not given." );

        IF_THEN_WARN( this->md_multiple && this->md_classification.size() == 0,
                      "The multiple sets analysis is enabled, but the target sets for analysis are "
                      "not given." );

        auto target_seubset_not_in_particle_types = [ this ]( std::vector< int >& subset ) -> bool {
            for ( auto& type_id : subset )
            {
                if ( std::find( this->md_particle_types.begin(), this->md_particle_types.end(),
                                type_id )
                     == this->md_particle_types.end() )
                    return true;
            }
            return false;
        };

        IF_ONE_THEN_WARN( this->md_target_sets, target_seubset_not_in_particle_types,
                          "Try to analysis multiple sets, but the target sets' particle types "
                          "are not include in \"particle_types\"." );

        IF_THEN_WARN( this->md_region_shape != "sphere" && this->md_region_shape != "cylinder"
                          && this->md_region_shape != "box",
                      "The region shape for model analysis is not supported, given: %s."
                      "\nSupported value: sphere, cylinder or box.",
                      this->md_region_shape.c_str() );

        IF_THEN_WARN(
            this->md_region_size <= 0,
            "The axis ratio for model analysis region is non-positive, which is not allowed." );

        IF_THEN_WARN(
            this->md_axis_ratio <= 0,
            "The axis ratio for model analysis region is non-positive, which is not allowed." );

        IF_THEN_WARN( this->md_bar_threshold <= 0 || this->md_bar_threshold >= 1,
                      "The bar threshold is not in the range of (0,1), which is not allowed." );

        if ( this->md_image )
        {
            IF_THEN_WARN(
                this->md_image_bins <= 0,
                "The number of bins for produce 2D image is non-positive, which is not allowed." );

            IF_THEN_WARN( this->md_colors.size() == 0,
                          "The image option is enabled, but none color is not specified." );

            auto invalid_color = []( std::string& color ) -> bool {
                return color != "number_density" && color != "surface_density"
                       && color != "mean_velocity" && color != "velocity_dispersion";
            };

            IF_ONE_THEN_WARN( this->md_colors, invalid_color,
                              "The image option is enabled, but the color option is not "
                              "given.\nPlease check your ini file." );

            auto invalid_m = []( int n ) -> bool { return n <= 0; };

            IF_ONE_THEN_WARN(
                this->md_an, invalid_m,
                "Try to calculate the amplitude of Fourier symmetry modes, but the order invalid: "
                "get a non-positive value.\n Please check your ini file." );
        }

        if ( this->md_bar_radius )
        {
            IF_THEN_WARN(
                this->md_deg <= 0 || this->md_deg >= 90,
                "The degree parameter for the Rbar1 is not in the range of (0,90), which is "
                "not allowed." );
            IF_THEN_WARN(
                this->md_percentage <= 0 || this->md_percentage >= 100,
                "The percentage parameter for the Rbar3 is not in the range of (0,100), which is "
                "not allowed." );
        }
    }

    // check the particle section
    if ( this->ptc_switch_on )
    {
        IF_THEN_WARN( this->ptc_period <= 0,
                      "The period of the particle analysis is non-positive, which is not allowed, "
                      "given value: %d",
                      this->ptc_period );

        IF_THEN_WARN( this->ptc_particle_types.size() == 0,
                      "Particle level analysis is enabled, but the target particle types for "
                      "particle analysis are not given." );

        if ( this->ptc_period < 1000 )
        {
            INFO( "\033[0;31mDangerous behavior!\033[0m"
                  "\nThe period of the particle analysis is too small (%d), "
                  "which may produce many snapshots."
                  "\nThis is not wrong, but make sure you know what you are doing.",
                  this->ptc_period );
        }
    }
    // check the orbit section
    if ( this->orb_switch_on )
    {
        IF_THEN_WARN( this->orb_period <= 0,
                      "The period of the orbit analysis is non-positive, which is not allowed." );

        IF_THEN_WARN( this->orb_idfile == "",
                      "The orbit curve log option is enabled, but \"idfile\" is not given in such "
                      "section,\nwhich is a id list filename to specify the "
                      "target particles to be logged." );
    }

    // check the group section
    if ( this->grp_switch_on )
    {
        IF_THEN_WARN( this->grp_period <= 0,
                      "The period of the group analysis is non-positive, which is not allowed." );

        IF_THEN_WARN( this->grp_group_types.size() == 0,
                      "The group analysis is enabled, but the target group types for analysis are "
                      "not given." );

        IF_THEN_WARN(
            this->grp_ellipticity,
            "The ellipticity option is enabled, but this feature is not supported at present." );

        IF_THEN_WARN( this->grp_rmg,
                      "The radial metallicity gradient option is enabled, but this feature "
                      "is not supported at present." );

        IF_THEN_WARN( this->grp_vmg,
                      "The vertical metallicity gradient option is enabled, but this feature "
                      "is not supported at present." );
    }

    return return_code;
}

#ifdef debug_parameter
#define printi( sec_alias, para )                                           \
    {                                                                       \
        println( #sec_alias "_" #para " = %d", ( int )sec_alias##_##para ); \
    }
#define printd( sec_alias, para )                                     \
    {                                                                 \
        println( #sec_alias "_" #para " = %lf", sec_alias##_##para ); \
    }
#define printis( sec_alias, para )           \
    {                                        \
        println( #sec_alias "_" #para ":" ); \
        for ( auto& i : sec_alias##_##para ) \
            println( "%d", i );              \
    }
#define printds( sec_alias, para )           \
    {                                        \
        println( #sec_alias "_" #para ":" ); \
        for ( auto& i : sec_alias##_##para ) \
            println( "%lf", i );             \
    }
#define prints( sec_alias, para )                                            \
    {                                                                        \
        println( #sec_alias "_" #para " = %s", sec_alias##_##para.c_str() ); \
    }
#define printss( sec_alias, para )           \
    {                                        \
        println( #sec_alias "_" #para ":" ); \
        for ( auto& i : sec_alias##_##para ) \
            println( "%s", i.c_str() );      \
    }

int para::test_print()
{

    // Global section
    printi( glb, switch_on );
    prints( glb, output_dir );
    prints( glb, convergence_type );
    printd( glb, convergence_threshold );
    printi( glb, max_iter );
    printd( glb, equal_threshold );
    prints( glb, sim_type );
    printi( glb, pot_tracer );

    // Pre section
    printi( pre, recenter );
    printis( pre, recenter_anchors );
    prints( pre, region_shape );
    printd( pre, axis_ratio );
    printd( pre, region_size );
    prints( pre, recenter_method );

    // Model section
    printi( md, switch_on );
    prints( md, filename );
    printi( md, period );
    printis( md, particle_types );
    printi( md, multiple );
    printss( md, classification );
    prints( md, region_shape );
    printd( md, axis_ratio );
    printd( md, region_size );
    printi( md, align_bar );
    printi( md, image );
    printi( md, image_bins );
    printss( md, colors );
    printi( md, bar_major_axis );
    printi( md, sbar );
    printd( md, bar_threshold );
    printi( md, bar_radius );
    printd( md, deg );
    printd( md, percentage );
    printi( md, sbuckle );
    printis( md, an );
    printi( md, inertia_tensor );
    printi( md, dispersion_tensor );

    // Particle section
    printi( ptc, switch_on );
    prints( ptc, filename );
    printi( ptc, period );
    printis( ptc, particle_types );
    printi( ptc, circularity );
    printi( ptc, circularity_3d );
    printi( ptc, rg );
    printi( ptc, freq );

    // orbit section
    printi( orb, switch_on );
    prints( orb, filename );
    printi( orb, period );
    prints( orb, idfile );

    // Group section
    printi( grp, switch_on );
    prints( grp, filename );
    printi( grp, period );
    printss( grp, group_types );
    printi( grp, ellipticity );
    printi( grp, rmg );
    printi( grp, vmg );

    // Post section
    printi( post, switch_on );
    prints( post, filename );
    printi( post, pattern_speed );

    CHECK_RETURN( true );
}

#endif

}  // namespace galotfa
#endif
