/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the router
 */

#include "NoximRouter.h"

void NoximRouter::rxProcess()
{
	if (reset.read())
	{
		// Clear outputs and indexes of receiving protocol
		for (int i = 0; i < DIRECTIONS + 1; i++)
		{
			ack_rx[i].write(0);
			current_level_rx[i] = 0;
		}
		reservation_table.clear();
		routed_flits = 0;
		local_drained = 0;
	}
	else
	{
		// For each channel decide if a new flit can be accepted
		//
		// This process simply sees a flow of incoming flits. All arbitration
		// and wormhole related issues are addressed in the txProcess()
		for (int i = 0; i < DIRECTIONS + 1; i++)
		{
			// To accept a new flit, the following conditions must match:
			//
			// 1) there is an incoming request
			// 2) there is a free slot in the input buffer of direction i
			if ((req_rx[i].read() == 1 - current_level_rx[i]) && !buffer[i].IsFull())
			{
				NoximFlit received_flit = flit_rx[i].read();
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF)
				{
					cout << sc_time_stamp().to_double() / 1000
						 << ": Router[" << local_id << "], Input[" << i
						 << "], Received flit: " << received_flit << endl;
				}
				// Store the incoming flit in the circular buffer
				buffer[i].Push(received_flit);
				// Negate the old value for Alternating Bit Protocol (ABP)
				current_level_rx[i] = 1 - current_level_rx[i];
				// Incoming flit
				stats.power.Buffering();
				if (received_flit.src_id == local_id)
					stats.power.EndToEnd();
			}
			ack_rx[i].write(current_level_rx[i]);
		}
	}
	stats.power.Leakage();
}

void NoximRouter::txProcess()
{
	if (reset.read())
	{
		// Clear outputs and indexes of transmitting protocol
		for (int i = 0; i < DIRECTIONS + 1; i++)
		{
			req_tx[i].write(0);
			current_level_tx[i] = 0;
		}
	}
	else
	{
		// 1st phase: Reservation
		for (int j = 0; j < DIRECTIONS + 1; j++)
		{
			int i = (start_from_port + j) % (DIRECTIONS + 1);

			if (!buffer[i].IsEmpty())
			{
				NoximFlit flit = buffer[i].Front();
				if (flit.flit_type == FLIT_TYPE_HEAD)
				{
					// prepare data for routing
					NoximRouteData route_data;
					route_data.current_id = local_id;
					route_data.src_id = flit.src_id;
					route_data.dst_id = flit.dst_id;
					route_data.dir_in = i;
					bool should_discard_local = false;
					int o = route(route_data, should_discard_local);

					stats.power.Arbitration();

					if (reservation_table.isAvailable(o))
					{
						stats.power.Crossbar();
						reservation_table.reserve(i, o);
						if(o == DIRECTION_LOCAL && should_discard_local)
							reservation_table.discardLocal();

						if (NoximGlobalParams::verbose_mode > VERBOSE_OFF)
						{
							cout << "#[D] Tx: "<< sc_time_stamp().to_double() / 1000
								 << ": Router[" << local_id
								 << "], Input[" << i << "] (" << buffer[i].Size() << " flits)"
								 << ", reserved Output["
								 << o << "], flit: " << flit << endl;
						}
					}
				}
			}
		}
		start_from_port++;

		// 2nd phase: Forwarding
		for (int i = 0; i < DIRECTIONS + 1; i++)
		{
			if (!buffer[i].IsEmpty())
			{
				NoximFlit flit = buffer[i].Front();
				int o = reservation_table.getOutputPort(i);
				bool should_discard_local = reservation_table.getShouldDiscardLocal();
				if (o != NOT_RESERVED)
				{
					if (current_level_tx[o] == ack_tx[o].read())
					{
						if (NoximGlobalParams::verbose_mode > VERBOSE_OFF)
						{
							cout << sc_time_stamp().to_double() / 1000
								 << ": Router[" << local_id
								 << "], Input[" << i << "] forward to Output[" << o << "], flit: "
								 << flit << endl;
						}
						flit_tx[o].write(flit);
						current_level_tx[o] = 1 - current_level_tx[o];
						req_tx[o].write(current_level_tx[o]);
						buffer[i].Pop();
						//@DEBUG
						if(flit.flit_type == FLIT_TYPE_HEAD && !should_discard_local)
						{
							string str_flit = flitToString(flit);
							NoximGlobalParams::hops_info[str_flit].push_back(local_id);
						}
						if (NoximGlobalParams::low_power_link_strategy)
						{
							if (flit.flit_type == FLIT_TYPE_HEAD ||
								flit.use_low_voltage_path == false)
								stats.power.Link(false);
							else
								stats.power.Link(true);
						}
						else
							stats.power.Link(false);

						if (flit.dst_id == local_id)
							stats.power.EndToEnd();

						if (flit.flit_type == FLIT_TYPE_TAIL)
							reservation_table.release(o);

						// Update stats
						if (o == DIRECTION_LOCAL)
						{
							double current_cycle = sc_time_stamp().to_double() / 1000;
							//bool should_measure = (current_cycle - DEFAULT_RESET_TIME > NoximGlobalParams::stats_warm_up_time);
							if(!should_discard_local)
							{
								stats.receivedFlit(current_cycle, flit);
								if (NoximGlobalParams::max_volume_to_be_drained)
								{
									if (drained_volume >= NoximGlobalParams::max_volume_to_be_drained)
									{
										total_sim_cycles = current_cycle;
										sc_stop();
									}
									else
									{
										drained_volume++;
										local_drained++;
									}
								}
							}

							if (NoximGlobalParams::mode_target_packets && (flit.flit_type == FLIT_TYPE_TAIL)) 
							{
								process_total++;
								NoximGlobalParams::processed_packets.push_back(flitToString(flit));
								if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
								{
									cout << "#[D] router " << local_id << ", cycle: " << current_cycle 
										<< ", prcess_total: " << process_total << endl;
								}
								if (process_total == NoximGlobalParams::target_packets)
								{
									total_sim_cycles = current_cycle;
									sc_stop();
								}	
							}
							
						}
						else if (i != DIRECTION_LOCAL)
						{
							// Increment routed flits counter
							routed_flits++;
						}
					}
				} // already reserved 
			}
		}
	} // else
	stats.power.Leakage();
}

NoximNoP_data NoximRouter::getCurrentNoPData() const
{
	NoximNoP_data NoP_data;

	for (int j = 0; j < DIRECTIONS; j++)
	{
		NoP_data.channel_status_neighbor[j].free_slots =
			free_slots_neighbor[j].read();
		NoP_data.channel_status_neighbor[j].available =
			(reservation_table.isAvailable(j));
	}

	NoP_data.sender_id = local_id;

	return NoP_data;
}

void NoximRouter::bufferMonitor()
{
	if (reset.read())
	{
		for (int i = 0; i < DIRECTIONS + 1; i++)
			free_slots[i].write(buffer[i].GetMaxBufferSize());
	}
	else
	{
		if (NoximGlobalParams::selection_strategy == SEL_BUFFER_LEVEL ||
			NoximGlobalParams::selection_strategy == SEL_NOP)
		{
			// update current input buffers level to neighbors
			for (int i = 0; i < DIRECTIONS + 1; i++)
				free_slots[i].write(buffer[i].getCurrentFreeSlots());

			// NoP selection: send neighbor info to each direction 'i'
			NoximNoP_data current_NoP_data = getCurrentNoPData();

			for (int i = 0; i < DIRECTIONS; i++)
				NoP_data_out[i].write(current_NoP_data);
		}
	}
}

void NoximRouter::showBufferSize()
{
	if(!reset.read())
	{
		int current_cycle =static_cast<int>(sc_time_stamp().to_double() / 1000 + 0.5);
		if(current_cycle % 100000 == 0)
		{
			cout << "#[D] buffer information of router: " << local_id << ", current cycle: " << current_cycle << endl;
			for (int i = 0; i < DIRECTIONS + 1; i++)
				cout << buffer[i].Size() << " ";
			cout << endl;
		}
	}
}

int NoximRouter::route(const NoximRouteData &route_data, bool &should_discard_local)
{
	stats.power.Routing();
	should_discard_local = false;
	if (route_data.dst_id == local_id)
		return DIRECTION_LOCAL;

	vector<int> candidate_channels = routing_algo->route(route_data);
	// select the candidate direction, at which router is not faulty
	vector<int> filtered_channels;
	for (int i = 0; i < candidate_channels.size(); ++i)
	{
		int channel = candidate_channels[i];
		if(channel != NOT_VALID)
		{
			int neighbor_id = getNeighborId(route_data.current_id, channel);
			//assert(neighbor_id != NOT_VALID);
			if (neighbor_id != NOT_VALID && isRouterGood(neighbor_id))
				filtered_channels.push_back(channel);
		}
	}
	if (filtered_channels.size() > 0)
		return selectionFunction(filtered_channels, route_data);
	else
	{
		should_discard_local = true;
		return DIRECTION_LOCAL;
	}
}

void NoximRouter::NoP_report() const
{
	NoximNoP_data NoP_tmp;
	cout << sc_time_stamp().to_double() /
				1000
		 << ": Router[" << local_id << "] NoP report: " << endl;

	for (int i = 0; i < DIRECTIONS; i++)
	{
		NoP_tmp = NoP_data_in[i].read();
		if (NoP_tmp.sender_id != NOT_VALID)
			cout << NoP_tmp;
	}
}

//---------------------------------------------------------------------------

int NoximRouter::NoPScore(const NoximNoP_data &nop_data,
						  const vector<int> &nop_channels) const
{
	int score = 0;

	for (unsigned int i = 0; i < nop_channels.size(); i++)
	{
		int available;

		if (nop_data.channel_status_neighbor[nop_channels[i]].available)
			available = 1;
		else
			available = 0;

		int free_slots =
			nop_data.channel_status_neighbor[nop_channels[i]].free_slots;

		score += available * free_slots;
	}

	return score;
}

int NoximRouter::selectionNoP(const vector<int> &directions,
							  const NoximRouteData &route_data)
{
	vector<int> neighbors_on_path;
	vector<int> score;
	int direction_selected = NOT_VALID;

	int current_id = route_data.current_id;

	for (uint i = 0; i < directions.size(); i++)
	{
		// get id of adjacent candidate
		int candidate_id = getNeighborId(current_id, directions[i]);

		// apply routing function to the adjacent candidate node
		NoximRouteData tmp_route_data;
		tmp_route_data.current_id = candidate_id;
		tmp_route_data.src_id = route_data.src_id;
		tmp_route_data.dst_id = route_data.dst_id;
		tmp_route_data.dir_in = reflexDirection(directions[i]);

		vector<int> next_candidate_channels = routing_algo->route(route_data);

		// select useful data from Neighbor-on-Path input
		NoximNoP_data nop_tmp = NoP_data_in[directions[i]].read();

		// store the score of node in the direction[i]
		score.push_back(NoPScore(nop_tmp, next_candidate_channels));
	}

	// check for direction with higher score
	int max_direction = directions[0];
	int max = score[0];
	for (unsigned int i = 0; i < directions.size(); i++)
	{
		if (score[i] > max)
		{
			max_direction = directions[i];
			max = score[i];
		}
	}

	// if multiple direction have the same score = max, choose randomly.

	vector<int> equivalent_directions;

	for (unsigned int i = 0; i < directions.size(); i++)
		if (score[i] == max)
			equivalent_directions.push_back(directions[i]);

	direction_selected =
		equivalent_directions[rand() % equivalent_directions.size()];

	return direction_selected;
}

int NoximRouter::selectionBufferLevel(const vector<int> &directions)
{
	vector<int> best_dirs;
	int max_free_slots = 0;
	for (unsigned int i = 0; i < directions.size(); i++)
	{
		int free_slots = free_slots_neighbor[directions[i]].read();
		bool available = reservation_table.isAvailable(directions[i]);
		if (available)
		{
			if (free_slots > max_free_slots)
			{
				max_free_slots = free_slots;
				best_dirs.clear();
				best_dirs.push_back(directions[i]);
			}
			else if (free_slots == max_free_slots)
				best_dirs.push_back(directions[i]);
		}
	}

	if (best_dirs.size())
		return (best_dirs[rand() % best_dirs.size()]);
	else
		return (directions[rand() % directions.size()]);
}

int NoximRouter::selectionRandom(const vector<int> &directions)
{
	return directions[rand() % directions.size()];
}

int NoximRouter::selectionFunction(const vector<int> &directions,
								   const NoximRouteData &route_data)
{
	// not so elegant but fast escape
	if (directions.size() == 1)
		return directions[0];

	stats.power.Selection();

	switch (NoximGlobalParams::selection_strategy)
	{
	case SEL_RANDOM:
		return selectionRandom(directions);
	case SEL_BUFFER_LEVEL:
		return selectionBufferLevel(directions);
	case SEL_NOP:
		return selectionNoP(directions, route_data);
	default:
		assert(false);
	}

	return 0;
}



void NoximRouter::configure(const int _id,
							const double _warm_up_time,
							const unsigned int _max_buffer_size,
							NoximGlobalRoutingTable &grt)
{
	local_id = _id;
	stats.configure(_id, _warm_up_time);

	start_from_port = DIRECTION_LOCAL;

	if (grt.isValid())
		routing_table.configure(grt, _id);

	for (int i = 0; i < DIRECTIONS + 1; i++)
		buffer[i].SetMaxBufferSize(_max_buffer_size);

	int row = _id / NoximGlobalParams::mesh_dim_x;
	int col = _id % NoximGlobalParams::mesh_dim_x;
	if (row == 0)
		buffer[DIRECTION_NORTH].Disable();
	if (row == NoximGlobalParams::mesh_dim_y - 1)
		buffer[DIRECTION_SOUTH].Disable();
	if (col == 0)
		buffer[DIRECTION_WEST].Disable();
	if (col == NoximGlobalParams::mesh_dim_x - 1)
		buffer[DIRECTION_EAST].Disable();
}

unsigned long NoximRouter::getRoutedFlits()
{
	return routed_flits;
}

unsigned int NoximRouter::getFlitsCount()
{
	unsigned count = 0;

	for (int i = 0; i < DIRECTIONS + 1; i++)
		count += buffer[i].Size();

	return count;
}

double NoximRouter::getPower()
{
	return stats.power.getPower();
}

int NoximRouter::reflexDirection(int direction) const
{
	if (direction == DIRECTION_NORTH)
		return DIRECTION_SOUTH;
	if (direction == DIRECTION_EAST)
		return DIRECTION_WEST;
	if (direction == DIRECTION_WEST)
		return DIRECTION_EAST;
	if (direction == DIRECTION_SOUTH)
		return DIRECTION_NORTH;

	// you shouldn't be here
	assert(false);
	return NOT_VALID;
}

bool NoximRouter::inCongestion()
{
	for (int i = 0; i < DIRECTIONS; i++)
	{
		int flits =
			NoximGlobalParams::buffer_depth - free_slots_neighbor[i];
		if (flits >
			(int)(NoximGlobalParams::buffer_depth *
				  NoximGlobalParams::dyad_threshold))
			return true;
	}

	return false;
}

void NoximRouter::ShowBuffersStats(std::ostream &out)
{
	for (int i = 0; i < DIRECTIONS + 1; i++)
		buffer[i].ShowStats(out);
}
