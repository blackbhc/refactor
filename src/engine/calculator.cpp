#ifndef GALOTFA_CALCULATOR_CPP
#define GALOTFA_CALCULATOR_CPP
#include "calculator.h"
#include "../analysis/utils.h"
namespace ana = galotfa::analysis;
namespace galotfa {

calculator::calculator( galotfa::para* parameter ) : para( parameter )
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
        REACH;
        REACH;
        if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                        "number_density" )
             != this->para->md_colors.end() )
        {
            REACH;
            for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
            {
                delete[] this->ptrs_of_results->images[ 0 ][ 0 ][ i ];
                delete[] this->ptrs_of_results->images[ 0 ][ 1 ][ i ];
                delete[] this->ptrs_of_results->images[ 0 ][ 2 ][ i ];
            }
            REACH;
        }
        if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                        "surface_density" )
             != this->para->md_colors.end() )
        {
            for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
            {
                delete[] this->ptrs_of_results->images[ 1 ][ 0 ][ i ];
                delete[] this->ptrs_of_results->images[ 1 ][ 1 ][ i ];
                delete[] this->ptrs_of_results->images[ 1 ][ 2 ][ i ];
            }
        }
        if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                        "mean_velocity" )
             != this->para->md_colors.end() )
        {
            for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                for ( int k = 2; k < 5; ++k )
                {
                    delete[] this->ptrs_of_results->images[ k ][ 0 ][ i ];
                    delete[] this->ptrs_of_results->images[ k ][ 1 ][ i ];
                    delete[] this->ptrs_of_results->images[ k ][ 2 ][ i ];
                }
        }
        if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                        "velocity_dispersion" )
             != this->para->md_colors.end() )
        {
            for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                for ( int k = 5; k < 8; ++k )
                {
                    delete[] this->ptrs_of_results->images[ k ][ 0 ][ i ];
                    delete[] this->ptrs_of_results->images[ k ][ 1 ][ i ];
                    delete[] this->ptrs_of_results->images[ k ][ 2 ][ i ];
                }
        }
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
            this->ptrs_of_results->bar_marjor_axis.resize( this->para->md_target_sets.size() );
        if ( this->para->md_bar_length )
            this->ptrs_of_results->bar_length.resize( this->para->md_target_sets.size() );
        if ( this->para->md_sbar )
            this->ptrs_of_results->s_bar.resize( this->para->md_target_sets.size() );
        if ( this->para->md_sbuckle )
            this->ptrs_of_results->s_buckle.resize( this->para->md_target_sets.size() );
        if ( this->para->md_an.size() > 0 )
            for ( auto& n : this->para->md_an )
                this->ptrs_of_results->Ans[ n ].resize( this->para->md_target_sets.size() );
        if ( this->para->md_image )
        {

            double       base_size   = this->para->md_region_size;
            unsigned int base_binnum = this->para->md_image_bins;
            double       third_size  = base_size * this->para->md_axis_ratio;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;
            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "number_density" )
                 != this->para->md_colors.end() )
            {
                for ( int j = 0; j < 3; ++j )
                    this->ptrs_of_results->images[ 0 ][ j ].resize(
                        this->para->md_target_sets.size() );
                for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                {
                    double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                    double* image_xz_ptr = new double[ base_binnum * base_binnum ];
                    double* image_yz_ptr = new double[ base_binnum * base_binnum ];
                    this->ptrs_of_results->images[ 0 ][ 0 ][ i ] = image_xy_ptr;
                    this->ptrs_of_results->images[ 0 ][ 1 ][ i ] = image_xz_ptr;
                    this->ptrs_of_results->images[ 0 ][ 2 ][ i ] = image_yz_ptr;
                }
            }
            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "surface_density" )
                 != this->para->md_colors.end() )
            {
                for ( int j = 0; j < 3; ++j )
                    this->ptrs_of_results->images[ 1 ][ j ].resize(
                        this->para->md_target_sets.size() );
                for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                {
                    double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                    double* image_xz_ptr = new double[ base_binnum * base_binnum ];
                    double* image_yz_ptr = new double[ base_binnum * base_binnum ];
                    this->ptrs_of_results->images[ 1 ][ 0 ][ i ] = image_xy_ptr;
                    this->ptrs_of_results->images[ 1 ][ 1 ][ i ] = image_xz_ptr;
                    this->ptrs_of_results->images[ 1 ][ 2 ][ i ] = image_yz_ptr;
                }
            }
            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "mean_velocity" )
                 != this->para->md_colors.end() )
            {
                for ( int j = 0; j < 3; ++j )
                    for ( int k = 2; k < 5; ++k )
                        this->ptrs_of_results->images[ k ][ j ].resize(
                            this->para->md_target_sets.size() );
                for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                {
                    for ( int k = 2; k < 5; ++k )
                    {
                        double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                        double* image_xz_ptr = new double[ base_binnum * base_binnum ];
                        double* image_yz_ptr = new double[ base_binnum * base_binnum ];
                        this->ptrs_of_results->images[ k ][ 0 ][ i ] = image_xy_ptr;
                        this->ptrs_of_results->images[ k ][ 1 ][ i ] = image_xz_ptr;
                        this->ptrs_of_results->images[ k ][ 2 ][ i ] = image_yz_ptr;
                    }
                }
            }
            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "velocity_dispersion" )
                 != this->para->md_colors.end() )
            {
                for ( int j = 0; j < 3; ++j )
                    for ( int k = 5; k < 8; ++k )
                        this->ptrs_of_results->images[ k ][ j ].resize(
                            this->para->md_target_sets.size() );
                for ( size_t i = 0; i < this->para->md_target_sets.size(); i++ )
                {
                    for ( int k = 5; k < 8; ++k )
                    {
                        double* image_xy_ptr = new double[ base_binnum * base_binnum ];
                        double* image_xz_ptr = new double[ base_binnum * base_binnum ];
                        double* image_yz_ptr = new double[ base_binnum * base_binnum ];
                        this->ptrs_of_results->images[ k ][ 0 ][ i ] = image_xy_ptr;
                        this->ptrs_of_results->images[ k ][ 1 ][ i ] = image_xz_ptr;
                        this->ptrs_of_results->images[ k ][ 2 ][ i ] = image_yz_ptr;
                    }
                }
            }
        }

        if ( this->para->md_dispersion_tensor )
            this->ptrs_of_results->dispersion_tensor.resize( this->para->md_target_sets.size() );
    }
}

int calculator::call_pre_module( unsigned long& partnum_total, unsigned long types[],
                                 double masses[], double coordinates[][ 3 ] ) const
{
    vector< unsigned long* > id_for_pre;   // the array index of pre-process section's target
                                           // particles in the simulation data
    vector< unsigned long > part_num_pre;  // the length of the array index

    auto          ids     = new unsigned long[ partnum_total ];
    unsigned long counter = 0;  // how many particles have been used in this iteration
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
    for ( size_t i = 0; i < this->para->md_target_sets.size(); ++i )
    {  // analysis of each target set
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
        for ( int j = 0; j < ( int )part_num_md[ j ]; ++j )
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
            this->ptrs_of_results->bar_marjor_axis[ i ] =
                ana::bar_major_axis( part_num_md[ i ], mass, x, y );
        if ( this->para->md_bar_length )
            this->ptrs_of_results->bar_length[ i ] =
                ana::bar_length( part_num_md[ i ], mass, x, y );  // TODO: to be implemented
        if ( this->para->md_sbar )
            this->ptrs_of_results->s_bar[ i ] = ana::s_bar( part_num_md[ i ], mass, x, y );
        if ( this->para->md_sbuckle )
            this->ptrs_of_results->s_buckle[ i ] = ana::s_buckle( part_num_md[ i ], mass, x, y, z );
        if ( this->para->md_an.size() > 0 )
            for ( auto& n : this->para->md_an )
            {
                this->ptrs_of_results->Ans[ n ][ i ] = ana::An( part_num_md[ i ], mass, x, y, n );
            }

        if ( this->para->md_align_bar
             && this->ptrs_of_results->s_bar[ i ] > this->para->md_bar_threshold )
        {
            double phi = this->ptrs_of_results->bar_marjor_axis[ i ];
            double _x, _y;  // tmp variables
            for ( int j = 0; j < part_num_md[ i ]; ++j )
            {
                _x     = x[ j ];
                _y     = x[ j ];
                x[ j ] = _x * cos( phi ) - _y * sin( phi );
                y[ j ] = _x * sin( phi ) + _y * cos( phi );
            }
        }

        if ( this->para->md_image )
        {
            double       base_size   = this->para->md_region_size;
            unsigned int base_binnum = this->para->md_image_bins;
            double       third_size  = base_size * this->para->md_axis_ratio;
            unsigned int third_binnum =
                ( unsigned int )this->para->md_image_bins * this->para->md_axis_ratio;

            int k, m, n;  // tmp variables for the loop

            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "number_density" )
                 != this->para->md_colors.end() )
            {
                auto image_xy =
                    ana::bin2d( part_num_md[ i ], x, y, x, -base_size, base_size, -base_size,
                                base_size, base_binnum, base_binnum, ana::stats_method::count );
                auto image_xz =
                    ana::bin2d( part_num_md[ i ], x, z, x, -base_size, base_size, -third_size,
                                third_size, base_binnum, third_binnum, ana::stats_method::count );
                auto image_yz =
                    ana::bin2d( part_num_md[ i ], y, z, x, -base_size, base_size, -third_size,
                                third_size, base_binnum, third_binnum, ana::stats_method::count );
                for ( k = 0; k < base_binnum; k++ )
                    for ( m = 0; m < third_binnum; m++ )
                    {
                        this->ptrs_of_results->images[ 0 ][ 0 ][ i ][ k * base_binnum + m ] =
                            image_xy[ k ][ m ];
                        this->ptrs_of_results->images[ 0 ][ 1 ][ i ][ k * base_binnum + m ] =
                            image_xz[ k ][ m ];
                        this->ptrs_of_results->images[ 0 ][ 2 ][ i ][ k * base_binnum + m ] =
                            image_yz[ k ][ m ];
                    }
            }

            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "surface_density" )
                 != this->para->md_colors.end() )
            {
                static double area_xy = 4 * base_size * base_size / base_binnum / base_binnum;
                static double area_xz = 4 * base_size * third_size / base_binnum / third_binnum;
                static double area_yz = 4 * base_size * third_size / base_binnum / third_binnum;
                auto          image_xy =
                    ana::bin2d( part_num_md[ i ], x, y, mass, -base_size, base_size, -base_size,
                                base_size, base_binnum, base_binnum, ana::stats_method::sum );
                auto image_xz =
                    ana::bin2d( part_num_md[ i ], x, z, mass, -base_size, base_size, -third_size,
                                third_size, base_binnum, third_binnum, ana::stats_method::sum );
                auto image_yz =
                    ana::bin2d( part_num_md[ i ], y, z, mass, -base_size, base_size, -third_size,
                                third_size, base_binnum, third_binnum, ana::stats_method::sum );
                for ( k = 0; k < base_binnum; k++ )
                    for ( m = 0; m < third_binnum; m++ )
                    {
                        this->ptrs_of_results->images[ 1 ][ 0 ][ i ][ k * base_binnum + m ] =
                            image_xy[ k ][ m ] / area_xy;
                        this->ptrs_of_results->images[ 1 ][ 1 ][ i ][ k * base_binnum + m ] =
                            image_xz[ k ][ m ] / area_xz;
                        this->ptrs_of_results->images[ 1 ][ 2 ][ i ][ k * base_binnum + m ] =
                            image_yz[ k ][ m ] / area_yz;
                    }
            }

            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "mean_velocity" )
                 != this->para->md_colors.end() )
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
                    for ( k = 0; k < base_binnum; k++ )
                        for ( m = 0; m < third_binnum; m++ )
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

            if ( std::find( this->para->md_colors.begin(), this->para->md_colors.end(),
                            "velocity_dispersion" )
                 != this->para->md_colors.end() )
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
                    for ( k = 0; k < base_binnum; k++ )
                        for ( m = 0; m < third_binnum; m++ )
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

bool calculator::is_target_of_pre( unsigned long& type, double& coordx, double& coordy,
                                   double& coordz ) const
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

bool calculator::is_target_of_md( unsigned long& type, double& coordx, double& coordy,
                                  double& coordz ) const
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
