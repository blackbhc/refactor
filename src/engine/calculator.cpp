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

    // assign the value of the ptrs_of_results
    if ( this->para->pre_recenter )
    {
        this->ptrs_of_results.system_center = this->system_center;
    }
}

inline int shuffle()
{
    // TODO: to be implemented

    // TODO: shuffle the flag status
    return 0;
}

int calculator::call_pre_module( unsigned long types[], double masses[], double coordinates[][ 3 ],
                                 unsigned long& partnum_total ) const
{
    vector< unsigned long* > id_for_pre;   // the array index of pre-process section's target
                                           // particles in the simulation data
    vector< unsigned long > part_num_pre;  // the length of the array index
    switch ( this->recenter_method )
    {
    case center_of_mass:
        for ( int i = 0; i < this->para->glb_max_iter; ++i )
        {
            auto          ids     = new unsigned long[ partnum_total ];
            unsigned long counter = 0;  // how many particles have been used in this iteration
            for ( int j = 0; j < partnum_total; ++j )
            {
                if ( this->is_target_of_pre( types[ j ], coordinates[ j ][ 0 ],
                                             coordinates[ j ][ 1 ], coordinates[ j ][ 2 ] ) )
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

            // backup the old value of the system center
            double offset[ 3 ] = { 0 };
            offset[ 0 ]        = this->system_center[ 0 ];
            offset[ 1 ]        = this->system_center[ 1 ];
            offset[ 2 ]        = this->system_center[ 2 ];

            ana::center_of_mass( counter, anchor_masses, anchor_coords, this->system_center );

            // release the memory
            delete[] ids;
            delete[] anchor_masses;
            delete[] anchor_coords;


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
        // TODO: to be implemented
        break;
    case most_bound_particle:
        WARN( "Most boud partcile is not implemented yet." );
        break;
    }
    return 0;
}

int calculator::call_md_module md_args const
{
    INFO( "Mock the behavior of model analysis module." );
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

galotfa::analysis_result calculator::feedback() const
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
        if ( !ana::in_spheroid( offset, this->para->pre_region_size,
                                this->para->pre_region_ratio ) )
            return false;
        break;
    case box:
        if ( !ana::in_box( offset, this->para->pre_region_size, this->para->pre_region_ratio ) )
            return false;
        break;
    case cylinder:
        if ( !ana::in_cylinder( offset, this->para->pre_region_size,
                                this->para->pre_region_ratio ) )
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
        if ( !ana::in_spheroid( offset, this->para->md_region_size, this->para->md_region_ratio ) )
            return false;
        break;
    case box:
        if ( !ana::in_box( offset, this->para->md_region_size, this->para->md_region_ratio ) )
            return false;
        break;
    case cylinder:
        if ( !ana::in_cylinder( offset, this->para->md_region_size, this->para->md_region_ratio ) )
            return false;
        break;
    }
    return true;
}

}  // namespace galotfa
#endif
