#ifndef __GALOTFA_DEFAULT_CPP__
#define __GALOTFA_DEFAULT_CPP__
#include "para.h"
#include "../tools/prompt.h"
#include "ini-parser.h"
#ifdef debug_parameter
#include "ini-parser.cpp"
#endif

#define update( sec_alias, para, sec, type )                       \
    {                                                              \
        if ( parser.has( #sec, #para ) )                           \
            sec_alias##_##para = parser.get_##type( #sec, #para ); \
    }                                                              \
// here type is not the internal types but name of APIs of the ini_parser:
// get_bool, get_int, get_double, get_str, get_ints, get_doubles, get_strs
// e.g. update( gb, switch, global, bool ) will be
// gb_switch = parser.get_bool( "global", "switch" );

// TODO: the move constructor of the struct para
namespace galotfa {
para::para( ini_parser& parser )
{
    // update the parameters from the ini_parser
    // emm ... stupid hard code
    // I know it's ugly, but choose this way for 2 reason:
    // R1. To support the parameters with possible multiple values, it should be a vector, but if
    // there is only 1 value, the ini_parser has no way to know it's a 1-element vector or a single
    // value,
    // R2. ugly code but the fast running speed ...

    // global section
    update( this->gb, switch, global, bool );
    update( this->gb, output_dir, global, str );
    update( this->gb, particle_types, global, ints );
    update( this->gb, convergence_type, global, str );
    update( this->gb, convergence_threshold, global, double );
    update( this->gb, max_iter, global, int );
    update( this->gb, equal_threshold, global, double );
    update( this->gb, sim_type, global, str );
    update( this->gb, pot_tracer, global, int );

    // pre section
    update( this->pre, recenter, pre, bool );
    update( this->pre, region_shape, pre, str );
    update( this->pre, ratio, pre, double );
    update( this->pre, region_size, pre, double );
    update( this->pre, recenter_method, pre, str );
    update( this->pre, align_bar, pre, bool );

    // model section
    update( this->md, switch, model, bool );
    update( this->md, filename, model, str );
    update( this->md, period, model, int );
    update( this->md, region_shape, model, str );
    update( this->md, ratio, model, double );
    update( this->md, region_size, model, double );
    update( this->md, image, model, bool );
    update( this->md, image_bins, model, int );
    update( this->md, colors, model, strs );
    update( this->md, bar_major_axis, model, bool );
    update( this->md, bar_length, model, bool );
    update( this->md, sbar, model, bool );
    update( this->md, sbuckle, model, bool );
    update( this->md, inertia_tensor, model, bool );
    update( this->md, An, model, ints );

    // particle section
    update( this->ptc, switch, particle, bool );
    update( this->ptc, filename, particle, str );
    update( this->ptc, period, particle, int );
    update( this->ptc, circularity, particle, bool );
    update( this->ptc, circularity_3d, particle, bool );
    update( this->ptc, rg, particle, bool );
    update( this->ptc, freq, particle, bool );

    // orbit section
    update( this->orb, switch, orbit, bool );
    update( this->orb, filename, orbit, str );
    update( this->orb, period, orbit, int );
    update( this->orb, idfile, orbit, str );

    // group section
    update( this->grp, switch, group, bool );
    update( this->grp, filename, group, str );
    update( this->grp, period, group, int );
    update( this->grp, group_types, group, strs );
    update( this->grp, ellipticity, group, bool );
    update( this->grp, rmg, group, bool );
    update( this->grp, vmg, group, bool );

    // post section
    update( this->post, switch, post, bool );
    update( this->post, filename, post, str );
    update( this->post, pattern_speed, post, bool );
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

    // global section
    printi( gb, switch );
    prints( gb, output_dir );
    printis( gb, particle_types );
    prints( gb, convergence_type );
    printd( gb, convergence_threshold );
    printi( gb, max_iter );
    printd( gb, equal_threshold );
    prints( gb, sim_type );
    printi( gb, pot_tracer );

    // pre section
    printi( pre, recenter );
    prints( pre, region_shape );
    printd( pre, ratio );
    printd( pre, region_size );
    prints( pre, recenter_method );
    printi( pre, align_bar );

    // model section
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

    // particle section
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

    // group section
    printi( grp, switch );
    prints( grp, filename );
    printi( grp, period );
    printss( grp, group_types );
    printi( grp, ellipticity );
    printi( grp, rmg );
    printi( grp, vmg );

    // post section
    printi( post, switch );
    prints( post, filename );
    printi( post, pattern_speed );
    CHECK_RETURN( true );
}

#endif

}  // namespace galotfa
#endif
