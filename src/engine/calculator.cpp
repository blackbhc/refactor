#ifndef GALOTFA_CALCULATOR_CPP
#define GALOTFA_CALCULATOR_CPP
#include "calculator.h"
#include "../analysis/utils.h"
#include <cstring>
namespace ana = galotfa::analysis;
namespace galotfa {

calculator::calculator( galotfa::para* parameter )
{
    // TEST: allocate the output data pointer
    this->para = parameter;
    if ( this->para->pre_recenter_method == "com" )
    {
        this->recenter_method = center_of_mass;
    }
    else
    {
        this->recenter_method = most_dense_pixel;
    }
    if ( this->para->pre_region_shape == "sphere" )
    {
        this->recenter_region_shape = sphere;
    }
    else if ( this->para->pre_region_shape == "cylinder" )
    {
        this->recenter_region_shape = cylinder;
    }
    else
    {
        this->recenter_region_shape = box;
    }

    this->ptrs_of_results = new analysis_result;
    this->setup_res();
}

calculator::~calculator()
{
    if ( this->para->md_switch_on && this->para->md_image )
    {
        for ( auto& color : this->colors )
        {
            if ( color == "number_density" )
            {
                for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                {
                    delete[] this->ptrs_of_results->images[ 0 ][ 0 ][ i ];
                    delete[] this->ptrs_of_results->images[ 0 ][ 1 ][ i ];
                    delete[] this->ptrs_of_results->images[ 0 ][ 2 ][ i ];
                }
            }
            else if ( color == "surface_density" )
            {
                for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                {
                    delete[] this->ptrs_of_results->images[ 1 ][ 0 ][ i ];
                    delete[] this->ptrs_of_results->images[ 1 ][ 1 ][ i ];
                    delete[] this->ptrs_of_results->images[ 1 ][ 2 ][ i ];
                }
            }
            else if ( color == "mean_velocity" )
            {
                for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    for ( int k = 2; k < 5; ++k )
                    {
                        delete[] this->ptrs_of_results->images[ k ][ 0 ][ i ];
                        delete[] this->ptrs_of_results->images[ k ][ 1 ][ i ];
                        delete[] this->ptrs_of_results->images[ k ][ 2 ][ i ];
                    }
            }
            else  // ( color == "velocity_dispersion" )
            {
                for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    for ( int k = 5; k < 8; ++k )
                    {
                        delete[] this->ptrs_of_results->images[ k ][ 0 ][ i ];
                        delete[] this->ptrs_of_results->images[ k ][ 1 ][ i ];
                        delete[] this->ptrs_of_results->images[ k ][ 2 ][ i ];
                    }
            }
        }
    }
    if ( this->para->md_dispersion_tensor )
    {
        for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
        {
            delete[] this->ptrs_of_results->dispersion_tensor[ i ];
        }
    }
    if ( this->para->md_inertia_tensor )
    {
        for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
        {
            delete[] this->ptrs_of_results->inertia_tensor[ i ];
        }
        this->ptrs_of_results->inertia_tensor.clear();
    }

    delete this->ptrs_of_results;
}

void calculator::setup_res()
{
    // assign the value of the pre-process part
    if ( this->para->pre_recenter )
    {
        this->ptrs_of_results->system_center = this->system_center;
    }
    // assign the value of the model analysis part
    if ( this->para->md_switch_on )
    {
        // set the vector of the results has size == the number of target analysis sets
        if ( this->para->md_bar_major_axis )
            this->ptrs_of_results->bar_major_axis.resize( this->para->md_target_sets.size() );
        if ( this->para->md_bar_radius )
        {
            this->ptrs_of_results->bar_radius.resize( this->para->md_target_sets.size() );
            for ( auto& radius_in_one_set : this->ptrs_of_results->bar_radius )
                radius_in_one_set.resize( 3 );
        }
        if ( this->para->md_sbar )
            this->ptrs_of_results->s_bar.resize( this->para->md_target_sets.size() );
        if ( this->para->md_sbuckle )
            this->ptrs_of_results->s_buckle.resize( this->para->md_target_sets.size() );
        if ( this->para->md_an.size() > 0 )
            for ( auto& n : this->para->md_an )
                this->ptrs_of_results->Ans[ n ].resize( this->para->md_target_sets.size() );
        if ( this->para->md_image )
        {
            this->colors             = this->para->md_colors;
            unsigned int base_binnum = this->para->md_image_bins;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;
            for ( auto& color : this->colors )
            {
                if ( color == "number_density" )
                {
                    for ( int j = 0; j < 3; ++j )
                        this->ptrs_of_results->images[ 0 ][ j ].resize(
                            this->para->md_target_sets.size() );
                    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    {
                        double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                        double* image_xz_ptr = new double[ base_binnum * third_binnum ];
                        double* image_yz_ptr = new double[ base_binnum * third_binnum ];
                        this->ptrs_of_results->images[ 0 ][ 0 ][ i ] = image_xy_ptr;
                        this->ptrs_of_results->images[ 0 ][ 1 ][ i ] = image_xz_ptr;
                        this->ptrs_of_results->images[ 0 ][ 2 ][ i ] = image_yz_ptr;
                    }
                }
                else if ( color == "surface_density" )
                {
                    for ( int j = 0; j < 3; ++j )
                        this->ptrs_of_results->images[ 1 ][ j ].resize(
                            this->para->md_target_sets.size() );
                    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    {
                        double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                        double* image_xz_ptr = new double[ base_binnum * third_binnum ];
                        double* image_yz_ptr = new double[ base_binnum * third_binnum ];
                        this->ptrs_of_results->images[ 1 ][ 0 ][ i ] = image_xy_ptr;
                        this->ptrs_of_results->images[ 1 ][ 1 ][ i ] = image_xz_ptr;
                        this->ptrs_of_results->images[ 1 ][ 2 ][ i ] = image_yz_ptr;
                    }
                }
                else if ( color == "mean_velocity" )
                {
                    for ( int j = 0; j < 3; ++j )
                        for ( int k = 2; k < 5; ++k )
                            this->ptrs_of_results->images[ k ][ j ].resize(
                                this->para->md_target_sets.size() );
                    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    {
                        for ( int k = 2; k < 5; ++k )
                        {
                            double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                            double* image_xz_ptr = new double[ base_binnum * third_binnum ];
                            double* image_yz_ptr = new double[ base_binnum * third_binnum ];
                            this->ptrs_of_results->images[ k ][ 0 ][ i ] = image_xy_ptr;
                            this->ptrs_of_results->images[ k ][ 1 ][ i ] = image_xz_ptr;
                            this->ptrs_of_results->images[ k ][ 2 ][ i ] = image_yz_ptr;
                        }
                    }
                }
                else  // ( color == "velocity_dispersion" )
                {
                    for ( int j = 0; j < 3; ++j )
                        for ( int k = 5; k < 8; ++k )
                            this->ptrs_of_results->images[ k ][ j ].resize(
                                this->para->md_target_sets.size() );
                    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
                    {
                        for ( int k = 5; k < 8; ++k )
                        {
                            double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                            double* image_xz_ptr = new double[ base_binnum * third_binnum ];
                            double* image_yz_ptr = new double[ base_binnum * third_binnum ];
                            this->ptrs_of_results->images[ k ][ 0 ][ i ] = image_xy_ptr;
                            this->ptrs_of_results->images[ k ][ 1 ][ i ] = image_xz_ptr;
                            this->ptrs_of_results->images[ k ][ 2 ][ i ] = image_yz_ptr;
                        }
                    }
                }
            }
        }
        if ( this->para->md_dispersion_tensor )
        {
            unsigned int base_binnum = this->para->md_image_bins;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;

            this->ptrs_of_results->dispersion_tensor.resize( this->para->md_target_sets.size() );
            for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
            {
                double* tensor_ptr = new double[ base_binnum * base_binnum * third_binnum * 3 * 3 ];
                this->ptrs_of_results->dispersion_tensor[ i ] = tensor_ptr;
            }
        }
        if ( this->para->md_inertia_tensor )
        {
            this->ptrs_of_results->inertia_tensor.resize( this->para->md_target_sets.size() );
            for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
            {
                double* tensor_ptr                         = new double[ 3 * 3 ];
                this->ptrs_of_results->inertia_tensor[ i ] = tensor_ptr;
            }
        }
    }
}

int calculator::call_pre_module( int& partnum_total, int types[], double masses[],
                                 double coordinates[][ 3 ] ) const
{
    vector< unsigned long* > id_for_pre;   // the array index of pre-process section's target
                                           // particles in the simulation data
    vector< unsigned long > part_num_pre;  // the length of the array index

    auto ids     = new unsigned long[ partnum_total ];
    int  counter = 0;  // how many particles have been used in this iteration
    for ( int j = 0; j < partnum_total; ++j )
    {
        if ( this->is_target_of_pre( types[ j ], coordinates[ j ][ 0 ], coordinates[ j ][ 1 ],
                                     coordinates[ j ][ 2 ] ) )
            ids[ counter++ ] = j;
    }
    double* anchor_masses         = new double[ counter ];
    double( *anchor_coords )[ 3 ] = new double[ counter ][ 3 ];
    // get the anchor particles' data
    for ( int j = 0; j < counter; ++j )
    {
        anchor_masses[ j ]      = masses[ ids[ j ] ];
        anchor_coords[ j ][ 0 ] = coordinates[ ids[ j ] ][ 0 ];
        anchor_coords[ j ][ 1 ] = coordinates[ ids[ j ] ][ 1 ];
        anchor_coords[ j ][ 2 ] = coordinates[ ids[ j ] ][ 2 ];
    }

    switch ( this->recenter_method )
    {
    case center_of_mass:
        for ( int i = 0; i < this->para->glb_max_iter; ++i )
        {
            // backup the old value of the system center
            double offset[ 3 ] = { 0 };
            offset[ 0 ]        = this->system_center[ 0 ];
            offset[ 1 ]        = this->system_center[ 1 ];
            offset[ 2 ]        = this->system_center[ 2 ];

            ana::center_of_mass( counter, anchor_masses, anchor_coords, this->system_center );

            offset[ 0 ] -= this->system_center[ 0 ];
            offset[ 1 ] -= this->system_center[ 1 ];
            offset[ 2 ] -= this->system_center[ 2 ];

            if ( this->para->glb_convergence_type == "relative" )
            {
                if ( ana::norm( offset ) / this->para->pre_region_size
                     <= this->para->glb_convergence_threshold )
                    break;
            }
            else
            {  // convergence type = absolute
                if ( ana::norm( offset ) <= this->para->glb_convergence_threshold )
                    break;
            }
        }
        break;
    case most_dense_pixel:
        for ( int i = 0; i < this->para->glb_max_iter; ++i )
        {
            // backup the old value of the system center
            double offset[ 3 ] = { 0 };
            offset[ 0 ]        = this->system_center[ 0 ];
            offset[ 1 ]        = this->system_center[ 1 ];
            offset[ 2 ]        = this->system_center[ 2 ];

            // For calculation convenience, always treate the shape as box (but the extracted data
            // may not, if pre_region_shape!="box")
            static unsigned int bin_num_x = this->para->md_image_bins;
            static unsigned int bin_num_y = bin_num_x;
            static unsigned int bin_num_z = ( unsigned int )bin_num_x * this->para->pre_axis_ratio;
            double lower_bound_x = this->system_center[ 0 ] - this->para->pre_region_size * 0.5;
            double upper_bound_x = this->system_center[ 0 ] + this->para->pre_region_size * 0.5;
            double lower_bound_y = this->system_center[ 1 ] - this->para->pre_region_size * 0.5;
            double upper_bound_y = this->system_center[ 1 ] + this->para->pre_region_size * 0.5;
            double lower_bound_z = this->system_center[ 1 ]
                                   - this->para->pre_region_size * 0.5 * this->para->pre_axis_ratio;
            double upper_bound_z = this->system_center[ 1 ]
                                   + this->para->pre_region_size * 0.5 * this->para->pre_axis_ratio;

            ana::most_dense_pixel( counter, anchor_coords, lower_bound_x, upper_bound_x,
                                   lower_bound_y, upper_bound_y, lower_bound_z, upper_bound_z,
                                   bin_num_x, bin_num_y, bin_num_z, this->system_center );

            offset[ 0 ] -= this->system_center[ 0 ];
            offset[ 1 ] -= this->system_center[ 1 ];
            offset[ 2 ] -= this->system_center[ 2 ];

            if ( this->para->glb_convergence_type == "relative" )
            {
                if ( ana::norm( offset ) / this->para->pre_region_size
                     <= this->para->glb_convergence_threshold )
                    break;
            }
            else
            {  // convergence type = absolute
                if ( ana::norm( offset ) <= this->para->glb_convergence_threshold )
                    break;
            }
        }
        break;
    case most_bound_particle:
        WARN( "Most boud partcile is not implemented yet." );
        break;
    }

    // release the memory
    delete[] ids;
    delete[] anchor_masses;
    delete[] anchor_coords;

    return 0;
}

int calculator::call_md_module md_args const
{
    // analysis of each target set
    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
    {
        // hte mass of the target set
        double* mass = new double[ part_num_md[ i ] ];
        // the coordinates used in the bin2d function
        double* x = new double[ part_num_md[ i ] ];
        double* y = new double[ part_num_md[ i ] ];
        double* z = new double[ part_num_md[ i ] ];
        double* vels[ 3 ];
        vels[ 0 ] = new double[ part_num_md[ i ] ];
        vels[ 1 ] = new double[ part_num_md[ i ] ];
        vels[ 2 ] = new double[ part_num_md[ i ] ];
        // extract the data of the target set
        for ( int j = 0; j < ( int )part_num_md[ i ]; ++j )
        {
            {
                x[ j ] = coordinates[ id_for_md[ i ][ j ] ][ 0 ] - this->system_center[ 0 ];
                y[ j ] = coordinates[ id_for_md[ i ][ j ] ][ 1 ] - this->system_center[ 1 ];
                z[ j ] = coordinates[ id_for_md[ i ][ j ] ][ 2 ]
                         - this->system_center[ 2 ] * this->para->md_axis_ratio;
                vels[ 0 ][ j ] = velocities[ id_for_md[ i ][ j ] ][ 0 ];
                vels[ 1 ][ j ] = velocities[ id_for_md[ i ][ j ] ][ 1 ];
                vels[ 2 ][ j ] = velocities[ id_for_md[ i ][ j ] ][ 2 ];
                mass[ j ]      = masses[ id_for_md[ i ][ j ] ];
            }
        }
        if ( this->para->md_bar_major_axis )
            this->ptrs_of_results->bar_major_axis[ i ] =
                ana::bar_major_axis( part_num_md[ i ], mass, x, y );
        if ( this->para->md_sbar )
            this->ptrs_of_results->s_bar[ i ] = ana::s_bar( part_num_md[ i ], mass, x, y );
        if ( this->para->md_sbuckle )
            this->ptrs_of_results->s_buckle[ i ] = ana::s_buckle( part_num_md[ i ], mass, x, y, z );
        if ( this->para->md_an.size() > 0 )
            for ( auto& n : this->para->md_an )
            {
                this->ptrs_of_results->Ans[ n ][ i ] = ana::An( part_num_md[ i ], mass, x, y, n );
            }

        if ( this->para->md_bar_radius )  // must call this after s_bar
        {
            if ( this->ptrs_of_results->s_bar[ i ] >= this->para->md_bar_threshold )
            {
                static double rmin = this->para->md_rmin, rmax = this->para->md_rmax;
                static int    rbins = this->para->md_rbins;
                // only calculate the bar radius when the bar is strong enough
                ana::bar_radius( part_num_md[ i ], mass, x, y, rmin, rmax, rbins,
                                 this->ptrs_of_results->bar_major_axis[ i ], this->para->md_deg,
                                 this->para->md_percentage,
                                 this->ptrs_of_results->bar_radius[ i ].data() );
            }
            else
            {
                this->ptrs_of_results->bar_radius[ i ][ 0 ] = 0;
                this->ptrs_of_results->bar_radius[ i ][ 1 ] = 0;
                this->ptrs_of_results->bar_radius[ i ][ 2 ] = 0;
            }
        }

        if ( this->para->md_align_bar
             && this->ptrs_of_results->s_bar[ i ] > this->para->md_bar_threshold )
        {
            // rotate the coordinates to align the bar
            double phi = -this->ptrs_of_results->bar_major_axis[ i ];
            // minus sign: passively rotate the coordinates
            double _x, _y;  // tmp variables
            for ( int j = 0; j < part_num_md[ i ]; ++j )
            {
                _x             = x[ j ];
                _y             = y[ j ];
                x[ j ]         = _x * cos( phi ) - _y * sin( phi );
                y[ j ]         = _x * sin( phi ) + _y * cos( phi );
                _x             = vels[ 0 ][ j ];
                _y             = vels[ 1 ][ j ];
                vels[ 0 ][ j ] = _x * cos( phi ) - _y * sin( phi );
                vels[ 1 ][ j ] = _x * sin( phi ) + _y * cos( phi );
            }
        }

        if ( this->para->md_image )
        {
            double       base_size   = this->para->md_region_size;
            unsigned int base_binnum = this->para->md_image_bins;
            double       third_size  = base_size * this->para->md_axis_ratio;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;

            unsigned int k, m, n;  // tmp variables for the loop
            for ( auto& color : this->colors )
            {
                if ( color == "number_density" )
                {
                    auto image_xy =
                        ana::bin2d( part_num_md[ i ], x, y, x, -base_size, base_size, -base_size,
                                    base_size, base_binnum, base_binnum, ana::stats_method::count );
                    auto image_xz = ana::bin2d( part_num_md[ i ], x, z, x, -base_size, base_size,
                                                -third_size, third_size, base_binnum, third_binnum,
                                                ana::stats_method::count );
                    auto image_yz = ana::bin2d( part_num_md[ i ], y, z, x, -base_size, base_size,
                                                -third_size, third_size, base_binnum, third_binnum,
                                                ana::stats_method::count );
                    for ( k = 0; k < base_binnum; ++k )
                        for ( m = 0; m < third_binnum; ++m )
                        {
                            this->ptrs_of_results->images[ 0 ][ 0 ][ i ][ k * base_binnum + m ] =
                                image_xy[ k ][ m ];
                            this->ptrs_of_results->images[ 0 ][ 1 ][ i ][ k * base_binnum + m ] =
                                image_xz[ k ][ m ];
                            this->ptrs_of_results->images[ 0 ][ 2 ][ i ][ k * base_binnum + m ] =
                                image_yz[ k ][ m ];
                        }
                }
                else if ( color == "surface_density" )
                {
                    static double area_xy = 4 * base_size * base_size / base_binnum / base_binnum;
                    static double area_xz = 4 * base_size * third_size / base_binnum / third_binnum;
                    static double area_yz = 4 * base_size * third_size / base_binnum / third_binnum;
                    auto          image_xy =
                        ana::bin2d( part_num_md[ i ], x, y, mass, -base_size, base_size, -base_size,
                                    base_size, base_binnum, base_binnum, ana::stats_method::sum );
                    auto image_xz = ana::bin2d( part_num_md[ i ], x, z, mass, -base_size, base_size,
                                                -third_size, third_size, base_binnum, third_binnum,
                                                ana::stats_method::sum );
                    auto image_yz = ana::bin2d( part_num_md[ i ], y, z, mass, -base_size, base_size,
                                                -third_size, third_size, base_binnum, third_binnum,
                                                ana::stats_method::sum );
                    for ( k = 0; k < base_binnum; ++k )
                        for ( m = 0; m < third_binnum; ++m )
                        {
                            this->ptrs_of_results->images[ 1 ][ 0 ][ i ][ k * base_binnum + m ] =
                                image_xy[ k ][ m ] / area_xy;
                            this->ptrs_of_results->images[ 1 ][ 1 ][ i ][ k * base_binnum + m ] =
                                image_xz[ k ][ m ] / area_xz;
                            this->ptrs_of_results->images[ 1 ][ 2 ][ i ][ k * base_binnum + m ] =
                                image_yz[ k ][ m ] / area_yz;
                        }
                }
                else if ( color == "mean_velocity" )
                {
                    for ( n = 0; n < 3; ++n )
                    {
                        auto image_xy = ana::bin2d( part_num_md[ i ], x, y, vels[ n ], -base_size,
                                                    base_size, -base_size, base_size, base_binnum,
                                                    base_binnum, ana::stats_method::mean );
                        auto image_xz = ana::bin2d( part_num_md[ i ], x, z, vels[ n ], -base_size,
                                                    base_size, -third_size, third_size, base_binnum,
                                                    third_binnum, ana::stats_method::mean );
                        auto image_yz = ana::bin2d( part_num_md[ i ], y, z, vels[ n ], -base_size,
                                                    base_size, -third_size, third_size, base_binnum,
                                                    third_binnum, ana::stats_method::mean );
                        for ( k = 0; k < base_binnum; ++k )
                            for ( m = 0; m < third_binnum; ++m )
                            {
                                this->ptrs_of_results
                                    ->images[ 2 + n ][ 0 ][ i ][ k * base_binnum + m ] =
                                    image_xy[ k ][ m ];
                                this->ptrs_of_results
                                    ->images[ 2 + n ][ 1 ][ i ][ k * base_binnum + m ] =
                                    image_xz[ k ][ m ];
                                this->ptrs_of_results
                                    ->images[ 2 + n ][ 2 ][ i ][ k * base_binnum + m ] =
                                    image_yz[ k ][ m ];
                            }
                    }
                }
                else  // ( color == "velocity_dispersion" )
                {
                    for ( n = 0; n < 3; ++n )
                    {
                        auto image_xy = ana::bin2d( part_num_md[ i ], x, y, vels[ n ], -base_size,
                                                    base_size, -base_size, base_size, base_binnum,
                                                    base_binnum, ana::stats_method::std );
                        auto image_xz = ana::bin2d( part_num_md[ i ], x, z, vels[ n ], -base_size,
                                                    base_size, -third_size, third_size, base_binnum,
                                                    third_binnum, ana::stats_method::std );
                        auto image_yz = ana::bin2d( part_num_md[ i ], y, z, vels[ n ], -base_size,
                                                    base_size, -third_size, third_size, base_binnum,
                                                    third_binnum, ana::stats_method::std );
                        for ( k = 0; k < base_binnum; ++k )
                            for ( m = 0; m < third_binnum; ++m )
                            {
                                this->ptrs_of_results
                                    ->images[ 5 + n ][ 0 ][ i ][ k * base_binnum + m ] =
                                    image_xy[ k ][ m ];
                                this->ptrs_of_results
                                    ->images[ 5 + n ][ 1 ][ i ][ k * base_binnum + m ] =
                                    image_xz[ k ][ m ];
                                this->ptrs_of_results
                                    ->images[ 5 + n ][ 2 ][ i ][ k * base_binnum + m ] =
                                    image_yz[ k ][ m ];
                            }
                    }
                }
            }
        }
        if ( this->para->md_dispersion_tensor )
        {
            double       base_size   = this->para->md_region_size;
            unsigned int base_binnum = this->para->md_image_bins;
            double       third_size  = base_size * this->para->md_axis_ratio;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;
            if ( this->para->md_region_shape == "box" )
            {
                ana::dispersion_tensor( part_num_md[ i ], x, y, z, vels[ 0 ], vels[ 1 ], vels[ 2 ],
                                        -base_size, base_size, -base_size, base_size, -third_size,
                                        third_size, base_binnum, base_binnum, third_binnum,
                                        this->ptrs_of_results->dispersion_tensor[ i ] );
            }
            else if ( this->para->md_region_shape == "sphere" )
            {
                double* v_r     = new double[ part_num_md[ i ] ];
                double* v_phi   = new double[ part_num_md[ i ] ];
                double* v_theta = new double[ part_num_md[ i ] ];
                for ( int j = 0; j < part_num_md[ i ]; ++j )
                {
                    v_r[ j ] = ( vels[ 0 ][ j ] * x[ j ] + vels[ 1 ][ j ] * y[ j ]
                                 + vels[ 2 ][ j ] * z[ j ] )
                               / sqrt( x[ j ] * x[ j ] + y[ j ] * y[ j ] + z[ j ] * z[ j ] );
                    v_phi[ j ] = ( -vels[ 0 ][ j ] * y[ j ] + vels[ 1 ][ j ] * x[ j ] )
                                 / sqrt( x[ j ] * x[ j ] + y[ j ] * y[ j ] );
                    v_theta[ j ] = ( -vels[ 0 ][ j ] * z[ j ]
                                     + vels[ 2 ][ j ] * ( x[ j ] * x[ j ] + y[ j ] * y[ j ] ) )
                                   / sqrt( x[ j ] * x[ j ] + y[ j ] * y[ j ] + z[ j ] * z[ j ] );
                }
                ana::dispersion_tensor( part_num_md[ i ], x, y, z, v_r, v_phi, v_theta, -base_size,
                                        base_size, -base_size, base_size, -third_size, third_size,
                                        base_binnum, base_binnum, third_binnum,
                                        this->ptrs_of_results->dispersion_tensor[ i ] );
                delete[] v_r;
                delete[] v_phi;
                delete[] v_theta;
            }
            else  // ( this->para->md_region_shape == "cylinder" )
            {

                double* v_R   = new double[ part_num_md[ i ] ];
                double* v_phi = new double[ part_num_md[ i ] ];
                for ( int j = 0; j < part_num_md[ i ]; ++j )
                {
                    v_R[ j ] = ( vels[ 0 ][ j ] * x[ j ] + vels[ 1 ][ j ] * y[ j ] )
                               / sqrt( x[ j ] * x[ j ] + y[ j ] * y[ j ] );
                    v_phi[ j ] = ( -vels[ 0 ][ j ] * y[ j ] + vels[ 1 ][ j ] * x[ j ] )
                                 / sqrt( x[ j ] * x[ j ] + y[ j ] * y[ j ] );
                }
                ana::dispersion_tensor( part_num_md[ i ], x, y, z, v_R, v_phi, vels[ 2 ],
                                        -base_size, base_size, -base_size, base_size, -third_size,
                                        third_size, base_binnum, base_binnum, third_binnum,
                                        this->ptrs_of_results->dispersion_tensor[ i ] );
                delete[] v_R;
                delete[] v_phi;
            }
        }
        if ( this->para->md_inertia_tensor )
        {
            ana::inertia_tensor( part_num_md[ i ], mass, x, y, z,
                                 this->ptrs_of_results->inertia_tensor[ i ] );
        }
        // release the memory
        delete[] x;
        delete[] y;
        delete[] z;
        for ( int i = 0; i < 3; ++i )
            delete[] vels[ i ];
        delete[] mass;
    }
    return 0;
}

int calculator::call_ptc_module() const
{
    INFO( "Mock the behavior of particle analysis module." );
    return 0;
}

int calculator::call_orb_module() const
{
    INFO( "Mock the behavior of orbital curve log module." );
    return 0;
}

int calculator::call_grp_module() const
{
    INFO( "Mock the behavior of group analysis module." );
    return 0;
}

int calculator::call_post_module() const
{
    INFO( "Mock the behavior of post analysis module." );
    return 0;
}

galotfa::analysis_result* calculator::feedback() const
{
    return this->ptrs_of_results;
}

bool calculator::is_target_of_pre( int& type, double& coordx, double& coordy, double& coordz ) const
{
    double offset[ 3 ] = { 0 };  // the offset w.r.t. the system centers
    offset[ 0 ]        = coordx - this->system_center[ 0 ];
    offset[ 1 ]        = coordy - this->system_center[ 1 ];
    offset[ 2 ]        = coordz - this->system_center[ 2 ];

    // check whether the particle type is in the target list
    if ( std::find( this->para->pre_recenter_anchors.begin(),
                    this->para->pre_recenter_anchors.end(), type )
         == this->para->pre_recenter_anchors.end() )
        return false;

    switch ( this->recenter_region_shape )
    {
    case sphere:
        if ( !ana::in_spheroid( offset, this->para->pre_region_size, this->para->pre_axis_ratio ) )
            return false;
        break;
    case box:
        if ( !ana::in_box( offset, this->para->pre_region_size, this->para->pre_axis_ratio ) )
            return false;
        break;
    case cylinder:
        if ( !ana::in_cylinder( offset, this->para->pre_region_size, this->para->pre_axis_ratio ) )
            return false;
        break;
    }
    return true;
}

bool calculator::is_target_of_md( int& type, double& coordx, double& coordy, double& coordz ) const
{
    double offset[ 3 ] = { 0 };  // the offset w.r.t. the system centers
    offset[ 0 ]        = coordx - this->system_center[ 0 ];
    offset[ 1 ]        = coordy - this->system_center[ 1 ];
    offset[ 2 ]        = coordz - this->system_center[ 2 ];

    // check whether the particle type is in the target list
    if ( std::find( this->para->md_particle_types.begin(), this->para->md_particle_types.end(),
                    type )
         == this->para->md_particle_types.end() )
        return false;

    switch ( this->model_region_shape )
    {
    case sphere:
        if ( !ana::in_spheroid( offset, this->para->md_region_size, this->para->md_axis_ratio ) )
            return false;
        break;
    case box:
        if ( !ana::in_box( offset, this->para->md_region_size, this->para->md_axis_ratio ) )
            return false;
        break;
    case cylinder:
        if ( !ana::in_cylinder( offset, this->para->md_region_size, this->para->md_axis_ratio ) )
            return false;
        break;
    }
    return true;
}

}  // namespace galotfa
#endif
