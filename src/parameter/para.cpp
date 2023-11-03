#ifndef GALOTFA_DEFAULT_CPP
#define GALOTFA_DEFAULT_CPP
#include "para.h"
#include "../tools/prompt.h"
#include "ini_parser.h"
#ifdef debug_parameter
#include "ini_parser.cpp"
#endif

// here type is not the internal types but name of APIs of the ini_parser:
// get_bool, get_int, get_double, get_str, get_ints, get_doubles, get_strs
// e.g. update( gb, switch, Global, bool ) will be
// gb_switch = parser.get_bool( "Global", "switch" );
#define update( sec_alias, para, sec, type )                       \
    {                                                              \
        if ( parser.has( #sec, #para ) )                           \
            sec_alias##_##para = parser.get_##type( #sec, #para ); \
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

    // Global section
    update( this->gb, switch, Global, bool );
    update( this->gb, output_dir, Global, str );
    update( this->gb, particle_types, Global, ints );
    update( this->gb, convergence_type, Global, str );
    update( this->gb, convergence_threshold, Global, double );
    update( this->gb, max_iter, Global, int );
    update( this->gb, equal_threshold, Global, double );
    update( this->gb, sim_type, Global, str );
    update( this->gb, pot_tracer, Global, int );

    // Pre section
    update( this->pre, recenter, Pre, bool );
    update( this->pre, region_shape, Pre, str );
    update( this->pre, ratio, Pre, double );
    update( this->pre, region_size, Pre, double );
    update( this->pre, recenter_method, Pre, str );
    update( this->pre, align_bar, Pre, bool );

    // Model section
    update( this->md, switch, Model, bool );
    update( this->md, filename, Model, str );
    update( this->md, period, Model, int );
    update( this->md, region_shape, Model, str );
    update( this->md, ratio, Model, double );
    update( this->md, region_size, Model, double );
    update( this->md, image, Model, bool );
    update( this->md, image_bins, Model, int );
    update( this->md, colors, Model, strs );
    update( this->md, bar_major_axis, Model, bool );
    update( this->md, bar_length, Model, bool );
    update( this->md, sbar, Model, bool );
    update( this->md, sbuckle, Model, bool );
    update( this->md, inertia_tensor, Model, bool );
    update( this->md, An, Model, ints );

    // Particle section
    update( this->ptc, switch, Particle, bool );
    update( this->ptc, filename, Particle, str );
    update( this->ptc, period, Particle, int );
    update( this->ptc, circularity, Particle, bool );
    update( this->ptc, circularity_3d, Particle, bool );
    update( this->ptc, rg, Particle, bool );
    update( this->ptc, freq, Particle, bool );

    // orbit section
    update( this->orb, switch, orbit, bool );
    update( this->orb, filename, orbit, str );
    update( this->orb, period, orbit, int );
    update( this->orb, idfile, orbit, str );

    // Group section
    update( this->grp, switch, Group, bool );
    update( this->grp, filename, Group, str );
    update( this->grp, period, Group, int );
    update( this->grp, group_types, Group, strs );
    update( this->grp, ellipticity, Group, bool );
    update( this->grp, rmg, Group, bool );
    update( this->grp, vmg, Group, bool );

    // Post section
    update( this->post, switch, Post, bool );
    update( this->post, filename, Post, str );
    update( this->post, pattern_speed, Post, bool );
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
    // update the parameters from the ini_parser
    // emm ... stupid hard code
    // I know it's ugly, but choose this way for 2 reason:
    // R1. To support the parameters with possible multiple values, it should be a vector, but if
    // there is only 1 value, the ini_parser has no way to know it's a 1-element vector or a single
    // value,
    // R2. ugly code but the fast running speed ...

    // Global section
    printi( gb, switch );
    prints( gb, output_dir );
    printis( gb, particle_types );
    prints( gb, convergence_type );
    printd( gb, convergence_threshold );
    printi( gb, max_iter );
    printd( gb, equal_threshold );
    prints( gb, sim_type );
    printi( gb, pot_tracer );

    // Pre section
    printi( pre, recenter );
    prints( pre, region_shape );
    printd( pre, ratio );
    printd( pre, region_size );
    prints( pre, recenter_method );
    printi( pre, align_bar );

    // Model section
    printi( md, switch );
    prints( md, filename );
    printi( md, period );
    prints( md, region_shape );
    printd( md, ratio );
    printd( md, region_size );
    printi( md, image );
    printi( md, image_bins );
    printss( md, colors );
    printi( md, bar_major_axis );
    printi( md, bar_length );
    printi( md, sbar );
    printi( md, sbuckle );
    printi( md, inertia_tensor );
    printis( md, An );

    // Particle section
    printi( ptc, switch );
    prints( ptc, filename );
    printi( ptc, period );
    printi( ptc, circularity );
    printi( ptc, circularity_3d );
    printi( ptc, rg );
    printi( ptc, freq );

    // orbit section
    printi( orb, switch );
    prints( orb, filename );
    printi( orb, period );
    prints( orb, idfile );

    // Group section
    printi( grp, switch );
    prints( grp, filename );
    printi( grp, period );
    printss( grp, group_types );
    printi( grp, ellipticity );
    printi( grp, rmg );
    printi( grp, vmg );

    // Post section
    printi( post, switch );
    prints( post, filename );
    printi( post, pattern_speed );
    CHECK_RETURN( true );
}

#endif

}  // namespace galotfa
#endif
