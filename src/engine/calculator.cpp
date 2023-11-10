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
    }
}

inline int shuffle()
{
    // TODO: to be implemented

    // TODO: shuffle the flag status
    return 0;
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
