/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the router
 */

#ifndef __NOXIMROUTER_H__
#define __NOXIMROUTER_H__

#include <systemc.h>
#include "NoximMain.h"
#include "NoximBuffer.h"
#include "NoximStats.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximLocalRoutingTable.h"
#include "NoximReservationTable.h"
#include "routings/xyrouting.h"
#include "routings/negativefirstrouting.h"
#include "routings/oddevenrouting.h"
#include "routings/tablebasedrouting.h"
#include "routings/ftnfrouting.h"
#include <memory>
using namespace std;

extern unsigned int drained_volume;
extern unsigned int process_total;
extern unsigned int total_sim_cycles;

SC_MODULE(NoximRouter)
{
  // I/O Ports
  sc_in_clk clock;   // The input clock for the router
  sc_in<bool> reset; // The reset signal for the router

  sc_in<NoximFlit> flit_rx[DIRECTIONS + 1]; // The input channels (including local one)
  sc_in<bool> req_rx[DIRECTIONS + 1];       // The requests associated with the input channels
  sc_out<bool> ack_rx[DIRECTIONS + 1];      // The outgoing ack signals associated with the input channels

  sc_out<NoximFlit> flit_tx[DIRECTIONS + 1]; // The output channels (including local one)
  sc_out<bool> req_tx[DIRECTIONS + 1];       // The requests associated with the output channels
  sc_in<bool> ack_tx[DIRECTIONS + 1];        // The outgoing ack signals associated with the output channels

  sc_out<int> free_slots[DIRECTIONS + 1];
  sc_in<int> free_slots_neighbor[DIRECTIONS];

  // Neighbor-on-Path related I/O
  sc_out<NoximNoP_data> NoP_data_out[DIRECTIONS];
  sc_in<NoximNoP_data> NoP_data_in[DIRECTIONS];

  // Registers
  int local_id;                            // Unique ID
  NoximBuffer buffer[DIRECTIONS + 1];      // Buffer for each input channel
  bool current_level_rx[DIRECTIONS + 1];   // Current level for Alternating Bit Protocol (ABP)
  bool current_level_tx[DIRECTIONS + 1];   // Current level for Alternating Bit Protocol (ABP)
  NoximStats stats;                        // Statistics
  NoximLocalRoutingTable routing_table;    // Routing table
  NoximReservationTable reservation_table; // Switch reservation table
  int start_from_port;                     // Port from which to start the reservation cycle
  unsigned long routed_flits;

  // Functions
  void rxProcess(); // The receiving process
  void txProcess(); // The transmitting process
  void bufferMonitor();
  void showBufferSize();
  void configure(const int _id, const double _warm_up_time,
                 const unsigned int _max_buffer_size,
                 NoximGlobalRoutingTable &grt);

  unsigned long getRoutedFlits(); // Returns the number of routed flits
  unsigned int getFlitsCount();   // Returns the number of flits into the router
  double getPower();              // Returns the total power dissipated by the router

  // Constructor
  SC_CTOR(NoximRouter)
  {
    SC_METHOD(rxProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(txProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(bufferMonitor);
    sensitive << reset;
    sensitive << clock.pos();

    //@DEBUG
    /*
    SC_METHOD(showBufferSize);
    sensitive << reset;
    sensitive << clock.neg();
    */
    //c++14: make_unique<Class>()
    switch (NoximGlobalParams::routing_algorithm)
    {
    case ROUTING_XY:
      routing_algo = unique_ptr<XYRouting>(new XYRouting());
      break;
    case ROUTING_NEGATIVE_FIRST:
      routing_algo = unique_ptr<NegativeFirstRouting>(new NegativeFirstRouting());
      break;
    case ROUTING_ODD_EVEN:
      routing_algo = unique_ptr<OddEvenRouting>(new OddEvenRouting());
      break;
    case ROUTING_TABLE_BASED:
      routing_algo = unique_ptr<TableBasedRouting>(new TableBasedRouting(routing_table));
      break;
    case ROUTING_FTNF:
      routing_algo = unique_ptr<FtnfRouting>(new FtnfRouting());
      break;
    default:
      routing_algo = unique_ptr<XYRouting>(new XYRouting());
    }
  }

private:
  // performs actual routing + selection
  int route(const NoximRouteData &route_data, bool &should_discard_local);

  // wrappers
  int selectionFunction(const vector<int> &directions,
                        const NoximRouteData &route_data);
  // selection strategies
  int selectionRandom(const vector<int> &directions);
  int selectionBufferLevel(const vector<int> &directions);
  int selectionNoP(const vector<int> &directions,
                   const NoximRouteData &route_data);

  // routing
  unique_ptr<BaseRouting> routing_algo;

  NoximNoP_data getCurrentNoPData() const;
  void NoP_report() const;
  int NoPScore(const NoximNoP_data &nop_data, const vector<int> &nop_channels) const;
  int reflexDirection(int direction) const;
  bool inCongestion();

public:
  unsigned int local_drained;
  void ShowBuffersStats(std::ostream & out);
};

#endif