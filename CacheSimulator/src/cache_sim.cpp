#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
//#include "request_handlers.cpp"
#include "request_handlers.cpp"
#include <vector>
#include "parse.h"
using namespace std;



unsigned int L1_SIZE;            //L1 cache size in Bytes
unsigned int L1_ASSOC;           //L1 set-associativity
unsigned int L1_BLOCKSIZE;       //L1 block size in Bytes

unsigned int VC_NUM_BLOCKS;		 //Number of blocks in the Victim Cache (0 if no VC)

unsigned int L2_SIZE;            //L2 cache size in bytes. (0 if no L2)
unsigned int L2_ASSOC;           //L2 set-associativity

char *tracefile;                 //Character string specifying the full name of trace file


int extractBits(int number, int p, int q) {
    int mask = ((1 << (q - p + 1)) - 1) << p;
    return (number & mask) >> p;
}

void print_stats(CACHEMEMORY* L1, CACHEMEMORY* L2, unsigned int L1_BLOCKSIZE, unsigned int L1_SIZE, unsigned int L1_ASSOC, unsigned int L2_ASSOC, unsigned int L2_SIZE);

void print_stats(L1VC* L1, CACHEMEMORY* L2, unsigned int L1_BLOCKSIZE, unsigned int L1_SIZE, unsigned int L1_ASSOC, unsigned int VC_NUM_BLOCKS, unsigned int L2_ASSOC, unsigned int L2_SIZE);

int test(){

    unsigned int size = 1024;
	unsigned int assoc = 2;
	unsigned int block_size = 16;

	float access_time = 0;
	float energy = 0;
	float area = 0;

	int cacti = get_cacti_results(size, block_size, assoc, &access_time, &energy, &area);

	if(cacti > 0){
		cout<<"Cacti Failed"<<endl;
	}

	cout<<"Access Time: "<<access_time<<" Energy: "<<energy<<" area "<<area<<endl;

	return 0;

}
// 1110 -> e
// 1111 -> f


int main(int argc, char* argv[]){

    L1_SIZE = static_cast<unsigned int>(strtoul(argv[1], 0, 10));
	L1_ASSOC = static_cast<unsigned int>(strtoul(argv[2], 0, 10));
    L1_BLOCKSIZE = static_cast<unsigned int>(strtoul(argv[3], 0, 10));
	VC_NUM_BLOCKS = static_cast<unsigned int>(strtoul(argv[4], 0, 10));
    L2_SIZE = static_cast<unsigned int>(strtoul(argv[5], 0, 10));
    L2_ASSOC = static_cast<unsigned int>(strtoul(argv[6], 0, 10));
    tracefile = argv[7];

    cout << "===== Simulator configuration ====="<<endl;
	cout << "  L1_SIZE:\t\t" << L1_SIZE<<endl;
    cout << "  L1_ASSOC:\t\t" << L1_ASSOC<<endl;	
    cout << "  L1_BLOCKSIZE:\t\t" << L1_BLOCKSIZE<<endl;
	cout << "  VC_NUM_BLOCKS:\t" << VC_NUM_BLOCKS<<endl;
    cout << "  L2_SIZE:\t\t" << L2_SIZE<<endl;
    cout << "  L2_ASSOC:\t\t" << L2_ASSOC<<endl;
    cout << "  trace_file:\t\t" << tracefile << endl;
	cout<<endl;



	if(VC_NUM_BLOCKS == 0){
		CACHEMEMORY *L1 = new CACHEMEMORY(L1_SIZE, L1_BLOCKSIZE, L1_ASSOC);
		CACHEMEMORY *L2 = nullptr;

		if(L2_SIZE > 0){
			L2 = new CACHEMEMORY(L2_SIZE, L1_BLOCKSIZE, L2_ASSOC);
			// Append L1 to L2
			L1->append(L2);
		}

		// Open the trace file
		//string trace_file_dir = "samples/traces/";
		string trace_file_path = tracefile;
		ifstream file(trace_file_path);
		if (!file.is_open()) {
			cerr << "Unable to open file!" << endl;
			return 1;
		}

		string line;
		while (getline(file, line)) {
			// Check if the line starts with 'r' or 'w'
			if (line[0] == 'r' || line[0] == 'w') {
				char type = line[0];
				string hexValue = line.substr(1); // Extract the substring after 'r' or 'w'

				// Convert the hexadecimal string to an integer (optional)
				unsigned int addr;
				stringstream ss;
				ss << hex << hexValue;
				ss >> addr;

				if(type == 'r'){
					L1->read_request(addr);
				}else if(type == 'w'){
					L1->write_request(addr);
				}else{
					cerr<<"Invalid Request Type"<<endl;
				}
			}
		}

		// Close the file
		file.close();
		L1->sort_each_set();
		cout<<"===== L1 contents ====="<<endl;
		L1->show();
		if(L2_SIZE > 0){
			L2->sort_each_set();
			cout<<"===== L2 contents ====="<<endl;
			L2->show();
		}

		print_stats(L1, L2, L1_BLOCKSIZE, L1_SIZE, L1_ASSOC, L2_ASSOC, L2_SIZE);
		
	}else{
		L1VC *L1 = new L1VC(L1_SIZE, L1_BLOCKSIZE, L1_ASSOC, VC_NUM_BLOCKS);
		CACHEMEMORY *L2 = nullptr;

		if(L2_SIZE > 0){
			L2 = new CACHEMEMORY(L2_SIZE, L1_BLOCKSIZE, L2_ASSOC);
			// Append L1 to L2
			L1->append(L2);
		}

		// Open the trace file
		//string trace_file_dir = "samples/traces/";
		string trace_file_path = tracefile;
		ifstream file(trace_file_path);
		if (!file.is_open()) {
			cerr << "Unable to open file!" << endl;
			return 1;
		}

		string line;
		while (getline(file, line)) {
			// Check if the line starts with 'r' or 'w'
			if (line[0] == 'r' || line[0] == 'w') {
				char type = line[0];
				string hexValue = line.substr(1); // Extract the substring after 'r' or 'w'

				// Convert the hexadecimal string to an integer (optional)
				unsigned int addr;
				stringstream ss;
				ss << hex << hexValue;
				ss >> addr;

				if(type == 'r'){
					L1->read_request(addr);
				}else if(type == 'w'){
					L1->write_request(addr);
				}else{
					cerr<<"Invalid Request Type"<<endl;
				}
			}
		}

		// Close the file
		file.close();
		L1->sort_each_set();
		cout<<"===== L1 contents ====="<<endl;
		L1->show();
		cout<<endl;
		if(L2_SIZE > 0){
			L2->sort_each_set();
			cout<<"===== L2 contents ====="<<endl;
			L2->show();
		}

		print_stats(L1, L2, L1_BLOCKSIZE, L1_SIZE, L1_ASSOC, VC_NUM_BLOCKS, L2_ASSOC, L2_SIZE);

	}





	return 0;
}


void print_stats(CACHEMEMORY* L1, CACHEMEMORY* L2, unsigned int L1_BLOCKSIZE, unsigned int L1_SIZE, unsigned int L1_ASSOC, unsigned int L2_ASSOC, unsigned int L2_SIZE){

	int l1_reads = L1->reads;
	int l1_read_misses = L1->read_misses;
	int l1_writes = L1->writes;
	int l1_write_misses = L1->write_misses;
	int l1_write_backs = L1->write_backs;
	int l1_swap_requests = L1->swap_requests;
	int l1_swaps = L1->swaps;
	float l1_miss_rate = L1->get_miss_rate();

	cout<<"===== Simulation results (raw) ====="<<endl;
	cout<<"  a. number of L1 reads:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_reads<<endl;
	cout<<"  b. number of L1 read misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_read_misses<<endl;
	cout<<"  c. number of L1 writes:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_writes<<endl;
	cout<<"  d. number of L1 write misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_write_misses<<endl;

	cout<<"  e. number of swap requests:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_swap_requests<<endl;
	cout<<"  f. swap request rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<L1->get_swap_request_rate()<<endl;
	cout<<"  g. number of swaps:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_swaps<<endl;
	
	cout<<"  h. combined L1+VC miss rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l1_miss_rate<<endl;
	cout<<"  i. number writebacks from L1/VC:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_write_backs<<endl;
	
	int l2_reads = (L2 == nullptr) ? 0 : L2->reads;
	int l2_read_misses = (L2 == nullptr) ? 0 : L2->read_misses;
	int l2_writes = (L2 == nullptr) ? 0 : L2->writes;
	int l2_write_misses = (L2 == nullptr) ? 0 : L2->write_misses;
	int l2_write_backs = (L2 == nullptr) ? 0 : L2->write_backs;
	float l2_miss_rate = (L2 == nullptr) ? 0 : static_cast<float>(l2_read_misses)/(l2_reads);

	cout<<"  j. number of L2 reads:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_reads<<endl;
	cout<<"  k. number of L2 read misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_read_misses<<endl;
	cout<<"  l. number of L2 writes:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_writes<<endl;
	cout<<"  m. number of L2 write misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_write_misses<<endl;
	cout<<"  n. L2 miss rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l2_miss_rate<<endl;
	cout<<"  o. number of writebacks from L2:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_write_backs<<endl;


	int total_mem_traffic = 0;
	
	if(L2_SIZE > 0){
		total_mem_traffic = l2_read_misses + l2_write_misses + l2_write_backs;
	}else{
		total_mem_traffic = l1_read_misses + l1_write_misses - l1_swaps + l1_write_backs;
	}

	cout<<"  p. total memory traffic:"<<"\t"<<"\t"<<"\t"<<"\t"<<total_mem_traffic<<endl;

	cout<<endl;

	cout<<"===== Simulation results (performance) ====="<<endl;


	float l1_access_time = 0;
	float l1_energy = 0;
	float l1_area = 0;
	float l1_avg_access_time = 0;

	float vc_energy = 0;
	float vc_area = 0;
	
	float edp = 0;
	float area = 0;
	
	int cacti = get_cacti_results(L1_SIZE, L1_BLOCKSIZE, L1_ASSOC, 
									&l1_access_time, &l1_energy, &l1_area);


	
	//cout<<"L1 Access Time: "<<l1_access_time<<"L1 Energy: "<<l1_energy<<"L1 Area: "<<l1_area<<endl;

	float l2_access_time = 0;
	float l2_energy = 0;
	float l2_area = 0;
	float l2_avg_access_time = 20;

	float main_mem_access_time = 20;
	float main_mem_energy = 0.05;
	
	if(L2_SIZE > 0){
		cacti = get_cacti_results(L2_SIZE, L1_BLOCKSIZE, L2_ASSOC, &l2_access_time, &l2_energy, &l2_area);
		l2_avg_access_time = l2_access_time + l2_miss_rate*main_mem_access_time;
		//cout<<"L2 Access Time: "<<l2_access_time<<" Energy: "<<l2_energy<<" area: "<<l2_area<<endl;
	}

	l1_avg_access_time = l1_access_time + l1_miss_rate*l2_avg_access_time;

	if(L2_SIZE > 0){
		float total_energy = (l1_reads + l1_writes)*l1_energy + 
			  (l1_read_misses + l1_write_misses)*l1_energy +
			  2*l1_swap_requests*vc_energy +
			  (l2_reads + l2_writes)*l2_energy + 
			  (l2_read_misses + l2_write_misses)*l2_energy +
			  (l2_read_misses + l2_write_misses)*main_mem_energy +
			  l2_write_backs * main_mem_energy;

		edp = total_energy * l1_avg_access_time * (l1_reads + l1_writes);

	}else{
		float total_energy = (l1_reads + l1_writes)*l1_energy + 
			  (l1_read_misses + l1_write_misses)*l1_energy +
			  2*l1_swap_requests*vc_energy +
			  (l1_read_misses + l1_write_misses - l1_swaps)*main_mem_energy + 
			  (l1_write_backs)*main_mem_energy;
	
		edp = total_energy * l1_avg_access_time * (l1_reads + l1_writes);
	}

	area = l1_area + l2_area + vc_area;

	cout<<"1. average access time:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l1_avg_access_time<<endl;
	cout<<"2. energy-delay product:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<edp<<endl;
	cout<<"3. total area:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<area<<endl;

}


void print_stats(L1VC* L1, CACHEMEMORY* L2, unsigned int L1_BLOCKSIZE, unsigned int L1_SIZE, unsigned int L1_ASSOC, unsigned int VC_NUM_BLOCKS, unsigned int L2_ASSOC, unsigned int L2_SIZE){

	int l1_reads = L1->reads;
	int l1_read_misses = L1->read_misses;
	int l1_writes = L1->writes;
	int l1_write_misses = L1->write_misses;
	int l1_write_backs = L1->write_backs;
	int l1_swap_requests = L1->swap_requests;
	int l1_swaps = L1->swaps;
	float l1_miss_rate = L1->get_miss_rate();

	cout<<"===== Simulation results (raw) ====="<<endl;
	cout<<"  a. number of L1 reads:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_reads<<endl;
	cout<<"  b. number of L1 read misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_read_misses<<endl;
	cout<<"  c. number of L1 writes:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_writes<<endl;
	cout<<"  d. number of L1 write misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_write_misses<<endl;

	cout<<"  e. number of swap requests:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_swap_requests<<endl;
	cout<<"  f. swap request rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<L1->get_swap_request_rate()<<endl;
	cout<<"  g. number of swaps:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_swaps<<endl;
	
	cout<<"  h. combined L1+VC miss rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l1_miss_rate<<endl;
	cout<<"  i. number writebacks from L1/VC:"<<"\t"<<"\t"<<"\t"<<"\t"<<l1_write_backs<<endl;
	
	int l2_reads = (L2 == nullptr) ? 0 : L2->reads;
	int l2_read_misses = (L2 == nullptr) ? 0 : L2->read_misses;
	int l2_writes = (L2 == nullptr) ? 0 : L2->writes;
	int l2_write_misses = (L2 == nullptr) ? 0 : L2->write_misses;
	int l2_write_backs = (L2 == nullptr) ? 0 : L2->write_backs;
	float l2_miss_rate = (L2 == nullptr) ? 0 : static_cast<float>(l2_read_misses)/(l2_reads);

	cout<<"  j. number of L2 reads:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_reads<<endl;
	cout<<"  k. number of L2 read misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_read_misses<<endl;
	cout<<"  l. number of L2 writes:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_writes<<endl;
	cout<<"  m. number of L2 write misses:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_write_misses<<endl;
	cout<<"  n. L2 miss rate:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l2_miss_rate<<endl;
	cout<<"  o. number of writebacks from L2:"<<"\t"<<"\t"<<"\t"<<"\t"<<l2_write_backs<<endl;


	int total_mem_traffic = 0;
	
	if(L2_SIZE > 0){
		total_mem_traffic = l2_read_misses + l2_write_misses + l2_write_backs;
	}else{
		total_mem_traffic = l1_read_misses + l1_write_misses - l1_swaps + l1_write_backs;
	}

	cout<<"  p. total memory traffic:"<<"\t"<<"\t"<<"\t"<<"\t"<<total_mem_traffic<<endl;

	cout<<endl;

	cout<<"===== Simulation results (performance) ====="<<endl;


	float l1_access_time = 0;
	float l1_energy = 0;
	float l1_area = 0;
	float l1_avg_access_time = 0;

	float vc_access_time = 0;
	float vc_energy = 0;
	float vc_area = 0;
	
	float edp = 0;
	float area = 0;
	
	int cacti = get_cacti_results(L1_SIZE, L1_BLOCKSIZE, L1_ASSOC, 
									&l1_access_time, &l1_energy, &l1_area);


	
	//cout<<"L1 Access Time: "<<l1_access_time<<"L1 Energy: "<<l1_energy<<"L1 Area: "<<l1_area<<endl;

	if(VC_NUM_BLOCKS > 0){
		unsigned int VC_SIZE = VC_NUM_BLOCKS * L1_BLOCKSIZE;
		unsigned int VC_ASSOC = VC_NUM_BLOCKS;
		cacti = get_cacti_results(VC_SIZE, L1_BLOCKSIZE, VC_ASSOC, &vc_access_time, &vc_energy, &vc_area);
	}

	float l2_access_time = 0;
	float l2_energy = 0;
	float l2_area = 0;
	float l2_avg_access_time = 20;

	float main_mem_access_time = 20;
	float main_mem_energy = 0.05;
	
	if(L2_SIZE > 0){
		cacti = get_cacti_results(L2_SIZE, L1_BLOCKSIZE, L2_ASSOC, &l2_access_time, &l2_energy, &l2_area);
		l2_avg_access_time = l2_access_time + l2_miss_rate*main_mem_access_time;
		//cout<<"L2 Access Time: "<<l2_access_time<<" Energy: "<<l2_energy<<" area: "<<l2_area<<endl;
	}

	l1_avg_access_time = l1_access_time + l1_miss_rate*l2_avg_access_time;

	if(L2_SIZE > 0){
		float total_energy = (l1_reads + l1_writes)*l1_energy + 
			  (l1_read_misses + l1_write_misses)*l1_energy +
			  2*l1_swap_requests*vc_energy + 
			  (l2_reads + l2_writes)*l2_energy + 
			  (l2_read_misses + l2_write_misses)*l2_energy +
			  (l2_read_misses + l2_write_misses)*main_mem_energy +
			  l2_write_backs * main_mem_energy;

		//cout<<"Total Energy"<<total_energy<<endl;
		edp = total_energy * l1_avg_access_time * (l1_reads + l1_writes);

	}else{
		float total_energy = (l1_reads + l1_writes)*l1_energy + 
			  (l1_read_misses + l1_write_misses)*l1_energy +
			  2*l1_swap_requests*vc_energy +
			  (l1_read_misses + l1_write_misses - l1_swaps)*main_mem_energy + 
			  (l1_write_backs)*main_mem_energy;
	
		edp = total_energy * l1_avg_access_time * (l1_reads + l1_writes);
	}

	area = l1_area + l2_area + vc_area;

	cout<<"1. average access time:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<l1_avg_access_time<<endl;
	cout<<"2. energy-delay product:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<edp<<endl;
	cout<<"3. total area:"<<"\t"<<"\t"<<"\t"<<"\t"<<fixed<<setprecision(3)<<area<<endl;

}