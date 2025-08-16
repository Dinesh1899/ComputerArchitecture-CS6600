#include "victim.cpp"

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <math.h>

using namespace std;

unsigned int L1VC::get_masked_data(unsigned int addr, unsigned int mask, int start){
    return (addr & mask) >> start;
}

CACHEBLOCK L1VC::get_read_block(unsigned int addr, bool is_write_request){
    CACHEBLOCK cb;
    int tag_lsb = this->block_offset_bits_length + this->index_bits_length;
    cb.tag = this->get_masked_data(addr, this->tag_mask, tag_lsb);
    cb.block_offset = addr;
    cb.counter = 0;
    cb.is_dirty = is_write_request;
    cb.is_valid = true;

    return cb;
}


void L1VC::insert_block_and_update_lru(int row, int hit, CACHEBLOCK& new_block, bool update_dirty_bit){

    int hit_block_counter = this->cache[row][hit].counter;

    for(int j=0;j<this->cache[row].size();j++){
        if(this->cache[row][j].is_valid){
            if(j == hit){
                this->cache[row][j].block_offset = new_block.block_offset;
                this->cache[row][j].tag = new_block.tag;
                this->cache[row][j].is_valid = true;
                if(update_dirty_bit)
                    this->cache[row][j].is_dirty = new_block.is_dirty;
                this->cache[row][j].counter = 0;
            }else{
                //Update the block with the new block details
                int val = this->cache[row][j].counter;
                if(val < hit_block_counter)
                    this->cache[row][j].counter++;
            }
        }
    }

}

void L1VC::insert_block_replace_invalid_block_and_update_lru(int row, CACHEBLOCK& new_block, bool update_dirty_bit){

    int idx = 0;

    for(CACHEBLOCK block : this->cache[row]){
        if(!block.is_valid){
            break;
        }
        idx++;
    }

    for(int j=0;j<this->cache[row].size();j++){
        if(this->cache[row][j].is_valid && j != idx){
            //Update the block with the new block details
            this->cache[row][j].counter++;
        }
    }

    //Update the block with the new block details
    this->cache[row][idx].block_offset = new_block.block_offset;
    this->cache[row][idx].tag = new_block.tag;
    this->cache[row][idx].is_valid = true;
    if(update_dirty_bit)
        this->cache[row][idx].is_dirty = new_block.is_dirty;
    this->cache[row][idx].counter = 0;

}

int L1VC::search_block_by_tag(int row, int tag){
    int hit_block_idx = -1;
    int idx = 0;

    for(CACHEBLOCK block : this->cache.at(row)){
        if(block.is_valid && block.tag == tag){
            return idx;
        }
        idx++;
    }
    return -1;
}

bool L1VC::is_cache_full(int row){
    int idx = 0;

    for(CACHEBLOCK block : this->cache[row]){
        if(block.is_valid){
            idx++;
        }
    }
    return idx == this->cache[row].size();
}

CACHEBLOCK L1VC::get_lru_block(int row){
    int idx = 0;
    int assoc = this->cache[row].size() - 1;
    CACHEBLOCK evict;
    for(CACHEBLOCK block : this->cache[row]){
        if(block.is_valid && block.counter == assoc){
            evict = block;
            break;
        }
        idx++;
    }

    this->cache[row][idx].is_valid = false;

    return evict;
}

int L1VC::search_block_in_vc(unsigned int block_addr){
    int n = this->vc.size();
    int idx = -1;

    for(int i=0;i<n;i++){
        if(this->vc[i].is_valid && this->vc[i].tag == block_addr){
            idx = i;
            break;
        }
    }

    return idx;
}


int L1VC::evict_lru_from_vc(){

    //Check if vc is full
    int n = this->vc.size();
    int idx = -1;

    for(int i=0;i<n;i++){
        if(!this->vc[i].is_valid){
            idx = i;
            break;
        }
    }

    if(idx == -1){
        //vc is full
        int lru_idx = -1;
        int max_val = -1;
        for(int i=0;i<n;i++){
            if(this->vc[i].counter > max_val){
                max_val = this->vc[i].counter;
                lru_idx = i;
            }
        }

        //evict lru block --> issue write back to L2 if dirty
        if(this->vc[lru_idx].is_dirty){
            unsigned int addr = this->vc[lru_idx].block_offset;
            if(next_mem != nullptr){
                this->write_backs++;
                this->next_mem->write_request(addr);
            }
        }
        //invalidate lru block
        this->vc[lru_idx].is_valid = false;
        return lru_idx;
    }else{
        this->vc[idx].is_valid = false;
        return idx;
    }

}

void L1VC::insert_block_in_vc(CACHEBLOCK new_block, int pos){

    // Increment LRU values of valid blocks
    for(int i=0;i<this->vc.size();i++){
        if(this->vc[i].is_valid && i != pos){
            //Update the block with the new block details
            this->vc[i].counter++;
        }
    }

    //Update the block with the new block details
    this->vc[pos].block_offset = new_block.block_offset;
    this->vc[pos].tag = this->get_masked_data(new_block.block_offset, this->block_addr_mask, this->block_offset_bits_length);
    this->vc[pos].is_valid = true;
    this->vc[pos].is_dirty = new_block.is_dirty;        
    this->vc[pos].counter = 0;
}

bool L1VC::read_request(unsigned int addr){
    // Get the cache line from addr and index mask
    this->reads++;
    int idx = get_masked_data(addr, this->index_mask, this->block_offset_bits_length);
    int block_addr = get_masked_data(addr, this->block_addr_mask, this->block_offset_bits_length);
    // //Debug
    // if(idx == 1)
    //     cout<<hex<<"Addr with index 4: "<<"r "<<addr<<endl;

    // Search block from the cacheline
    CACHEBLOCK requested_block = this->get_read_block(addr, false);
    int block_idx = search_block_by_tag(idx, requested_block.tag);

    if(block_idx == -1){
        // Read Miss
        // fetch from next level 
        // insert new block
        // update lru
        // return false
        this->read_misses++;
        if(is_cache_full(idx)){
            // line is full --> evict block
            this->swap_requests++;
            CACHEBLOCK evicted_block = get_lru_block(idx);
            
            //search in vc
            int vc_idx = search_block_in_vc(block_addr);
            //cout<<"Block Addr: "<<hex<<block_addr<<" VC Index is: "<<dec<<vc_idx<<endl;
            if(vc_idx == -1){
                // vc miss
                int pos = evict_lru_from_vc();
                insert_block_in_vc(evicted_block, pos);

                // fetch from next level
                if(this->next_mem != nullptr){
                    this->next_mem->read_request(addr);
                }

                // insert new block
                // update lru
                insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
                return false; 
            }else{
                this->swaps++;
                //vc hit
                //swap vc block with L1
                requested_block.is_dirty = this->vc[vc_idx].is_dirty;

                //update all details except for lru
                this->vc[vc_idx].tag = get_masked_data(evicted_block.block_offset, block_addr_mask, block_offset_bits_length);    
                this->vc[vc_idx].block_offset = evicted_block.block_offset;     
                this->vc[vc_idx].is_dirty = evicted_block.is_dirty;    
                this->vc[vc_idx].is_valid = true;    

                //update LRU in L1
                insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
                return true;
            }


        }else{
            // fetch from next level
            if(this->next_mem != nullptr){
                this->next_mem->read_request(addr);
            }

            // insert new block
            // update lru
            insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
            return false;             
        }
    }else{
        // Read Hit --> return true and update lru counter
        insert_block_and_update_lru(idx, block_idx, requested_block, false);
        return true;
    }
    
}

bool L1VC::write_request(unsigned int addr){
    this->writes++;
    // Get the cache line from addr and index mask
    int idx = get_masked_data(addr, this->index_mask, this->block_offset_bits_length);
    int block_addr = get_masked_data(addr, this->block_addr_mask, this->block_offset_bits_length);
    //Debug
    // if(idx == 1)
    //     cout<<hex<<"Addr with index 1: "<<"r "<<addr<<endl;

    // Search block from the cacheline
    CACHEBLOCK requested_block = this->get_read_block(addr, true);
    int block_idx = search_block_by_tag(idx, requested_block.tag);

    if(block_idx == -1){
        // Read Miss
        // fetch from next level 
        // insert new block
        // update lru
        // return false
        this->write_misses++;
        if(is_cache_full(idx)){
            this->swap_requests++;
            // line is full --> evict block
            CACHEBLOCK evicted_block = get_lru_block(idx);
            
            //search in vc
            int vc_idx = search_block_in_vc(block_addr);
            
            if(vc_idx == -1){
                // vc miss
                int pos = evict_lru_from_vc();
                insert_block_in_vc(evicted_block, pos);

                // fetch from next level
                if(this->next_mem != nullptr){
                    this->next_mem->read_request(addr);
                }

                // insert new block
                // update lru
                insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
                return false; 
            }else{
                this->swaps++;
                //vc hit
                //swap vc block with L1
                //Write request block is anyways dirty
                //requested_block.is_dirty = this->vc[vc_idx].is_dirty;

                //update all details except for lru
                this->vc[vc_idx].tag = get_masked_data(evicted_block.block_offset, block_addr_mask, block_offset_bits_length);    
                this->vc[vc_idx].block_offset = evicted_block.block_offset;     
                this->vc[vc_idx].is_dirty = evicted_block.is_dirty;    
                this->vc[vc_idx].is_valid = true;    

                //update LRU in L1
                insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
                return true;
            }
        }else{
            // fetch from next level
            if(this->next_mem != nullptr){
                this->next_mem->read_request(addr);
            }

            // insert new block
            // update lru
            insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
            return false;             
        }
    }else{
        // Read Hit --> return true and update lru counter
        insert_block_and_update_lru(idx, block_idx, requested_block, true);
        return true;
    }
    
}


unsigned int CACHEMEMORY::get_masked_data(unsigned int addr, unsigned int mask, int start){
    return (addr & mask) >> start;
}

CACHEBLOCK CACHEMEMORY::get_read_block(unsigned int addr, bool is_write_request){
    CACHEBLOCK cb;
    int tag_lsb = this->block_offset_bits_length + this->index_bits_length;
    cb.tag = this->get_masked_data(addr, this->tag_mask, tag_lsb);
    cb.block_offset = addr;
    cb.counter = 0;
    cb.is_dirty = is_write_request;
    cb.is_valid = true;

    return cb;
}


void CACHEMEMORY::insert_block_and_update_lru(int row, int hit, CACHEBLOCK& new_block, bool update_dirty_bit){

    int hit_block_counter = this->cache[row][hit].counter;

    for(int j=0;j<this->cache[row].size();j++){
        if(this->cache[row][j].is_valid){
            if(j == hit){
                this->cache[row][j].block_offset = new_block.block_offset;
                this->cache[row][j].tag = new_block.tag;
                this->cache[row][j].is_valid = true;
                if(update_dirty_bit)
                    this->cache[row][j].is_dirty = new_block.is_dirty;
                this->cache[row][j].counter = 0;
            }else{
                //Update the block with the new block details
                int val = this->cache[row][j].counter;
                if(val < hit_block_counter)
                    this->cache[row][j].counter++;
            }
        }
    }

}

void CACHEMEMORY::insert_block_replace_invalid_block_and_update_lru(int row, CACHEBLOCK& new_block, bool update_dirty_bit){

    int idx = 0;

    for(CACHEBLOCK block : this->cache[row]){
        if(!block.is_valid){
            break;
        }
        idx++;
    }

    for(int j=0;j<this->cache[row].size();j++){
        if(this->cache[row][j].is_valid && j != idx){
            //Update the block with the new block details
            this->cache[row][j].counter++;
        }
    }

    //Update the block with the new block details
    this->cache[row][idx].block_offset = new_block.block_offset;
    this->cache[row][idx].tag = new_block.tag;
    this->cache[row][idx].is_valid = true;
    if(update_dirty_bit)
        this->cache[row][idx].is_dirty = new_block.is_dirty;
    this->cache[row][idx].counter = 0;

}

int CACHEMEMORY::search_block_by_tag(int row, int tag){
    int hit_block_idx = -1;
    int idx = 0;

    for(CACHEBLOCK block : this->cache.at(row)){
        if(block.is_valid && block.tag == tag){
            return idx;
        }
        idx++;
    }
    return -1;
}

bool CACHEMEMORY::is_cache_full(int row){
    int idx = 0;

    for(CACHEBLOCK block : this->cache[row]){
        if(block.is_valid){
            idx++;
        }
    }
    return idx == this->cache[row].size();
}

CACHEBLOCK CACHEMEMORY::get_lru_block(int row){
    int idx = 0;
    int assoc = this->cache[row].size() - 1;
    CACHEBLOCK evict;
    for(CACHEBLOCK block : this->cache[row]){
        if(block.is_valid && block.counter == assoc){
            evict = block;
            break;
        }
        idx++;
    }

    this->cache[row][idx].is_valid = false;

    return evict;
}

bool CACHEMEMORY::read_request(unsigned int addr){
    // Get the cache line from addr and index mask
    this->reads++;
    int idx = get_masked_data(addr, this->index_mask, this->block_offset_bits_length);
    //Debug
    // if(idx == 1)
    //     cout<<hex<<"Addr with index 1: "<<"r "<<addr<<endl;

    // Search block from the cacheline
    CACHEBLOCK requested_block = this->get_read_block(addr, false);
    int block_idx = search_block_by_tag(idx, requested_block.tag);

    if(block_idx == -1){
        // Read Miss
        // fetch from next level 
        // insert new block
        // update lru
        // return false
        this->read_misses++;
        if(is_cache_full(idx)){
            // line is full --> evict block
            CACHEBLOCK evicted_block = get_lru_block(idx);

            if(evicted_block.is_dirty){
                this->write_backs++;
                if(this->next_mem != nullptr){ 
                    unsigned int write_back_addr = evicted_block.block_offset;
                    this->next_mem->write_request(write_back_addr);
                }
            }

        }
        // fetch from next level
        if(this->next_mem != nullptr){
            this->next_mem->read_request(addr);
        }

        // insert new block
        // update lru
        insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
        return false; 
    }else{
        // Read Hit --> return true and update lru counter
        insert_block_and_update_lru(idx, block_idx, requested_block, false);
        return true;
    }
    
}

bool CACHEMEMORY::write_request(unsigned int addr){
        // Get the cache line from addr and index mask
    this->writes++;
    int idx = get_masked_data(addr, this->index_mask,this->block_offset_bits_length);

    //Debug
    // if(idx == 1)
    //     cout<<"Addr with index 1: "<<"w "<<hex<<addr<<endl;

    // Search block from the cacheline
    CACHEBLOCK requested_block = this->get_read_block(addr, true);
    int block_idx = search_block_by_tag(idx, requested_block.tag);

    if(block_idx == -1){
        // Read Miss
        // fetch from next level 
        // insert new block
        // update lru
        // return false
        this->write_misses++;
        if(is_cache_full(idx)){
            // line is full --> evict block
            CACHEBLOCK evicted_block = get_lru_block(idx);

            if(evicted_block.is_dirty){
                this->write_backs++;
                if(this->next_mem != nullptr){ 
                    unsigned int write_back_addr = evicted_block.block_offset;
                    this->next_mem->write_request(write_back_addr);
                }
            }

        }
        // fetch from next level
        if(this->next_mem != nullptr){
            this->next_mem->read_request(addr);
        }

        // insert new block
        // update lru
        insert_block_replace_invalid_block_and_update_lru(idx, requested_block, true);
        return false; 
    }else{
        // Read Hit --> return true and update lru counter
        insert_block_and_update_lru(idx, block_idx, requested_block, true);
        return true;
    }
}
