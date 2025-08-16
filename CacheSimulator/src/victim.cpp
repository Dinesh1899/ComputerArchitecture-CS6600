#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <vector>
#include <algorithm>
#include "cache_mem.cpp"
using namespace std;

class L1VC
{
private:
    /* data */
    // Map index -> Set of CACHEBLOCKS
    vector<vector<CACHEBLOCK>> cache;
    vector<CACHEBLOCK> vc;
    CACHEMEMORY* next_mem;

    unsigned int index_mask;
    unsigned int tag_mask;
    unsigned int block_offset_mask;

    int tag_bits_length;
    int index_bits_length;
    int block_offset_bits_length;
    int block_addr_mask;


public:

    /////////////////////////////STATS////////////////////////////////
    int reads = 0;
    int read_misses = 0;
    int writes = 0;
    int write_misses = 0;
    int swap_requests = 0;
    int swaps = 0;
    int write_backs = 0;

    static const int ADDR_LENGTH = 32;
    L1VC(/* args */);
    ~L1VC();
    L1VC(int size, int blocksize, int assoc, int vc_blocks);
    
    void append(CACHEMEMORY* memory);
    void sort_each_set();
    void set_tag_mask(int tag_length);
    void set_block_addr_mask(int tag_length);
    void set_index_mask(int index_length, int block_offset_length);
    void set_block_offset_mask(int block_offset_length);
    void set_masks(int blocksize, int num_sets);
    unsigned int get_mask(int right, int left);
    unsigned int get_masked_data(unsigned int addr, unsigned int mask, int right);
    CACHEBLOCK get_read_block(unsigned int addr, bool is_write_request);
    void insert_block_and_update_lru(int row, int col, CACHEBLOCK &new_block, bool is_dirty);
    void insert_block_replace_invalid_block_and_update_lru(int row, CACHEBLOCK &new_block, bool is_dirty);
    int search_block_by_tag(int row, int tag);
    void show();

    bool is_cache_full(int row);

    CACHEBLOCK get_lru_block(int row);

    bool read_request(unsigned int addr);

    bool write_request(unsigned int addr);

    int search_block_in_vc(unsigned int block_addr);

    int evict_lru_from_vc();

    void insert_block_in_vc(CACHEBLOCK block, int pos);

    float get_swap_request_rate();

    float get_miss_rate();

};

L1VC::L1VC(){}

L1VC::~L1VC(){}

L1VC::L1VC(int size, int blocksize, int assoc, int vc_blocks){
    this->next_mem = nullptr;
    int num_sets = size/(blocksize * assoc);
    this->cache.resize(num_sets);
    this->vc.resize(vc_blocks);

    for(int i=0;i<this->cache.size();i++){
        this->cache[i].resize(assoc);
    }

    set_masks(blocksize, num_sets);
    
    for(int i=0;i<this->cache.size();i++){
        for(int j=0;j<this->cache[i].size();j++){
            this->cache[i][j].is_valid = false;
        }
    } 
}

void L1VC::append(CACHEMEMORY* memory){
    this->next_mem = memory;
}

float L1VC::get_swap_request_rate(){
    float srr = (static_cast<float>(this->swap_requests))/(this->reads + this->writes);
    return srr;
}

float L1VC::get_miss_rate(){
    float misses = this->read_misses + this->write_misses - this->swaps;
    float requests = this->reads + this->writes;
    
    float mr = misses/requests;

    //cout<<"Misses: "<<misses<<" Requests: "<<requests<<" Miss rate: "<<mr<<endl;

    return mr;
}


void L1VC::sort_each_set() {
    for (auto &vec : this->cache) {
        std::sort(vec.begin(), vec.end(), counter_comparator);
    }

    std::sort(this->vc.begin(), this->vc.end(), counter_comparator);
}

void L1VC::show(){
   
    for(int i = 0; i < this->cache.size(); i++){
        cout<<dec<<"\tset "<<i<<":\t";
        for(CACHEBLOCK block : this->cache[i]){
            cout<<hex<<block.tag<<" ";
            //cout<<"Counter: "<<block.counter<<" ";
            //cout<<"Valid Bit: "<<block.is_valid<<" ";
            char dirty_char = block.is_dirty ? 'D' : ' ';
            cout<<dec<<dirty_char<<"\t";
        }
        cout<<endl;
    }

    cout<<endl;

    cout<<"===== VC contents ====="<<endl;
    cout<<dec<<"\tset 0:\t";
    for(CACHEBLOCK block : this->vc){
        cout<<hex<<block.tag<<" ";
        //cout<<"Counter: "<<block.counter<<" ";
        //cout<<"Valid Bit: "<<block.is_valid<<" ";
        char dirty_char = block.is_dirty ? 'D' : ' ';
        cout<<dec<<dirty_char<<"\t";
    }    

    cout<<endl;

}

void L1VC::set_masks(int blocksize, int num_sets){

    int block_offset_length = log2(blocksize);
    int index_length = log2(num_sets);
    int tag_length = ADDR_LENGTH - index_length - block_offset_length;
    int block_addr_length = ADDR_LENGTH - block_offset_length;

    this->tag_bits_length = tag_length;
    this->index_bits_length = index_length;
    this->block_offset_bits_length = block_offset_length;
    this->block_addr_mask = block_addr_length;

    set_block_offset_mask(block_offset_length);
    set_index_mask(index_length, block_offset_length);
    set_tag_mask(tag_length);
    set_block_addr_mask(block_addr_length);

}

void L1VC::set_block_offset_mask(int block_offset_length){
    this->block_offset_mask = get_mask(0, block_offset_length-1);
}

void L1VC::set_index_mask(int index_length, int block_offset_length){
    this->index_mask = get_mask(block_offset_length, block_offset_length+index_length-1);
}

void L1VC::set_tag_mask(int tag_length){
    this->tag_mask = get_mask(ADDR_LENGTH-tag_length,ADDR_LENGTH);
}

void L1VC::set_block_addr_mask(int block_addr_length){
    this->block_addr_mask = get_mask(ADDR_LENGTH-block_addr_length, ADDR_LENGTH);
}

unsigned int L1VC::get_mask(int right, int left){
    return ((1 << (left - right + 1)) - 1) << right;
}
