/*******************************************************************************
 * \copyright   This file is part of the GADGET4 N-body/SPH code developed
 * \copyright   by Volker Springel. Copyright (C) 2014-2020 by Volker Springel
 * \copyright   (vspringel@mpa-garching.mpg.de) and all contributing authors.
 *******************************************************************************/

/*! \file  run.cc
 *
 *  \brief contains the basic simulation loop that iterates over timesteps
 */

// clang-format off
#include "gadgetconfig.h"
// clang-format on

#include <ctype.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../cooling_sfr/cooling.h"
#include "../data/allvars.h"
#include "../data/dtypes.h"
#include "../data/mymalloc.h"
#include "../domain/domain.h"
#include "../gravtree/gravtree.h"
#include "../io/io.h"
#include "../io/snap_io.h"
#include "../lightcone/lightcone_massmap_io.h"
#include "../lightcone/lightcone_particle_io.h"
#include "../logs/logs.h"
#include "../main/main.h"
#include "../main/simulation.h"
#include "../ngbtree/ngbtree.h"
#include "../sort/parallel_sort.h"
#include "../system/system.h"

// galotfa
#ifdef GALOTFA_ON
#include <galotfa.h>
#endif

/*!
 * Main driver routine for advancing the simulation forward in time.
 * The loop terminates when the cpu-time limit is reached, when a `stop' file
 * is found in the output directory, or when the simulation ends because we
 * arrived at TimeMax.
 *
 * If the simulation is started from initial conditions, an initial domain
 * decomposition is performed, the gravitational forces are computed and
 * initial hydro forces are calculated.
 */
void sim::run(void)
{
  // call the galotfa api
#ifdef ZERO_MASS_POT_TRACER
  void write_potential_tracers(char filename[], double potentials[], double positions[][3], int ids[], double &time,
                               int &num);  // the function to write potential tracers to hdf5 file, only called by the root rank
  void collect_potential_tracers(double localPot[], double localPos[][3], int localIDs[], double globalPot[], double globalPos[][3],
                                 int globalIDs[], int &localNum, int &globalNum, int &rank, int &size);
  MyReal(*initPos)[3];                     // backup of the initial positions of the potential tracer particles in global process
                                           // aim: avoid the numerical error of the positions correction which is multiple
                                           // summation of the position shift (a small number) in the double precision
  initPos = new MyReal[Sp.TotNumPart][3];  // copy of the init positions of the potential tracers, release by OS when the program ends
  int firstIDofPotTracer = INT_MAX;        // the starting ID of the potential tracer particles in global process
  // global arrays of the particles' data
  double(*posGlobal)[3];
  double *potGlobal;
  int *pIDsGlobal;
  if(ThisTask == 0)
    {
      posGlobal  = new double[Sp.TotNumPart][3];  // release by OS when the program ends
      potGlobal  = new double[Sp.TotNumPart];
      pIDsGlobal = new int[Sp.TotNumPart];
    }
  else
    {
      posGlobal  = nullptr;
      potGlobal  = nullptr;
      pIDsGlobal = nullptr;
    }
  // arrays used to recenter the potential tracer particles to the center of mass of the system
  double pos[3]          = {0, 0, 0};        // a temporary array used for coordinate transformation
  MyIntPosType intpos[3] = {0, 0, 0};        // unsigned integer positions
  double centerOfMass[3] = {0.0, 0.0, 0.0};  // center of mass
  double offset          = 0.0;              // offset for the particle positions w.r.t. the center of mass
  // initialize the center of mass
  double comNumerator[3] = {0.0, 0.0, 0.0};                                      // local sum of positions
  double comDenominator  = 0.0;                                                  // local sum of Mass
  double oldValue[3]     = {centerOfMass[0], centerOfMass[1], centerOfMass[2]};  // old center of mass
#endif

#if defined(NGENIC_TEST) && defined(PERIODIC) && defined(PMGRID)
  snap_io Snap(&Sp, Communicator, All.SnapFormat);             /* get an I/O object */
  Snap.write_snapshot(All.SnapshotFileCount, NORMAL_SNAPSHOT); /* write snapshot file */
#if defined(POWERSPEC_ON_OUTPUT)
  PM.calculate_power_spectra(All.SnapshotFileCount);
#endif
  return;
#endif

  while(1) /* main loop over synchronization points */
    {
      /* store old time for logging purposes */
      All.TimeOld = All.Time;

      /* determine the next synchronization time at which we have active particles */
      integertime ti_next_kick_global = Sp.find_next_sync_point();

#ifdef OUTPUT_NON_SYNCHRONIZED_ALLOWED
      while(ti_next_kick_global > All.Ti_nextoutput && All.Ti_nextoutput >= 0)
        {
          All.Ti_Current = All.Ti_nextoutput;
          All.Time       = All.get_absolutetime_from_integertime(All.Ti_Current);
          All.set_cosmo_factors_for_current_time();

          Sp.drift_all_particles();
          create_snapshot_if_desired();
        }
#endif

      All.Ti_Current = ti_next_kick_global;
      All.Time       = All.get_absolutetime_from_integertime(All.Ti_Current);
      All.set_cosmo_factors_for_current_time();
      All.TimeStep = All.Time - All.TimeOld;

#ifdef LIGHTCONE
#ifdef LIGHTCONE_PARTICLES
      mpi_printf("LIGHTCONE_PARTICLES: Lp.NumPart=%d\n", Lp.NumPart);
#endif
#ifdef LIGHTCONE_MASSMAPS
      mpi_printf("LIGHTCONE_MASSMAPS:  Mp.NumPart=%d \n", Mp.NumPart);
#endif
#if defined(LIGHTCONE_MASSMAPS) || defined(LIGHTCONE_PARTICLES_GROUPS)
      Sp.drift_all_particles();  // we do this here to be able to avoid large buffer sizes, if needed multiple binning operations are
                                 // done
#endif
#endif

      /* mark the timebins that are active on this step */
      Sp.mark_active_timebins();

      /* create lists with the particles that are synchronized on this step */
      Sp.make_list_of_active_particles();

      /* produce some further log messages */
      Logs.output_log_messages();

      /* call functions that update certain 'extra' physics settings to new current time */
      set_non_standard_physics_for_current_time();

      /* for sufficiently large steps, carry out a new domain decomposition */
      if(All.HighestActiveTimeBin >= All.SmallestTimeBinWithDomainDecomposition)
        {
          NgbTree.treefree();
          Domain.domain_free();

          Sp.drift_all_particles();

#ifdef LIGHTCONE_PARTICLES
          LightCone.lightcone_clear_boxlist(All.Time);
#endif

#ifdef DEBUG_MD5
          Logs.log_debug_md5("C");
#endif
          Domain.domain_decomposition(STANDARD);

#ifdef DEBUG_MD5
          Logs.log_debug_md5("D");
#endif

          NgbTree.treeallocate(Sp.NumGas, &Sp, &Domain);
          NgbTree.treebuild(Sp.NumGas, NULL);
        }

      /* compute SPH densities and smoothing lengths for active SPH particles, and optionally those
       * accessed passively. This also creates the list of active hydro particles at this
       * synchronization point, which is stored in the list TimeBinsHydro.ActiveParticleList[].
       * This list is reused for the subsequent second and first hydro step. */
      NgbTree.compute_densities();

      /* if particles have increased their smoothing lengths, this is recorded in parent tree nodes */
      NgbTree.update_maxhsml();

      /* hydro-forces, second half-step. This will also update the predicted velocities/entropies with the new current ones */
      do_hydro_step_second_half();

      /* this does the closing gravity half-step for the timebins that end at the current synchronization point */
      do_gravity_step_second_half();

      /* do any extra physics, in a Strang-split way, for the timesteps that are finished */
      calculate_non_standard_physics_end_of_step();

#ifdef DEBUG_MD5
      Logs.log_debug_md5("A");
#endif

      Logs.compute_statistics();

      Logs.flush_everything();

#ifdef DEBUG_MD5
      Logs.log_debug_md5("BEFORE SNAP");
#endif
      create_snapshot_if_desired();

#ifdef DEBUG_MD5
      Logs.log_debug_md5("AFTER SNAP");
#endif

      if(All.Ti_Current >= TIMEBASE || All.Time > All.TimeMax) /* did we reached the final time? */
        {
          mpi_printf("\nFinal time=%g reached. Simulation ends.\n", All.TimeMax);

          /* make a snapshot at the final time in case none has been produced at this time yet */
          if(All.Ti_lastoutput != All.Ti_Current)
            {
              All.Ti_nextoutput = All.Ti_Current;
              create_snapshot_if_desired();
            }

          break;
        }

      /* kicks particles by half a gravity step */
      find_timesteps_and_do_gravity_step_first_half();

#ifdef DEBUG_MD5
      Logs.log_debug_md5("B");
#endif

      /* Find new hydro timesteps. This will not change the set of active hydro particles at this synchronization point,
       * but it can change how they are distributed over timebins. */
      find_hydro_timesteps();

      /* compute hydro-forces and apply momentum changes to interacting particle pairs for first half-steps */
      do_hydro_step_first_half();

      /* update the neighbor tree with the new velocities */
      NgbTree.update_velocities();

      /* output some CPU usage log-info (accounts for everything needed up to complete the previous timestep) */
      Logs.write_cpu_log();

#ifdef STOP_AFTER_STEP
      if(All.NumCurrentTiStep == STOP_AFTER_STEP)
        {
          mpi_printf("RUN: We have reached the timestep specified with STOP_AFTER_STEP and therefore stop.");
          endrun();
        }
#endif

#ifdef ZERO_MASS_POT_TRACER  // the potential tracer part
      // array of the particles' data in local process
      double positions[Sp.NumPart][3];     // positions of tracers
      double potentials[Sp.NumPart];       // potentials of the tracers
      int partIDs[Sp.NumPart];             // particle IDs of the tracers
      int idPotTracer[Sp.NumPart];         // id of tracers in the local array
      int numRecenter     = 0;             // number of recentering anchor particles in local process
      int numPotTracer    = 0;             // number of tracers in local process
      int numPotTracerTot = 0;             // number of tracers in global process
      int idRecenter[Sp.NumPart];          // id of recentering anchors in the local array
      for(int i = 0; i < Sp.NumPart; ++i)  // collect the data of the potential tracers
        {
          if(Sp.P[i].getType() == All.PotTracerType)
            {
              if(All.NumCurrentTiStep % All.PotOutStep == 0)  // collect potentials only at the specified output steps
                {
                  Sp.intpos_to_pos(Sp.P[i].IntPos, pos);  // collect positions
                  positions[numPotTracer][0] = pos[0];
                  positions[numPotTracer][1] = pos[1];
                  positions[numPotTracer][2] = pos[2];
                  potentials[numPotTracer]   = Sp.P[i].Potential;
                }
              partIDs[numPotTracer]       = (int)Sp.P[i].ID.get();  // collect particle IDs in local process
              idPotTracer[numPotTracer++] = i;                      // get the number and id of the potential tracers
            }
          else if(Sp.P[i].getType() == All.RecenterPartType)
            idRecenter[numRecenter++] = i;  // get the number and id of the recentering anchors
        }
      if(All.NumCurrentTiStep % All.PotOutStep == 0)
        // output the positions and potentials of the potential tracers at specified output steps
        {
          collect_potential_tracers(potentials, positions, partIDs, potGlobal, posGlobal, pIDsGlobal, numPotTracer, numPotTracerTot,
                                    ThisTask, NTask);  // collect the data of the potential tracers in all processes
          if(ThisTask == 0)                            // only the root rank writes the data to the file
            write_potential_tracers(All.PotOutFile, potGlobal, posGlobal, pIDsGlobal, All.Time, numPotTracerTot);

          if(All.NumCurrentTiStep == 0)  // backup the position of the potential tracers at the beginning of the simulation
            {
              memset(initPos, 0, Sp.TotNumPart * 3 * sizeof(MyReal));  // initialize the array to 0
              if(numPotTracer > 0)
                // only calculate the minimum if there are potential tracers, to avoid garbage value
                firstIDofPotTracer = *(std::min_element(partIDs, partIDs + numPotTracer));
              else
                firstIDofPotTracer = INT_MAX;  // set the first ID to a large value if there is no potential tracer, so that the
                                               // following MPI_Allreduce will not change the value
              MPI_Allreduce(MPI_IN_PLACE, &firstIDofPotTracer, 1, MPI_INT, MPI_MIN,
                            Communicator);  // get the first ID of the potential tracer particles in global process
              for(int i = 0; i < numPotTracer; ++i)
                {
                  initPos[partIDs[i] - firstIDofPotTracer][0] = positions[i][0];  // use the related id as the index of the array
                  initPos[partIDs[i] - firstIDofPotTracer][1] = positions[i][1];
                  initPos[partIDs[i] - firstIDofPotTracer][2] = positions[i][2];
                }
              MPI_Allreduce(MPI_IN_PLACE, initPos, 3 * Sp.TotNumPart, MPI_DOUBLE, MPI_SUM, Communicator);
            }
        }

      // the main loop of the recentering
      for(int loop = 0; loop < 25; ++loop)  // MAX number of iterations = 25
        {
          double factor =
              All.NumCurrentTiStep == 0 ? 100.0 : 1;  // the scale factor for region size: set a large region for the 1st iteration
          memset(comNumerator, 0, 3 * sizeof(double));
          comDenominator = 0.0;
          // backup the old value
          oldValue[0] = centerOfMass[0];
          oldValue[1] = centerOfMass[1];
          oldValue[2] = centerOfMass[2];
          for(int i = 0; i < numRecenter; ++i)  // calculate the center of mass: denominator and numerator in local process
            {
              Sp.intpos_to_pos(Sp.P[idRecenter[i]].IntPos, pos);
              // only consider the particles within the specified radius
              offset = sqrt((pos[0] - centerOfMass[0]) * (pos[0] - centerOfMass[0]) +
                            (pos[1] - centerOfMass[1]) * (pos[1] - centerOfMass[1]) +
                            (pos[2] - centerOfMass[2]) * (pos[2] - centerOfMass[2]));
              if(offset < All.RecenterSize * factor)  // only consider the particles within the specified radius
                {
                  comNumerator[0] += pos[0] * Sp.P[idRecenter[i]].getMass();
                  comNumerator[1] += pos[1] * Sp.P[idRecenter[i]].getMass();
                  comNumerator[2] += pos[2] * Sp.P[idRecenter[i]].getMass();
                  comDenominator += Sp.P[idRecenter[i]].getMass();
                }
            }
          // MPI reduction to get the demoninator and numerator of the center of mass
          MPI_Allreduce(MPI_IN_PLACE, comNumerator, 3, MPI_DOUBLE, MPI_SUM, Communicator);
          MPI_Allreduce(MPI_IN_PLACE, &comDenominator, 1, MPI_DOUBLE, MPI_SUM, Communicator);
          // update the center of mass
          centerOfMass[0] = comNumerator[0] / comDenominator;
          centerOfMass[1] = comNumerator[1] / comDenominator;
          centerOfMass[2] = comNumerator[2] / comDenominator;
          // check whether the center of mass has converged
          if((centerOfMass[0] - oldValue[0]) * (centerOfMass[0] - oldValue[0]) +
                 (centerOfMass[1] - oldValue[1]) * (centerOfMass[1] - oldValue[1]) +
                 (centerOfMass[2] - oldValue[2]) * (centerOfMass[2] - oldValue[2]) <
             All.RecenterThreshold)  // if the center of mass has converged, break the loop
            break;
        }
      // shift the potential tracer particles w.r.t the center of mass
      for(int i = 0; i < numPotTracer; ++i)
        {
          MyReal pos[3] = {initPos[partIDs[i] - firstIDofPotTracer][0] + centerOfMass[0],
                           initPos[partIDs[i] - firstIDofPotTracer][1] + centerOfMass[1],
                           initPos[partIDs[i] - firstIDofPotTracer][2] + centerOfMass[2]};
          Sp.pos_to_intpos(pos, intpos);
          Sp.P[idPotTracer[i]].IntPos[0] = intpos[0];
          Sp.P[idPotTracer[i]].IntPos[1] = intpos[1];
          Sp.P[idPotTracer[i]].IntPos[2] = intpos[2];
        }  // update the center of mass for potential tracer particles
#endif

#ifdef GALOTFA_ON
        // Data collection part, which will be used in galotfa
        // array of the particles' data
#ifndef ZERO_MASS_POT_TRACER
      double positions[Sp.NumPart][3];  // positions array
      static double pos[3];
#endif  // use the positions and potentials of the potential tracer particles to save memory if ZERO_MASS_POT_TRACER is defined
      int ids[Sp.NumPart];
      double masses[Sp.NumPart];
      double velocities[Sp.NumPart][3];
      int types[Sp.NumPart];
      for(int i = 0; i < Sp.NumPart; i++)
        {
          ids[i]    = Sp.P[i].ID.get();           // collect particle ids
          masses[i] = Sp.P[i].getMass();          // collect masses
          Sp.intpos_to_pos(Sp.P[i].IntPos, pos);  // collect positions
          positions[i][0]  = pos[0];
          positions[i][1]  = pos[1];
          positions[i][2]  = pos[2];
          velocities[i][0] = Sp.P[i].Vel[0];
          velocities[i][1] = Sp.P[i].Vel[1];
          velocities[i][2] = Sp.P[i].Vel[2];
          types[i]         = (unsigned int)Sp.P[i].getType();
        }

      // API of galotfa
      galotfa_without_pot_tracer(ids, types, masses, positions, velocities,  All.Time, Sp.NumPart);
#endif

      All.NumCurrentTiStep++;

      /* Check whether we should write a restart file */

      if(check_for_interruption_of_run())
        return;
    }

#ifdef ZERO_MASS_POT_TRACER  // the potential tracer part
  // array of the particles' data in local process
  double positions[Sp.NumPart][3];     // positions of tracers
  double potentials[Sp.NumPart];       // potentials of the tracers
  int partIDs[Sp.NumPart];             // particle IDs of the tracers
  int idPotTracer[Sp.NumPart];         // id of tracers in the local array
  int numRecenter     = 0;             // number of recentering anchor particles in local process
  int numPotTracer    = 0;             // number of tracers in local process
  int numPotTracerTot = 0;             // number of tracers in global process
  int idRecenter[Sp.NumPart];          // id of recentering anchors in the local array
  for(int i = 0; i < Sp.NumPart; ++i)  // collect the data of the potential tracers
    {
      if(Sp.P[i].getType() == All.PotTracerType)
        {
          if(All.NumCurrentTiStep % All.PotOutStep == 0)  // collect potentials only at the specified output steps
            {
              Sp.intpos_to_pos(Sp.P[i].IntPos, pos);  // collect positions
              positions[numPotTracer][0] = pos[0];
              positions[numPotTracer][1] = pos[1];
              positions[numPotTracer][2] = pos[2];
              potentials[numPotTracer]   = Sp.P[i].Potential;
            }
          partIDs[numPotTracer]       = (int)Sp.P[i].ID.get();  // collect particle IDs in local process
          idPotTracer[numPotTracer++] = i;                      // get the number and id of the potential tracers
        }
      else if(Sp.P[i].getType() == All.RecenterPartType)
        idRecenter[numRecenter++] = i;  // get the number and id of the recentering anchors
    }
  if(All.NumCurrentTiStep % All.PotOutStep == 0)
    // output the positions and potentials of the potential tracers at specified output steps
    {
      collect_potential_tracers(potentials, positions, partIDs, potGlobal, posGlobal, pIDsGlobal, numPotTracer, numPotTracerTot,
                                ThisTask, NTask);  // collect the data of the potential tracers in all processes
      if(ThisTask == 0)                            // only the root rank writes the data to the file
        write_potential_tracers(All.PotOutFile, potGlobal, posGlobal, pIDsGlobal, All.Time, numPotTracerTot);

      if(All.NumCurrentTiStep == 0)  // backup the position of the potential tracers at the beginning of the simulation
        {
          memset(initPos, 0, Sp.TotNumPart * 3 * sizeof(MyReal));  // initialize the array to 0
          if(numPotTracer > 0)
            // only calculate the minimum if there are potential tracers, to avoid garbage value
            firstIDofPotTracer = *(std::min_element(partIDs, partIDs + numPotTracer));
          else
            firstIDofPotTracer = INT_MAX;  // set the first ID to a large value if there is no potential tracer, so that the
                                           // following MPI_Allreduce will not change the value
          MPI_Allreduce(MPI_IN_PLACE, &firstIDofPotTracer, 1, MPI_INT, MPI_MIN,
                        Communicator);  // get the first ID of the potential tracer particles in global process
          for(int i = 0; i < numPotTracer; ++i)
            {
              initPos[partIDs[i] - firstIDofPotTracer][0] = positions[i][0];  // use the related id as the index of the array
              initPos[partIDs[i] - firstIDofPotTracer][1] = positions[i][1];
              initPos[partIDs[i] - firstIDofPotTracer][2] = positions[i][2];
            }
          MPI_Allreduce(MPI_IN_PLACE, initPos, 3 * Sp.TotNumPart, MPI_DOUBLE, MPI_SUM, Communicator);
        }
    }

  // the main loop of the recentering
  for(int loop = 0; loop < 25; ++loop)  // MAX number of iterations = 25
    {
      double factor =
          All.NumCurrentTiStep == 0 ? 100.0 : 1;  // the scale factor for region size: set a large region for the 1st iteration
      memset(comNumerator, 0, 3 * sizeof(double));
      comDenominator = 0.0;
      // backup the old value
      oldValue[0] = centerOfMass[0];
      oldValue[1] = centerOfMass[1];
      oldValue[2] = centerOfMass[2];
      for(int i = 0; i < numRecenter; ++i)  // calculate the center of mass: denominator and numerator in local process
        {
          Sp.intpos_to_pos(Sp.P[idRecenter[i]].IntPos, pos);
          // only consider the particles within the specified radius
          offset =
              sqrt((pos[0] - centerOfMass[0]) * (pos[0] - centerOfMass[0]) + (pos[1] - centerOfMass[1]) * (pos[1] - centerOfMass[1]) +
                   (pos[2] - centerOfMass[2]) * (pos[2] - centerOfMass[2]));
          if(offset < All.RecenterSize * factor)  // only consider the particles within the specified radius
            {
              comNumerator[0] += pos[0] * Sp.P[idRecenter[i]].getMass();
              comNumerator[1] += pos[1] * Sp.P[idRecenter[i]].getMass();
              comNumerator[2] += pos[2] * Sp.P[idRecenter[i]].getMass();
              comDenominator += Sp.P[idRecenter[i]].getMass();
            }
        }
      // MPI reduction to get the demoninator and numerator of the center of mass
      MPI_Allreduce(MPI_IN_PLACE, comNumerator, 3, MPI_DOUBLE, MPI_SUM, Communicator);
      MPI_Allreduce(MPI_IN_PLACE, &comDenominator, 1, MPI_DOUBLE, MPI_SUM, Communicator);
      // update the center of mass
      centerOfMass[0] = comNumerator[0] / comDenominator;
      centerOfMass[1] = comNumerator[1] / comDenominator;
      centerOfMass[2] = comNumerator[2] / comDenominator;
      // check whether the center of mass has converged
      if((centerOfMass[0] - oldValue[0]) * (centerOfMass[0] - oldValue[0]) +
             (centerOfMass[1] - oldValue[1]) * (centerOfMass[1] - oldValue[1]) +
             (centerOfMass[2] - oldValue[2]) * (centerOfMass[2] - oldValue[2]) <
         All.RecenterThreshold)  // if the center of mass has converged, break the loop
        break;
    }
  // shift the potential tracer particles w.r.t the center of mass
  for(int i = 0; i < numPotTracer; ++i)
    {
      MyReal pos[3] = {initPos[partIDs[i] - firstIDofPotTracer][0] + centerOfMass[0],
                       initPos[partIDs[i] - firstIDofPotTracer][1] + centerOfMass[1],
                       initPos[partIDs[i] - firstIDofPotTracer][2] + centerOfMass[2]};
      Sp.pos_to_intpos(pos, intpos);
      Sp.P[idPotTracer[i]].IntPos[0] = intpos[0];
      Sp.P[idPotTracer[i]].IntPos[1] = intpos[1];
      Sp.P[idPotTracer[i]].IntPos[2] = intpos[2];
    }  // update the center of mass for potential tracer particles
#endif

#ifdef GALOTFA_ON
    // Data collection part, which will be used in galotfa
    // array of the particles' data
#ifndef ZERO_MASS_POT_TRACER
  double positions[Sp.NumPart][3];  // positions array
  double pos[3];
#endif  // use the positions and potentials of the potential tracer particles to save memory if ZERO_MASS_POT_TRACER is defined
  int ids[Sp.NumPart];
  double masses[Sp.NumPart];
  double velocities[Sp.NumPart][3];
  int types[Sp.NumPart];
  for(int i = 0; i < Sp.NumPart; i++)
    {
      ids[i]    = Sp.P[i].ID.get();           // collect particle ids
      masses[i] = Sp.P[i].getMass();          // collect masses
      Sp.intpos_to_pos(Sp.P[i].IntPos, pos);  // collect positions
      positions[i][0]  = pos[0];
      positions[i][1]  = pos[1];
      positions[i][2]  = pos[2];
      velocities[i][0] = Sp.P[i].Vel[0];
      velocities[i][1] = Sp.P[i].Vel[1];
      velocities[i][2] = Sp.P[i].Vel[2];
      types[i]         = (unsigned int)Sp.P[i].getType();
    }

      // API of galotfa
      galotfa_without_pot_tracer(ids, types, masses, positions, velocities,  All.Time, Sp.NumPart);
#endif
  restart Restart{Communicator};
  Restart.write(this); /* write a restart file at final time - can be used to continue simulation beyond final time */

  Logs.write_cpu_log(); /* output final cpu measurements */
}

/*! \brief calls extra modules after drift operator
 *
 * This routine is called after a new synchronization time has been determined.
 */
void sim::set_non_standard_physics_for_current_time(void)
{
#ifdef COOLING
  CoolSfr.IonizeParams(); /* set UV background for the current time */
#endif
}

/*! \brief calls extra modules at the end of the run loop
 *
 * The second gravitational half kick has already been applied to the
 * particles at this time, i.e. the particles at the sync-point have finished their regular timestep.
 *
 */
void sim::calculate_non_standard_physics_end_of_step(void)
{
#ifdef COOLING
#ifdef STARFORMATION
  CoolSfr.sfr_create_star_particles(&Sp);
  CoolSfr.cooling_and_starformation(&Sp);
#else
  CoolSfr.cooling_only(&Sp);
#endif
#endif

#ifdef MEASURE_TOTAL_MOMENTUM
  Logs.compute_total_momentum();
#endif
}

/*! \brief checks whether the run must interrupted
 *
 * The run is interrupted either if the stop file is present or,
 * if 85% of the CPU time are up. This routine also handles the
 * regular writing of restart files. The restart file is also
 * written if the restart file is present
 *
 * \return 1 if the run has to be interrupted, 0 otherwise
 */
int sim::check_for_interruption_of_run(void)
{
  /* Check whether we need to interrupt the run */
  int stopflag = 0;
  if(ThisTask == 0)
    {
      FILE *fd;
      char stopfname[MAXLEN_PATH_EXTRA];

      snprintf(stopfname, MAXLEN_PATH_EXTRA, "%sstop", All.OutputDir);
      if((fd = fopen(stopfname, "r"))) /* Is the stop-file present? If yes, interrupt the run. */
        {
          fclose(fd);
          printf("stop-file detected. stopping.\n");
          stopflag = 1;
          unlink(stopfname);
        }

      snprintf(stopfname, MAXLEN_PATH_EXTRA, "%srestart", All.OutputDir);
      if((fd = fopen(stopfname, "r"))) /* Is the restart-file present? If yes, write a user-requested restart file. */
        {
          fclose(fd);
          printf("restart-file detected. writing restart files.\n");
          stopflag = 3;
          unlink(stopfname);
        }

      if(Logs.CPUThisRun > 0.85 * All.TimeLimitCPU) /* are we running out of CPU-time ? If yes, interrupt run. */
        {
          printf("reaching time-limit. stopping.\n");
          stopflag = 2;
        }
    }

  MPI_Bcast(&stopflag, 1, MPI_INT, 0, Communicator);

  if(stopflag)
    {
      restart Restart{Communicator};
      Restart.write(this); /* write restart file */
      MPI_Barrier(Communicator);

      if(stopflag == 3)
        return 0;

      if(stopflag == 2 && ThisTask == 0)
        {
          FILE *fd;
          char contfname[MAXLEN_PATH_EXTRA];
          snprintf(contfname, MAXLEN_PATH_EXTRA, "%scont", All.OutputDir);
          if((fd = fopen(contfname, "w")))
            fclose(fd);
        }
      return 1;
    }

  /* is it time to write a regular restart-file? (for security) */
  if(ThisTask == 0)
    {
      if((Logs.CPUThisRun - All.TimeLastRestartFile) >= All.CpuTimeBetRestartFile)
        {
          All.TimeLastRestartFile = Logs.CPUThisRun;
          stopflag                = 3;
        }
      else
        stopflag = 0;
    }

  MPI_Bcast(&stopflag, 1, MPI_INT, 0, Communicator);

  if(stopflag == 3)
    {
      restart Restart{Communicator};
      Restart.write(this); /* write an occasional restart file */
      stopflag = 0;
    }
  return 0;
}

/*! \brief Returns the next output time that is equal or larger than
 *  ti_curr
 *
 *  \param ti_curr current simulation time
 */
integertime sim::find_next_outputtime(integertime ti_curr)
{
  integertime ti;
  integertime ti_next = -1;
  double time;

  All.DumpFlag_nextoutput = 1;

  if(All.OutputListOn)
    {
      for(int i = 0; i < All.OutputListLength; i++)
        {
          time = All.OutputListTimes[i];

          if(time >= All.TimeBegin && time <= All.TimeMax)
            {
              if(All.ComovingIntegrationOn)
                ti = (integertime)(log(time / All.TimeBegin) / All.Timebase_interval);
              else
                ti = (integertime)((time - All.TimeBegin) / All.Timebase_interval);

#ifndef OUTPUT_NON_SYNCHRONIZED_ALLOWED
              /* We will now modify 'ti' to map it to the closest available output time according to the specified MaxSizeTimestep.
               * The real output time may hence deviate by  +/- 0.5*MaxSizeTimestep from the desired output time.
               */

              /* first, determine maximum output interval based on All.MaxSizeTimestep */
              integertime timax = (integertime)(All.MaxSizeTimestep / All.Timebase_interval);

              /* make it a power 2 subdivision */
              integertime ti_min = TIMEBASE;
              while(ti_min > timax)
                ti_min >>= 1;
              timax = ti_min;

              double multiplier = ti / ((double)timax);

              /* now round this to the nearest multiple of timax */
              ti = ((integertime)(multiplier + 0.5)) * timax;
#endif

              if(ti >= ti_curr)
                {
                  if(ti_next == -1)
                    {
                      ti_next                 = ti;
                      All.DumpFlag_nextoutput = All.OutputListFlag[i];
                    }

                  if(ti_next > ti)
                    {
                      ti_next                 = ti;
                      All.DumpFlag_nextoutput = All.OutputListFlag[i];
                    }
                }
            }
        }
    }
  else
    {
      if(All.ComovingIntegrationOn)
        {
          if(All.TimeBetSnapshot <= 1.0)
            Terminate("TimeBetSnapshot > 1.0 required for your simulation.\n");
        }
      else
        {
          if(All.TimeBetSnapshot <= 0.0)
            Terminate("TimeBetSnapshot > 0.0 required for your simulation.\n");
        }
      time     = All.TimeOfFirstSnapshot;
      int iter = 0;

      while(time < All.TimeBegin)
        {
          if(All.ComovingIntegrationOn)
            time *= All.TimeBetSnapshot;
          else
            time += All.TimeBetSnapshot;

          iter++;

          if(iter > 1000000)
            Terminate("Can't determine next output time.\n");
        }

      while(time <= All.TimeMax)
        {
          if(All.ComovingIntegrationOn)
            ti = (integertime)(log(time / All.TimeBegin) / All.Timebase_interval);
          else
            ti = (integertime)((time - All.TimeBegin) / All.Timebase_interval);

#ifndef OUTPUT_NON_SYNCHRONIZED_ALLOWED
          /* We will now modify 'ti' to map it to the closest available output time according to the specified MaxSizeTimestep.
           * The real output time may hence deviate by  +/- 0.5*MaxSizeTimestep from the desired output time.
           */

          /* first, determine maximum output interval based on All.MaxSizeTimestep */
          integertime timax = (integertime)(All.MaxSizeTimestep / All.Timebase_interval);

          /* make it a power 2 subdivision */
          integertime ti_min = TIMEBASE;
          while(ti_min > timax)
            ti_min >>= 1;
          timax = ti_min;

          double multiplier = ti / ((double)timax);

          /* now round this to the nearest multiple of timax */
          ti = ((integertime)(multiplier + 0.5)) * timax;
#endif

          if(ti >= ti_curr)
            {
              ti_next = ti;
              break;
            }

          if(All.ComovingIntegrationOn)
            time *= All.TimeBetSnapshot;
          else
            time += All.TimeBetSnapshot;

          iter++;

          if(iter > MAXITER)
            Terminate("Can't determine next output time.\n");
        }
    }

  if(ti_next == -1)
    {
      ti_next = 2 * TIMEBASE; /* this will prevent any further output */

      mpi_printf("\nSNAPSHOT: There is no valid time for a further snapshot file.\n");
    }
  else
    {
      double next = All.get_absolutetime_from_integertime(ti_next);

      mpi_printf("\nSNAPSHOT: Setting next time for snapshot file to Time_next= %g  (DumpFlag=%d)\n\n", next, All.DumpFlag_nextoutput);
    }

  return ti_next;
}

/*! \brief Check if a snapshot should be saved
 *
 * This function checks whether a snapshot file or other kinds of output files,
 * such as a projection, should be saved at the current time-step.
 * If that is the case, the appropriate functions to produce the
 * desired file are called and the parameter controlling the output are updated
 * accordingly.
 */
void sim::create_snapshot_if_desired(void)
{
#if defined(LIGHTCONE_MASSMAPS)
  /* we may do this on partial timesteps since for massmaps we always drift all particles, i.e. the lightcone is complete up to
   * All.Time */
  LightCone.lightcone_massmap_flush(1);
#endif

#ifndef OUTPUT_NON_SYNCHRONIZED_ALLOWED
  if(All.HighestActiveTimeBin == All.HighestOccupiedTimeBin) /* allow only top-level synchronization points */
#endif
    if(All.Ti_Current >= All.Ti_nextoutput && All.Ti_nextoutput >= 0)
      {
        for(int i = 0; i < Sp.NumPart; i++)
          if(Sp.P[i].Ti_Current != All.Ti_Current)
            Terminate("P[i].Ti_Current != All.Ti_Current");

#if defined(STARFORMATION) && defined(FOF)
        // do an extra domain decomposition here to make sure that there are no new stars among the block of gas particles
        NgbTree.treefree();
        Domain.domain_free();
        Domain.domain_decomposition(STANDARD);
        NgbTree.treeallocate(Sp.NumGas, &Sp, &Domain);
        NgbTree.treebuild(Sp.NumGas, NULL);
#endif

#ifndef OUTPUT_NON_SYNCHRONIZED_ALLOWED
        NgbTree.treefree();
        Sp.TimeBinsGravity.timebins_free();
        Sp.TimeBinsHydro.timebins_free();
#endif

#ifdef FOF
        mpi_printf("\nFOF: We shall first compute a group catalog for this snapshot file\n");

        /* this structure will hold auxiliary information for each particle, needed only during group finding */
        Sp.PS = (subfind_data *)Mem.mymalloc_movable(&Sp.PS, "PS", Sp.MaxPart * sizeof(subfind_data));
        memset(Sp.PS, 0, Sp.MaxPart * sizeof(subfind_data));

        /* First, we save the original location of the particles, in order to be able to revert to this layout later on */
        for(int i = 0; i < Sp.NumPart; i++)
          {
            Sp.PS[i].OriginTask  = ThisTask;
            Sp.PS[i].OriginIndex = i;
          }

        fof<simparticles> FoF{Communicator, &Sp, &Domain};

        FoF.fof_fof(All.SnapshotFileCount, "fof", "groups", 0);

#if defined(MERGERTREE) && defined(SUBFIND)
        MergerTree.CurrTotNsubhalos = FoF.TotNsubhalos;
        MergerTree.CurrNsubhalos    = FoF.Nsubhalos;

        MergerTree.mergertree_determine_descendants_on_the_fly(All.SnapshotFileCount);

        MergerTree.PrevTotNsubhalos = FoF.TotNsubhalos;
        MergerTree.PrevNsubhalos    = FoF.Nsubhalos;

        for(int n = 0; n < Sp.NumPart; n++)
          {
            Sp.P[n].PrevSubhaloNr     = Sp.PS[n].SubhaloNr;
            Sp.P[n].PrevSizeOfSubhalo = Sp.PS[n].SizeOfSubhalo;
            Sp.P[n].PrevRankInSubhalo = Sp.PS[n].RankInSubhalo;

            if(Sp.P[n].PrevSubhaloNr.get() >= MergerTree.PrevTotNsubhalos && Sp.P[n].PrevSubhaloNr.get() != HALONR_MAX)
              Terminate("Sp.P[n].PrevSubhaloNr=%lld  MergerTree.PrevTotNsubhalos=%lld\n", (long long)Sp.P[n].PrevSubhaloNr.get(),
                        (long long)MergerTree.PrevTotNsubhalos);

            if(Sp.P[n].PrevSizeOfSubhalo.get() > 0 && Sp.P[n].PrevSubhaloNr.get() == HALONR_MAX)
              Terminate("Sp.P[n].PrevSizeOfSubhalo=%d  Sp.P[n].PrevSubhaloNr=%lld\n", (int)Sp.P[n].PrevSizeOfSubhalo.get(),
                        (long long)Sp.P[n].PrevSubhaloNr.get());
          }
#endif
#endif

        if(All.DumpFlag_nextoutput)
          {
            snap_io Snap(&Sp, Communicator, All.SnapFormat);             /* get an I/O object */
            Snap.write_snapshot(All.SnapshotFileCount, NORMAL_SNAPSHOT); /* write snapshot file */
          }

#ifdef SUBFIND_ORPHAN_TREATMENT
        {
          snap_io Snap(&Sp, Communicator, All.SnapFormat);
          Snap.write_snapshot(All.SnapshotFileCount, MOST_BOUND_PARTICLE_SNAPHOT); /* write special snapshot file */
        }
#endif

#ifdef FOF
        /* now revert from output order to the original order */
        for(int n = 0; n < Sp.NumPart; n++)
          {
            Sp.PS[n].TargetTask  = Sp.PS[n].OriginTask;
            Sp.PS[n].TargetIndex = Sp.PS[n].OriginIndex;
          }

        TIMER_START(CPU_FOF);

        Domain.particle_exchange_based_on_PS(Communicator);

        TIMER_STOP(CPU_FOF);

        Mem.myfree(Sp.PS);
#endif

#if defined(POWERSPEC_ON_OUTPUT) && defined(PERIODIC) && defined(PMGRID)
        PM.calculate_power_spectra(All.SnapshotFileCount);
#endif

        All.SnapshotFileCount++;
        All.Ti_nextoutput = find_next_outputtime(All.Ti_Current + 1);

#ifndef OUTPUT_NON_SYNCHRONIZED_ALLOWED
        Sp.TimeBinsHydro.timebins_allocate();
        Sp.TimeBinsGravity.timebins_allocate();

        /* we need to reconstruct the timebins here. Even though the particles are in the same place again,
         * it could have happened that Sp.P was reduced in size temporarily below NumPart on a certain task,
         * in which case the timebin data may have become invalid.
         */
        Sp.reconstruct_timebins();

        NgbTree.treeallocate(Sp.NumGas, &Sp, &Domain);
        NgbTree.treebuild(Sp.NumGas, NULL);
#endif
      }

#if defined(LIGHTCONE_PARTICLES)
  if(Lp.TestIfAboveFillFactor(std::min<int>(Lp.MaxPart, Sp.MaxPart)))
    {
#if defined(LIGHTCONE_PARTICLES_GROUPS) && defined(FOF)
      /* do this only on full timesteps if groups are calculated on lightcone */
      if(All.Ti_Current >= TIMEBASE || All.HighestActiveTimeBin == All.HighestOccupiedTimeBin)
        {
          mpi_printf("\nLIGHTCONE_PARTICLES_GROUPS: We shall first compute a group catalogue for the lightcone particles\n");

          /* assign unique IDs to Lp particles */

          int *numlist = (int *)Mem.mymalloc("numlist", Lp.NumPart * sizeof(int));

          MPI_Allgather(&Lp.NumPart, 1, MPI_INT, numlist, 1, MPI_INT, Communicator);

          long long newID = 1;
          for(int i = 0; i < ThisTask; i++)
            newID += numlist[i];

          for(int i = 0; i < Lp.NumPart; i++)
            Lp.P[i].ID.set(newID++);

          Mem.myfree(numlist);

          domain<lcparticles> LcDomain(Communicator, &Lp);

          LcDomain.domain_decomposition(STANDARD);

          /* this structure will hold auxiliary information for each particle, needed only during group finding */
          Lp.PS = (subfind_data *)Mem.mymalloc_movable(&Lp.PS, "PS", Lp.MaxPart * sizeof(subfind_data));
          memset(Lp.PS, 0, Lp.MaxPart * sizeof(subfind_data));

          /* First, we save the original location of the particles, in order to be able to revert to this layout later on */
          for(int i = 0; i < Lp.NumPart; i++)
            {
              Lp.PS[i].OriginTask  = ThisTask;
              Lp.PS[i].OriginIndex = i;
            }

          fof<lcparticles> FoF{Communicator, &Lp, &LcDomain};

          double inner_distance = Driftfac.get_comoving_distance_for_scalefactor(All.Time);

          FoF.fof_fof(All.LightconeFileCount, "lc_fof", "lc_groups", inner_distance);

#endif

          {
#ifdef MERGERTREE
            MergerTree.Ntrees = 0;
            lightcone_particle_io Lcone(&Lp, &LightCone, &MergerTree, Communicator, All.SnapFormat); /* get an I/O object */
#else
        lightcone_particle_io Lcone(&Lp, &LightCone, Communicator, All.SnapFormat); /* get an I/O object */
#endif
            long long NumLP_tot = Lp.NumPart;
            MPI_Allreduce(MPI_IN_PLACE, &NumLP_tot, 1, MPI_LONG_LONG, MPI_SUM, Communicator);
            mpi_printf("\nLIGHTCONE: writing particle lightcone conesnap files #%d ... (NumLP_tot = %lld)\n", All.LightconeFileCount,
                       NumLP_tot);

            for(int i = 0; i < Lp.NumPart; i++)
              {
                double pos[3];
                Lp.signedintpos_to_pos((MySignedIntPosType *)Lp.P[i].IntPos, pos);
                vec2pix_ring(LIGHTCONE_ORDER_NSIDE, pos, &Lp.P[i].ipnest);
              }

#if !defined(LIGHTCONE_PARTICLES_GROUPS)
            /* let's now sort the lightcone_particle_data according to healpix pixel number */
            mycxxsort_parallel(Lp.P, Lp.P + Lp.NumPart, Lp.compare_ipnest, Communicator);
#endif

            for(int conenr = 0; conenr < LightCone.Nlightcones; conenr++)
              Lcone.lightcone_save(All.LightconeFileCount, conenr, false);

            mpi_printf("LIGHTCONE: done with writing files.\n");

            All.LightconeFileCount++;
            /* let's put this in brackets so that the object's destructor will be called already here */
          }

#if defined(LIGHTCONE_PARTICLES_GROUPS) && defined(FOF)
          /* now revert from output order to the original order */
          for(int n = 0; n < Lp.NumPart; n++)
            {
              Lp.PS[n].TargetTask  = Lp.PS[n].OriginTask;
              Lp.PS[n].TargetIndex = Lp.PS[n].OriginIndex;
            }

          TIMER_START(CPU_FOF);

          LcDomain.particle_exchange_based_on_PS(Communicator);

          Mem.myfree(Lp.PS);

          LcDomain.domain_free();

          TIMER_STOP(CPU_FOF);

          int ncount[2] = {0, 0};
          long long nsum[2];
          for(int n = 0; n < Lp.NumPart; n++)
            {
              if(Lp.P[n].getFlagSaveDistance())
                {
                  Lp.P[n--] = Lp.P[--Lp.NumPart];
                  ncount[0]++;
                }
              else
                {
                  ncount[1]++;
                }
            }

          sumup_large_ints(2, ncount, nsum, Communicator);
          mpi_printf("LIGHTCONE_PARTICLES_GROUPS: We could store %lld particles from the buffer, but had to keep %lld\n", nsum[0],
                     nsum[1]);
        }
#else
      Lp.NumPart = 0;
#endif

      if(Lp.MaxPart > LIGHTCONE_ALLOC_FAC * Sp.MaxPart + 1 && Lp.NumPart < LIGHTCONE_ALLOC_FAC * Sp.MaxPart)
        Lp.reallocate_memory_maxpart(LIGHTCONE_ALLOC_FAC * Sp.MaxPart);
    }
#endif
}

#ifdef ZERO_MASS_POT_TRACER
// My functions: Bin-Hui Chen
void write_potential_tracers(char filename[], double potentials[], double positions[][3], int ids[], double &time,
                             int &partNum)  // the function to write potential tracers to hdf5 file, only called by the root rank
{
  static int outputCount = 0;
  static char potFile[MAXLEN_PATH_EXTRA];
  snprintf(potFile, MAXLEN_PATH_EXTRA, "%s%s.hdf5", All.OutputDir, filename);
  static hsize_t tracerNum        = (hsize_t)partNum;
  static hsize_t dims_1d[1]       = {tracerNum};
  static hsize_t maxdims_1d[1]    = {tracerNum};
  static hsize_t chunk_dims_1d[1] = {tracerNum};
  static hid_t dataspace_id_1d    = H5Screate_simple(1, dims_1d, maxdims_1d);
  static hid_t prop_id_1d         = H5Pcreate(H5P_DATASET_CREATE);
  static hsize_t dims_2d[2]       = {tracerNum, 3};
  static hsize_t maxdims_2d[2]    = {tracerNum, 3};
  static hsize_t chunk_dims_2d[2] = {tracerNum, 3};
  static hid_t dataspace_id_2d    = H5Screate_simple(2, dims_2d, maxdims_2d);
  static hid_t prop_id_2d         = H5Pcreate(H5P_DATASET_CREATE);
  // if(firstTime)
  //   {
  H5Pset_chunk(prop_id_1d, 1, chunk_dims_1d);  // set chunk size
  H5Pset_chunk(prop_id_2d, 2, chunk_dims_2d);
  // }
  static hid_t file_id = H5Fcreate(potFile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);  // open the file
  char group_name[20];                                                                 // create dataset name
  snprintf(group_name, 20, "Output_%d", outputCount++);
  hid_t group_id      = H5Gcreate2(file_id, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  hid_t dataset_coord = H5Dcreate2(group_id, "Coordinates", H5T_NATIVE_DOUBLE, dataspace_id_2d, H5P_DEFAULT, prop_id_2d, H5P_DEFAULT);
  hid_t dataset_pot   = H5Dcreate2(group_id, "Potentials", H5T_NATIVE_DOUBLE, dataspace_id_1d, H5P_DEFAULT, prop_id_1d, H5P_DEFAULT);
  hid_t dataset_id    = H5Dcreate2(group_id, "TracerIDs", H5T_NATIVE_INT, dataspace_id_1d, H5P_DEFAULT, prop_id_1d, H5P_DEFAULT);
  hid_t dataSpace_coord = H5Dget_space(dataset_coord);
  hid_t dataSpace_pot   = H5Dget_space(dataset_pot);
  hid_t dataSpace_id    = H5Dget_space(dataset_id);
  H5Dwrite(dataset_coord, H5T_NATIVE_DOUBLE, dataspace_id_2d, dataSpace_coord, H5P_DEFAULT, positions);  // write data
  H5Dwrite(dataset_pot, H5T_NATIVE_DOUBLE, dataspace_id_1d, dataSpace_pot, H5P_DEFAULT, potentials);
  H5Dwrite(dataset_id, H5T_NATIVE_INT, dataspace_id_1d, dataSpace_id, H5P_DEFAULT, ids);
  // write attributes
  hid_t attr_id    = H5Screate(H5S_SCALAR);
  hid_t attr_space = H5Acreate2(group_id, "Time", H5T_NATIVE_DOUBLE, attr_id, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attr_space, H5T_NATIVE_DOUBLE, &time);
  // close attributes
  H5Aclose(attr_space);
  // close datasets
  H5Dclose(dataset_coord);
  H5Dclose(dataset_pot);
  H5Dclose(dataset_id);
  // close groups
  H5Gclose(group_id);
}

void collect_potential_tracers(double localPot[], double localPos[][3], int localIDs[], double globalPot[], double globalPos[][3],
                               int globalIDs[], int &localNum, int &globalNum, int &rank, int &size)
{
  if(rank == 0)  // collect data from the root rank
    {
      globalNum = localNum;
      for(int i = 0; i < localNum; ++i)
        {
          globalPot[i] = localPot[i];
          globalIDs[i] = localIDs[i];
          for(int j = 0; j < 3; ++j)
            globalPos[i][j] = localPos[i][j];
        }
    }
  if(size > 1)  // if there are more than one MPI tasks
    {
      if(rank == 0)
        {
          static int offset;  // the integer used to record the offset of the data to be sent
          offset                      = localNum;
          static int dataTransferSize = 0;
          for(int i = 1; i < size; ++i)
            {
              MPI_Recv(&dataTransferSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD,
                       MPI_STATUS_IGNORE);  // receive the size of data to be sent
              MPI_Recv(globalPot + offset, dataTransferSize, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Recv(globalPos + offset, 3 * dataTransferSize, MPI_DOUBLE, i, 1e5 + i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Recv(globalIDs + offset, dataTransferSize, MPI_INT, i, 2e5 + i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              offset += dataTransferSize;
            }
          globalNum = offset;
        }
      else
        {
          for(int i = 1; i < size; ++i)
            if(rank == i)
              {
                MPI_Send(&localNum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  // send the size of data to be sent
                MPI_Send(localPot, localNum, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
                MPI_Send(localPos, 3 * localNum, MPI_DOUBLE, 0, 1e5 + i, MPI_COMM_WORLD);
                MPI_Send(localIDs, localNum, MPI_INT, 0, 2e5 + i, MPI_COMM_WORLD);
              }
        }
    }
}
#endif
