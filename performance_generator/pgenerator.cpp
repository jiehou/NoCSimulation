#include "argparse.h"
#include "utils.h"
#include <chrono>
#include <thread>
using namespace std;

vector<vector<int>> links;
vector<vector<int>> neighbors;
Param param;
const int kNumLoops = 1000;

void simulation(int begin, int end,  vector<double> &res)
{
    //cout << "#[D] begin: " << begin << ", end: " << end << endl;
    for(int state = begin; state <= end; ++state) //for(int state = 100; state <= 675; ++state)
	{
		vector<vector<int>> router_combs;
		readFaultCombinations(state, router_combs);
		int num_combs = router_combs.size();
		//std::cout << "#[D] state: " << state << ", num_combs: " << num_combs << std::endl; 
		//1) load fault combs of routers
		//2) generate simulation command
		//3) start simulation and obtain result
		//if(num_combs > MAX_SAMPLES_PER_STATE) num_combs = MAX_SAMPLES_PER_STATE;
		Result sim_res;
		int num_loops = 0;
		bool flag_valid;
		double sum_fr = 0.0;
		long long sum_ct = 0;
		double avg_fr = 0.0, avg_ct = 0.0;
		for(int i = 0; i < kNumLoops; ++i) 
		{
			int rand_idx = randInt(0, num_combs-1);
			string router_comb = createRouterComb(router_combs[rand_idx]);
			string sim_command = createCommand(param, router_comb);
			//std::cout << "[D] i: " << i << ", idx: " << rand_idx << ", com: " << sim_command << std::endl;
			flag_valid = runSimulation(sim_command, state, sim_res);
			if(flag_valid)
			{
                //cout << "[D] state " << state << ", : " << sim_res.communication_time << endl;
				num_loops += 1;
				if(param.sim_mode == SIM_COMMUNICATION_TIME)
					sum_ct += sim_res.communication_time;
				if(param.sim_mode == SIM_FAULT_RESILIENCE)
					sum_fr += sim_res.recv_flits * 1.0 / (NUM_PACKETS*param.packet_size);
			}
			else
			{
				cout << "[ERROR] state: " << state << ", num_loops: " << num_loops 
					 << " network saturation occurs" << endl;
				break;
			}
		} // fault combinations
		if(param.sim_mode == SIM_COMMUNICATION_TIME)
		{
			avg_ct = sum_ct * 1.0 / num_loops;
			//cout << "[INFO] state: " << state << ", avg_ct: " << avg_ct << endl;
			//cout << avg_ct << endl;
            res.push_back(avg_ct);
		}
		if(param.sim_mode == SIM_FAULT_RESILIENCE)
		{
			avg_fr = sum_fr * 1.0 / num_loops;
			//cout << "[INFO] state: " << state << ", avg_fr: " << to_string(avg_fr) << endl;
			//cout << to_string(avg_fr) << endl;
            res.push_back(avg_fr);
		}
	}
}

double simState0() 
{
	Result sim_res;
	int num_loops = 0;
	bool flag_valid;
	double sum_fr = 0.0;
	long long sum_ct = 0;
	for(int i = 0; i < kNumLoops; ++i) 
	{
		string router_comb = "";
		string sim_command = createCommand(param, router_comb);
		//std::cout << "[D] com: " << sim_command << std::endl;
		flag_valid = runSimulation(sim_command, 0, sim_res);
		if(flag_valid)
		{
			num_loops += 1;
			if(param.sim_mode == SIM_COMMUNICATION_TIME)
				sum_ct += sim_res.communication_time;
			if(param.sim_mode == SIM_FAULT_RESILIENCE)
				sum_fr += sim_res.recv_flits * 1.0 / (NUM_PACKETS*param.packet_size);
		}
		else
		{
			cout << "[ERROR] state: 0, num_loops: " << num_loops 
				 << " network saturation occurs" << endl;
			break;
		}
	} // fault combinations
	if(param.sim_mode == SIM_COMMUNICATION_TIME)
		return sum_ct * 1.0 / num_loops;
	return sum_fr * 1.0 / num_loops;
}

int main(int argc, char* argv[]) {
    ArgumentParser parser("Simulation result generator");
	parser.add_argument("--dimx", "dimx", true);
	parser.add_argument("--dimy", "dimy", true);
	parser.add_argument("--routing", "routing algorithm (xy and ftnf)", true);
	parser.add_argument("--mode", "communication time (ct) or fault resilience (fr)", true);
	parser.add_argument("--pir", "packet inject rate", true);
	try {
		parser.parse(argc, argv);
	}
	catch(const ArgumentParser::ArgumentNotFound& e) {
		std::cout << e.what() << '\n';
		return 0;
	}
	if(parser.is_help()) return 0;
	double pir = parser.get<double>("pir");
	param.packet_size = 5;
	param.buffer_depth = 6;
	param.routing_algo = parser.get<string>("routing"); 
	param.sim_mode = parser.get<string>("mode");
	param.pir = pir;
	param.dimx = parser.get<int>("dimx");
	param.dimy = parser.get<int>("dimy");
	std::cout << "[I] routing: " << param.routing_algo << ", mode: " << param.sim_mode << "\n";
	//std::cout << simState0() << "\n";
	/*
	vector<double> res;
	simulation(60, 60, res);
	for(auto e : res)
		std::cout << e << " ";
	std::cout << "\n";
	*/
    // create four threads
	auto start = chrono::steady_clock::now();
    vector<double> res0;
    thread t0(simulation, 1, 58, ref(res0)); // 1, 27
    vector<double> res1;
    thread t1(simulation, 59, 115, ref(res1)); // 28, 54
	vector<double> res2;
    thread t2(simulation, 116, 172, ref(res2)); // 55, 81
	vector<double> res3;
    thread t3(simulation, 173, 229, ref(res3)); // 82, 109
    t0.join();
    t1.join();
	t2.join();
	t3.join();
	auto end = chrono::steady_clock::now();
	std::cout << "#[I] total execution time (minute): " 
		      << chrono::duration_cast<chrono::minutes>(end - start).count() << "\n";
	for(auto e : res0)
        std::cout << e << "\n";
    for(auto e : res1)
        std::cout << e << "\n";
	for(auto e : res2)
        std::cout << e << "\n";
	for(auto e : res3)
        std::cout << e << "\n";
    return 0;
}