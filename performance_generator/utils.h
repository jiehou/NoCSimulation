#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <cassert>
#include <set>

#define RESULT_FOLDER "../res/mesh/"
#define TEMP_INJECT_FAULTS "../bin/.faults.tmp"
#define NOXIM "../bin/noxim"
#define COMMAND_THROUGHPUT " -sim 10000"
#define COMMAND_FAULT_RESILIENCE  " -packets "
#define COMMAND_COMMUNICATION_TIME " -sim 1000000 -volume "
#define SIM_FAULT_RESILIENCE "fr" // fault resilience 
#define SIM_COMMUNICATION_TIME "ct" // communication time
#define TEMP_RESULT_FILE "./.result.tmp"

#define GLOBAL_THROUGHPUT_LABEL     "% Global average throughput (flits/cycle): "
#define GLOBAL_AVERAGE_DELAY_LABEL  "% Global average delay (cycles): "
#define THROUGHPUT_LABEL            "% Throughput (flits/cycle/IP): "
#define RECV_FLITS_LABEL            "% Total received flits: "
#define COMMUNICATION_TIME_LABEL    "% Communication time: "
#define NETWORK_SATURATED_LABEL     "% Network saturated: "
#define NUM_PACKETS     5000

using namespace std;

const string FAULT_FOLDER = "/home/houje/Work/HexagonalNoC/data/fault_combs_10percent/mesh10x10/";
const int    MAX_SAMPLES_PER_STATE = 5000;

struct Param
{
    int dimx;
    int dimy;
    int packet_size;
    int buffer_depth;
    double pir;
    string routing_algo; // 0: xy, 1: ftnf
    string sim_mode;
};

struct Result
{
    double global_throughput;
    double global_avg_delay;
    double throughput;
    int recv_flits;
    int communication_time;
};

template <typename T>
ostream &operator<<(ostream &os, const vector<T> &vec)
{
    for (auto &el : vec)
        os << el << " ";
    return os;
}

template <typename T>
void readMatrix(const string &file, vector<vector<T>> &matrix)
{
    ifstream file_in(file);
    if (file_in.is_open())
    {
        string line;
        int element;
        vector<T> row;
        while (getline(file_in, line))
        {
            if (!line.empty())
            {
                istringstream iss(line);
                while (iss >> element)
                {
                    row.push_back(element);
                }
                matrix.push_back(row);
                row.clear();
            }
        }
        file_in.close(); // read ends
    }
    else
    {
        cout << "#[I] cannot open " << file << endl;
        exit(1);
    }
} 

void readFaultCombinations(int state_id, 
        vector<vector<int>> &router_combs)
{
    // router combinations
    string file_routers = FAULT_FOLDER + to_string(state_id) + ".txt";
    readMatrix(file_routers, router_combs);
}

string createRouterComb(const vector<int>& routers) {
    string res = "";
    for(auto router : routers) res += to_string(router) + " ";
    return res;
}

string createCommand(const Param& param, const string &router_comb)
{
    string command = string(NOXIM);
    // dimension
    command += " -dimx " + to_string(param.dimx) + " -dimy " + to_string(param.dimy);
    // packet size
    command += " -size " + to_string(param.packet_size) + " " + to_string(param.packet_size);
    // buffer depth
    command += " -buffer " + to_string(param.buffer_depth);
    // packet injection rate
    command += " -pir " + to_string(param.pir).substr(0, 6) + " possion";
    // routing
    command += " -routing " + param.routing_algo;
    // router combination
    if(!router_comb.empty())
        command += " -frouters " + router_comb;
    // sim mode
    if(param.sim_mode == SIM_COMMUNICATION_TIME) // mode: communication time
        command += string(COMMAND_COMMUNICATION_TIME) + to_string(NUM_PACKETS * param.packet_size);
    else if(param.sim_mode == SIM_FAULT_RESILIENCE) // mode: fault resilience
        command += string(COMMAND_FAULT_RESILIENCE) + to_string(NUM_PACKETS);
    else // mode: throughput
        command += string(COMMAND_THROUGHPUT);
    //command += " -fdir " + param.fdir;
    return command;
}

bool readResult(Result &res, const string& result_file)
{
    ifstream file_in(result_file);
    if(file_in.is_open())
    {
        while(!file_in.eof())
        {
            string line;
            getline(file_in, line);
            size_t pos;
            // network saturated
            pos = line.find(NETWORK_SATURATED_LABEL);
            if(pos != string::npos)
            {
                return false; // result is wrong, drop it
            }
            // global average delay
            pos = line .find(GLOBAL_AVERAGE_DELAY_LABEL);
            if(pos != string::npos)
            {
                istringstream iss(line.substr(pos + string(GLOBAL_AVERAGE_DELAY_LABEL).size()));
                iss >> res.global_avg_delay;
                continue;
            }
            // communication time
            pos = line.find(COMMUNICATION_TIME_LABEL);
            if(pos != string::npos)
            {
                istringstream iss(line.substr(pos + string(COMMUNICATION_TIME_LABEL).size()));
                iss >> res.communication_time;
                continue;
            }
            // total received flits
            pos = line.find(RECV_FLITS_LABEL);
            if(pos != string::npos)
            {
                istringstream iss(line.substr(pos + string(RECV_FLITS_LABEL).size()));
                iss >> res.recv_flits;
            }
            // global throughput
            pos = line.find(GLOBAL_THROUGHPUT_LABEL);
            if(pos != string::npos)
            {
                istringstream iss(line.substr(pos + string(GLOBAL_THROUGHPUT_LABEL).size()));
                iss >> res.global_throughput;
                continue;
            }
            // throughput
            pos = line.find(THROUGHPUT_LABEL);
            if(pos != string::npos)
            {
                istringstream iss(line.substr(pos + string(THROUGHPUT_LABEL).size()));
                iss >> res.throughput;
                continue;
            }
        }
        file_in.close();
        return true;
    }
    else
    {
        cout << "#[I] cannot open file " << result_file << endl;
        return false;
    }
}

/*
* based on the created command, run simulation and save result of interest into 
* the specified file. file name template: size_routing_mode.csv. for one size,
* there are three files. e.g.: 36_xy_throughput.csv, 36_xy_fault_resilience.csv,
* 36_xy_communication_time.csv
*/
bool runSimulation(const string &command_base, int state_id, Result &res)
{
    // 1. run simulation
    string result_file = string(TEMP_RESULT_FILE) + to_string(state_id);
    string rm_cmd = string("rm -f ") + result_file;
    system(rm_cmd.c_str()); // if exists, first delete .result.tmp

    string sim_cmd = command_base + " >" + result_file + " 2>&1";
    system(sim_cmd.c_str());
    // 2. read simulation result
    bool should_continue = readResult(res, result_file);
    return should_continue;
}

int randInt(int min, int max)
{
    random_device rd;
    mt19937 eng(rd()); // engine
    uniform_int_distribution<> dist(min, max);
    return dist(eng);
}
#endif
