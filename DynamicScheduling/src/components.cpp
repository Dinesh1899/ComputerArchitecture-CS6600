#include "components.h"

void Instruction::update_exec_time(int n_cycles) {
    this->exec_time = n_cycles;
}


bool Instruction::is_exec_done(int cycle_count) {
    bool is_done = false;
    if(this->current_state == state::EX) {

        switch(opcode) {
            case 0:
                if(cycle_count-EX_START == 1) { is_done = true; }
                break;
            case 1:
                if(cycle_count-EX_START == 2) { is_done = true; }
                break;
            case 2:
                if(cycle_count-EX_START == 10) { is_done = true; }
                break;
        }
    }
    
    return is_done; 
}

