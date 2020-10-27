/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the top-level of Noxim
 */

#ifndef __NOXIMMAIN_H__
#define __NOXIMMAIN_H__

#include <cassert>
#include <systemc.h>
#include <vector>
#include <map>
#include <string>
using namespace std;

// Define the directions as numbers
#define DIRECTIONS 4
#define DIRECTION_NORTH 0
#define DIRECTION_EAST 1
#define DIRECTION_SOUTH 2
#define DIRECTION_WEST 3
#define DIRECTION_LOCAL 4

// Generic not reserved resource
#define NOT_RESERVED -2

// To mark invalid or non exhistent values
#define NOT_VALID -1

// Routing algorithms
#define ROUTING_XY 0
#define ROUTING_WEST_FIRST 1
#define ROUTING_NORTH_LAST 2
#define ROUTING_NEGATIVE_FIRST 3
#define ROUTING_ODD_EVEN 4
#define ROUTING_DYAD 5
#define ROUTING_FTNF 6
#define ROUTING_FULLY_ADAPTIVE 8
#define ROUTING_TABLE_BASED 9
#define INVALID_ROUTING -1

// Selection strategies
#define SEL_RANDOM 0
#define SEL_BUFFER_LEVEL 1
#define SEL_NOP 2
#define INVALID_SELECTION -1

// Traffic distribution
#define TRAFFIC_RANDOM 0
#define TRAFFIC_TRANSPOSE1 1
#define TRAFFIC_TRANSPOSE2 2
#define TRAFFIC_HOTSPOT 3
#define TRAFFIC_TABLE_BASED 4
#define TRAFFIC_BIT_REVERSAL 5
#define TRAFFIC_SHUFFLE 6
#define TRAFFIC_BUTTERFLY 7
#define INVALID_TRAFFIC -1

// Verbosity levels
#define VERBOSE_OFF 0
#define VERBOSE_LOW 1
#define VERBOSE_MEDIUM 2
#define VERBOSE_HIGH 3

// Default configuration can be overridden with command-line arguments
#define DEFAULT_VERBOSE_MODE VERBOSE_OFF
#define DEFAULT_TRACE_MODE false
#define DEFAULT_TRACE_FILENAME ""
#define DEFAULT_MESH_DIM_X 4
#define DEFAULT_MESH_DIM_Y 4
#define DEFAULT_BUFFER_DEPTH 4
#define DEFAULT_MAX_PACKET_SIZE 10
#define DEFAULT_MIN_PACKET_SIZE 2
#define DEFAULT_ROUTING_ALGORITHM ROUTING_XY
#define DEFAULT_ROUTING_TABLE_FILENAME ""
#define DEFAULT_SELECTION_STRATEGY SEL_RANDOM
#define DEFAULT_PACKET_INJECTION_RATE 0.01
#define DEFAULT_PROBABILITY_OF_RETRANSMISSION 0.01
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_RANDOM
#define DEFAULT_TRAFFIC_TABLE_FILENAME ""
#define DEFAULT_RESET_TIME 1
#define DEFAULT_SIMULATION_TIME 20000
#define DEFAULT_STATS_WARM_UP_TIME 0
#define DEFAULT_DETAILED false
#define DEFAULT_DYAD_THRESHOLD 0.6
#define DEFAULT_MAX_VOLUME_TO_BE_DRAINED 0
#define DEFAULT_ROUTER_PWR_FILENAME "default_router.pwr"
#define DEFAULT_LOW_POWER_LINK_STRATEGY false
#define DEFAULT_QOS 1.0
#define DEFAULT_SHOW_BUFFER_STATS false
#define DEFAULT_TARGET_PACKETS 0
#define MAX_SIMULATION_TIME 500000
#define NOXIM_DIR "/home/houje/Work/ml_noxim/bin/"
#define TEMP_INJECT_FAULTS ".faults.tmp"

// TODO by Fafa - this MUST be removed!!! Use only STL vectors instead!!!
#define MAX_STATIC_DIM 32

typedef unsigned int uint;

// NoximGlobalParams -- used to forward configuration to every sub-block
struct NoximGlobalParams
{
    static int verbose_mode;
    static int trace_mode;
    static char trace_filename[128];
    static int mesh_dim_x;
    static int mesh_dim_y;
    static int buffer_depth;
    static int min_packet_size;
    static int max_packet_size;
    static int routing_algorithm;
    static char routing_table_filename[128];
    static int selection_strategy;
    static float packet_injection_rate;
    static float probability_of_retransmission;
    static int traffic_distribution;
    static char traffic_table_filename[128];
    static int simulation_time;
    static int stats_warm_up_time;
    static int rnd_generator_seed;
    static bool detailed;
    static vector<pair<int, double> > hotspots;
    static float dyad_threshold;
    static unsigned int max_volume_to_be_drained;
    static char router_power_filename[128];
    static bool low_power_link_strategy;
    static double qos;
    static bool show_buffer_stats;
    static vector<int> faulty_routers;
    static int target_packets;
    static bool mode_target_packets;
    static map<string, vector<int> > hops_info;
    static vector<string> processed_packets;
    static bool network_saturated;
    static int dropped_flits;
};

// NoximCoord -- XY coordinates type of the Tile inside the Mesh
class NoximCoord
{
  public:
    int x; // X coordinate
    int y; // Y coordinate

    inline bool operator==(const NoximCoord &coord) const
    {
        return (coord.x == x && coord.y == y);
    }
};

// NoximFlitType -- Flit type enumeration
enum NoximFlitType
{
    FLIT_TYPE_HEAD,
    FLIT_TYPE_BODY,
    FLIT_TYPE_TAIL
};

// fault-tolerant negative first routing case
enum FtnfRoutingDirection
{
    FTNF_NORTH_EAST, // NE
    FTNF_EAST,       // E
    FTNF_NORTH,      // N
    FTNF_NORTH_WEST, // NW
    FTNF_WEST,       // W
    FTNF_SOUTH_EAST, // SE
    FTNF_SOUTH,      // S
    FTNF_SOUTH_WEST  // SW
};

// NoximPayload -- Payload definition
struct NoximPayload
{
    sc_uint<32> data; // Bus for the data to be exchanged
    inline bool operator==(const NoximPayload &payload) const
    {
        return (payload.data == data);
    }
};

// NoximPacket -- Packet definition
struct NoximPacket
{
    int src_id;
    int dst_id;
    double timestamp; // SC timestamp at packet generation
    int size;
    int flit_left; // Number of remaining flits inside the packet
    bool use_low_voltage_path;

    // Constructors
    NoximPacket() {}

    NoximPacket(const int s, const int d, const double ts, const int sz)
    {
        make(s, d, ts, sz);
    }

    void make(const int s, const int d, const double ts, const int sz)
    {
        src_id = s;
        dst_id = d;
        timestamp = ts;
        size = sz;
        flit_left = sz;
        use_low_voltage_path = false;
    }
};

// NoximRouteData -- data required to perform routing
struct NoximRouteData
{
    int current_id;
    int src_id;
    int dst_id;
    int dir_in; // direction from which the packet comes from
};

struct NoximChannelStatus
{
    int free_slots; // occupied buffer slots
    bool available; //
    inline bool operator==(const NoximChannelStatus &bs) const
    {
        return (free_slots == bs.free_slots && available == bs.available);
    };
};

// NoximNoP_data -- NoP Data definition
struct NoximNoP_data
{
    int sender_id;
    NoximChannelStatus channel_status_neighbor[DIRECTIONS];

    inline bool operator==(const NoximNoP_data &nop_data) const
    {
        return (sender_id == nop_data.sender_id &&
                nop_data.channel_status_neighbor[0] ==
                    channel_status_neighbor[0] &&
                nop_data.channel_status_neighbor[1] ==
                    channel_status_neighbor[1] &&
                nop_data.channel_status_neighbor[2] ==
                    channel_status_neighbor[2] &&
                nop_data.channel_status_neighbor[3] ==
                    channel_status_neighbor[3]);
    };
};

// NoximFlit -- Flit definition
struct NoximFlit
{
    int src_id;
    int dst_id;
    NoximFlitType flit_type; // The flit type (FLIT_TYPE_HEAD, FLIT_TYPE_BODY, FLIT_TYPE_TAIL)
    int sequence_no;         // The sequence number of the flit inside the packet
    NoximPayload payload;    // Optional payload
    double timestamp;        // Unix timestamp at packet generation
    int hop_no;              // Current number of hops from source to destination
    bool use_low_voltage_path;

    inline bool operator==(const NoximFlit &flit) const
    {
        return (flit.src_id == src_id && flit.dst_id == dst_id && flit.flit_type == flit_type 
            && flit.sequence_no == sequence_no && flit.payload == payload && flit.timestamp == timestamp 
            && flit.hop_no == hop_no && flit.use_low_voltage_path == use_low_voltage_path);
    }
};

// Output overloading

inline ostream &operator<<(ostream &os, const NoximFlit &flit)
{

    if (NoximGlobalParams::verbose_mode == VERBOSE_HIGH)
    {
        os << "### FLIT ###" << endl;
        os << "Source Tile[" << flit.src_id << "]" << endl;
        os << "Destination Tile[" << flit.dst_id << "]" << endl;
        switch (flit.flit_type)
        {
        case FLIT_TYPE_HEAD:
            os << "Flit Type is HEAD" << endl;
            break;
        case FLIT_TYPE_BODY:
            os << "Flit Type is BODY" << endl;
            break;
        case FLIT_TYPE_TAIL:
            os << "Flit Type is TAIL" << endl;
            break;
        }
        os << "Sequence no. " << flit.sequence_no << endl;
        //os << "Payload printing not implemented (yet)." << endl;
        os << "Unix timestamp at packet generation " << flit.timestamp << endl;
        os << "Total number of hops from source to destination is " << flit.hop_no << endl;
    }
    else
    {
        os << "[type: ";
        switch (flit.flit_type)
        {
        case FLIT_TYPE_HEAD:
            os << "H";
            break;
        case FLIT_TYPE_BODY:
            os << "B";
            break;
        case FLIT_TYPE_TAIL:
            os << "T";
            break;
        }
        os << ", seq: " << flit.sequence_no << ", " << flit.src_id << "-->" << flit.dst_id << "]";
    }
    return os;
}

inline ostream &operator<<(ostream &os,
                           const NoximChannelStatus &status)
{
    char msg;
    if (status.available)
        msg = 'A';
    else
        msg = 'N';
    os << msg << "(" << status.free_slots << ")";
    return os;
}

inline ostream &operator<<(ostream &os, const NoximNoP_data &NoP_data)
{
    os << "      NoP data from [" << NoP_data.sender_id << "] [ ";

    for (int j = 0; j < DIRECTIONS; j++)
        os << NoP_data.channel_status_neighbor[j] << " ";

    cout << "]" << endl;
    return os;
}

inline ostream &operator<<(ostream &os, const NoximCoord &coord)
{
    os << "(" << coord.x << "," << coord.y << ")";

    return os;
}

// Trace overloading
inline void sc_trace(sc_trace_file *&tf, const NoximFlit &flit, string &name)
{
    sc_trace(tf, flit.src_id, name + ".src_id");
    sc_trace(tf, flit.dst_id, name + ".dst_id");
    sc_trace(tf, flit.sequence_no, name + ".sequence_no");
    sc_trace(tf, flit.timestamp, name + ".timestamp");
    sc_trace(tf, flit.hop_no, name + ".hop_no");
}

inline void sc_trace(sc_trace_file *&tf, const NoximNoP_data &NoP_data, string &name)
{
    sc_trace(tf, NoP_data.sender_id, name + ".sender_id");
}

inline void sc_trace(sc_trace_file *&tf, const NoximChannelStatus &bs, string &name)
{
    sc_trace(tf, bs.free_slots, name + ".free_slots");
    sc_trace(tf, bs.available, name + ".available");
}

// Misc common functions
inline NoximCoord id2Coord(int id)
{
    NoximCoord coord;

    coord.x = id % NoximGlobalParams::mesh_dim_x;
    coord.y = id / NoximGlobalParams::mesh_dim_x;
    if(coord.x >= NoximGlobalParams::mesh_dim_x || coord.y >= NoximGlobalParams::mesh_dim_y)
        cout << "#[D] id: " << id << ", x: " << coord.x << ",y: " << coord.y << endl; 
    assert(coord.x < NoximGlobalParams::mesh_dim_x);
    assert(coord.y < NoximGlobalParams::mesh_dim_y);

    return coord;
}

inline int coord2Id(const NoximCoord &coord)
{
    int id = (coord.y * NoximGlobalParams::mesh_dim_x) + coord.x;

    assert(id < NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y);

    return id;
}

inline int getNeighborId(int _id, int direction)
{
    NoximCoord my_coord = id2Coord(_id);
    assert(direction >= 0); // if direction is NOT_VALID, report error
    //cout << "#[D] x: " << my_coord.x << ", y: " << my_coord.y << ", direction: " << direction << endl;
    switch (direction)
    {
    case DIRECTION_NORTH:
        if (my_coord.y == 0)
            return NOT_VALID;
        my_coord.y--;
        break;
    case DIRECTION_SOUTH:
        if (my_coord.y == NoximGlobalParams::mesh_dim_y - 1)
            return NOT_VALID;
        my_coord.y++;
        break;
    case DIRECTION_EAST:
        if (my_coord.x == NoximGlobalParams::mesh_dim_x - 1)
            return NOT_VALID;
        my_coord.x++;
        break;
    case DIRECTION_WEST:
        if (my_coord.x == 0)
            return NOT_VALID;
        my_coord.x--;
        break;
    default:
        cout << "direction not valid : " << direction;
        assert(false);
    }
    int neighbor_id = coord2Id(my_coord);
    return neighbor_id;
}

/*
* check whether the connected router is faulty or not.
* @router_id: connected neighbor id
* output: true (good), false(faulty)
*/
inline bool isRouterGood(int router_id)
{
    // check the status of connected router
    vector<int>::iterator it = find(NoximGlobalParams::faulty_routers.begin(),
                                    NoximGlobalParams::faulty_routers.end(), router_id);
    if (it == NoximGlobalParams::faulty_routers.end())
    {
        return true;
    }
    return false;
}

/*
* check whether the router is located on the specified edge
* @router_id: router id
* @edge_position: edge position (N, E, S, W)
* output: true (located on the specified edge), otherwise false
*/
inline bool locatedOnEdge(int router_id, int edge_position)
{
    // N: 0, E: 1, S: 2, W: 3
    assert(edge_position >= 0 && edge_position < 4);
    int neighbor_id = getNeighborId(router_id, edge_position);
    return (neighbor_id == NOT_VALID);
}

inline string ftnfRoutingDirectionToString(const FtnfRoutingDirection &routing_direction)
{
    switch (routing_direction)
    {
    case FTNF_NORTH_EAST:
        return "NE";
    case FTNF_NORTH:
        return "N";
    case FTNF_EAST:
        return "E";
    case FTNF_NORTH_WEST:
        return "NW";
    case FTNF_WEST:
        return "W";
    case FTNF_SOUTH_EAST:
        return "SE";
    case FTNF_SOUTH:
        return "S";
    case FTNF_SOUTH_WEST:
        return "SW";
    }
}

inline string directionToString(int direction)
{
    switch (direction)
    {
    case DIRECTION_NORTH: 
        return "North";
    case DIRECTION_EAST:
        return "East";
    case DIRECTION_SOUTH:
        return "South";
    case DIRECTION_WEST:
        return "West";
    case DIRECTION_LOCAL:
        return "Local";
    }
}

inline void injectFaults()
{
    int size = NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y;
    string file_path = string(NOXIM_DIR) + string(TEMP_INJECT_FAULTS) + to_string(size);
    ifstream file_in(file_path);
    if (file_in.is_open())
    {
        cout << "#[I] open file " << file_path << endl;
        string line;
        while (getline(file_in, line))
        {
            if(!line.empty()) // make sure line is not empty
                NoximGlobalParams::faulty_routers.push_back(std::stoi(line));
        }
    }
    else
    {
        cout << "#[I] cannot open " << file_path << endl;
    }
}

inline void debugFlit(const NoximFlit& flit, double arrival_time)
{
    if(flit.flit_type == FLIT_TYPE_HEAD)
    {
        cout << "#[D] src: " << flit.src_id << ", dst: " << flit.dst_id << ", generate: " << flit.timestamp << ", arrive: " << arrival_time 
            << ", hop_count: " << flit.hop_no << endl;
    }
}

inline string flitToString(const NoximFlit& flit)
{
    int timestamp = static_cast<int>(flit.timestamp);
    return to_string(flit.src_id) +  "_" + to_string(flit.dst_id) + "_" + to_string(timestamp);
}


inline void outputHopsInfo()
{
    for(auto it = NoximGlobalParams::hops_info.begin(); it != NoximGlobalParams::hops_info.end(); ++it)
    {
        // if the packet has been processed (successfully delivered or dropped), they will not be displayed
        bool found = false;    
        for(string elem : NoximGlobalParams::processed_packets)
        {
            //cout << "#[D] elem: " < elem.c_str() << endl;
            if(elem == it->first)
            {    
                found = true;
                continue;
            }
        }

        if(!found)
        {
            cout << "#[D] packet: " << it->first << endl; // scr_dst_timestamp
            for(auto elem : it->second)
                cout << elem << " ";
            cout << endl;
        }
    }
}

inline void outputAvgHops()
{
    int num_packets = 0;
    int sum_hops = 0;
    for(auto it = NoximGlobalParams::hops_info.begin(); it != NoximGlobalParams::hops_info.end(); ++it)
    {
        cout << "#[D] " << it->first << ", " << (it->second).size() << endl;
        num_packets += 1;
        sum_hops += (it->second).size() - 1;
    }
    cout << "#[D] num_packets: " << num_packets << ", avg hops: " << sum_hops*1.0 / num_packets << endl;
}
#endif
