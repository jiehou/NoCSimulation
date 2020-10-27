/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file represents the top-level testbench
 */

#ifndef __NOXIMNOC_H__
#define __NOXIMNOC_H__

#include <systemc.h>
#include <vector>
#include "NoximTile.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximGlobalTrafficTable.h"
using namespace std;

extern unsigned int drained_volume;
extern unsigned int process_total;
extern unsigned int total_sim_cycles;

SC_MODULE(NoximNoC)
{
    // I/O Ports
    sc_in_clk clock;		// The input clock for the NoC
    sc_in < bool > reset;	// The reset signal for the NoC
	vector<sc_signal<bool> > test;
    // Signals
    sc_signal <bool> req_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <bool> ack_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <NoximFlit> flit_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <int> free_slots_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    // NoP
    sc_signal <NoximNoP_data> NoP_data_to_east[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_west[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_south[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_north[MAX_STATIC_DIM][MAX_STATIC_DIM];

    // Matrix of tiles
    NoximTile *t[MAX_STATIC_DIM][MAX_STATIC_DIM];

    // Global tables
    NoximGlobalRoutingTable grtable;
    NoximGlobalTrafficTable gttable;

    //---------- Mau experiment <start>
    void flitsMonitor() {
        if (!reset.read()) {
            //      if ((int)sc_simulation_time() % 5)
            //        return;

            unsigned int count = 0;
            for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
            for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
                count += t[i][j]->r->getFlitsCount();
            cout << count << endl;
        }
    }
    //---------- Mau experiment <stop>
    void showHopsInfo() 
    {
        if(!reset.read()) 
        {
            int current_cycle = static_cast<int>(sc_time_stamp().to_double() / 1000 + 0.5);
            if(current_cycle % 100000 == 0)
            {
                cout << "[D] ########## cycle: " << current_cycle << endl;
                outputHopsInfo();
            }
        }
    }

    void checkSaturation()
    {
        if(!reset.read())
        {
            int current_cycle = static_cast<int>(sc_time_stamp().to_double() / 1000 + 0.5);
            if(current_cycle % 5000 == 0)
            {
                if((NoximGlobalParams::mode_target_packets && prev_process_total == process_total) 
                ||(NoximGlobalParams::max_volume_to_be_drained > 0 && prev_drained_volume == drained_volume))
                {
                    cout << "[D] ########## Saturation happens ##" << endl;
                    NoximGlobalParams::network_saturated = true;
                    total_sim_cycles = current_cycle;
                    sc_stop();
                }
                prev_process_total = process_total;
                prev_drained_volume = drained_volume;
            }
        }
    }

    // Constructor
    SC_CTOR(NoximNoC) {
    // Build the Mesh
	buildMesh();
    prev_process_total = process_total;
    prev_drained_volume = drained_volume;
	//---------- Mau experiment <start>
	/*
	   SC_METHOD(flitsMonitor);
	   sensitive(reset);
	   sensitive_pos(clock);
	 */
	//---------- Mau experiment <stop>

    //@DEBUG
    /*
    SC_METHOD(showHopsInfo);
    sensitive << reset;
    sensitive << clock.neg();
    */

    // check saturation
    SC_METHOD(checkSaturation);
    sensitive << reset;
    sensitive << clock.neg();
    }   

    // Support methods
    NoximTile *searchNode(const int id) const;

  private:

    void buildMesh();
    unsigned int prev_process_total;
    unsigned int prev_drained_volume;
};

#endif
