#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;

struct CACHEBLOCK
{
    /* data */
    unsigned int tag;
    unsigned int block_offset;
    bool is_valid;
    bool is_dirty;
    int counter; // LRU Counter

};

bool counter_comparator(const CACHEBLOCK& a, const CACHEBLOCK& b)
{
    // CACHE BLOCK with least LRU comes first
    return a.counter < b.counter;
}

class CACHEMEMORY
{
private:
    /* data */
    // Map index -> Set of CACHEBLOCKS
    vector<vector<CACHEBLOCK>> cache;
    CACHEMEMORY* next_mem;

    unsigned int index_mask;
    unsigned int tag_mask;
    unsigned int block_offset_mask;

    int tag_bits_length;
    int index_bits_length;
    int block_offset_bits_length;

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
    CACHEMEMORY(/* args */);
    ~CACHEMEMORY();
    CACHEMEMORY(int size, int blocksize, int assoc);
    
    void append(CACHEMEMORY* memory);
    void sort_each_set();
    void set_tag_mask(int tag_length);
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

    float get_swap_request_rate();

    float get_miss_rate();

};

CACHEMEMORY::CACHEMEMORY(){}

CACHEMEMORY::~CACHEMEMORY(){}

CACHEMEMORY::CACHEMEMORY(int size, int blocksize, int assoc){
    this->next_mem = nullptr;
    int num_sets = size/(blocksize * assoc);
    this->cache.resize(num_sets);

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

void CACHEMEMORY::append(CACHEMEMORY* memory){
    this->next_mem = memory;
}

float CACHEMEMORY::get_swap_request_rate(){
    float srr = (this->swap_requests)/(this->reads + this->writes);
    return srr;
}

float CACHEMEMORY::get_miss_rate(){
    float misses = this->read_misses + this->write_misses - this->swaps;
    float requests = this->reads + this->writes;
    
    float mr = misses/requests;

    //cout<<"Misses: "<<misses<<" Requests: "<<requests<<" Miss rate: "<<mr<<endl;

    return mr;
}

void CACHEMEMORY::sort_each_set() {
    // int s = this->cache.size();
    // for(int i = 0; i < s; i++){
    //     int n = this->cache[i].size();
    //     for (int j = 0; j < n - 1; ++j) {
    //         for (int k = 0; k < n - j - 1; ++k) {
    //             if (this->cache[i][k].counter > this->cache[i][k + 1].counter) {
    //                 swap(this->cache[i][k], this->cache[i][k + 1]);
    //             }
    //         }
    //     }
    // }

    for (auto &vec : this->cache) {
        std::sort(vec.begin(), vec.end(), counter_comparator);
    }

}

void CACHEMEMORY::show(){
   
    for(int i = 0; i < this->cache.size(); i++){
        cout<<dec<<"\tset"<<"\t"<<i<<":\t";
        for(CACHEBLOCK block : this->cache[i]){
            cout<<hex<<block.tag<<" ";
            //cout<<"Counter: "<<block.counter<<" ";
            //cout<<"Dirty Bit: "<<block.is_dirty<<" ";
            char dirty_char = block.is_dirty ? 'D' : ' ';
            cout<<dec<<dirty_char<<"\t";
        }
        cout<<endl;
    }

    cout<<endl;

}

void CACHEMEMORY::set_masks(int blocksize, int num_sets){

    int block_offset_length = log2(blocksize);
    int index_length = log2(num_sets);
    int tag_length = ADDR_LENGTH - index_length - block_offset_length;

    this->tag_bits_length = tag_length;
    this->index_bits_length = index_length;
    this->block_offset_bits_length = block_offset_length;

    set_block_offset_mask(block_offset_length);
    set_index_mask(index_length, block_offset_length);
    set_tag_mask(tag_length);

}

void CACHEMEMORY::set_block_offset_mask(int block_offset_length){
    this->block_offset_mask = get_mask(0, block_offset_length-1);
}

void CACHEMEMORY::set_index_mask(int index_length, int block_offset_length){
    this->index_mask = get_mask(block_offset_length, block_offset_length+index_length-1);
}

void CACHEMEMORY::set_tag_mask(int tag_length){
    this->tag_mask = get_mask(ADDR_LENGTH-tag_length,ADDR_LENGTH);
}

unsigned int CACHEMEMORY::get_mask(int right, int left){
    return ((1 << (left - right + 1)) - 1) << right;
}

int test_mask(int right, int left){
    return ((1 << (left - right + 1)) - 1) << right;
}

// int main(){

//     int num = 0xeff5;
//     int mask = test_mask(1,2);
//     int res = (num & mask) >> 1;
//     cout<<"Mask is: "<<mask<<endl;
//     cout<<"Result is: "<<res<<endl;
//     return 0;
// }