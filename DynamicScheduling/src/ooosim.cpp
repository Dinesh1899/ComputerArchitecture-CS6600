#include <iostream>
#include <list>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <string.h>
#include "components.h"
#include "issue_handler.h"
#include <fstream>

using namespace std;

int S;
int N;         
string tracefile;   
ifstream trace;

Register *reg_file_list;
list<Instruction> rob;
list<Instruction> dispatch_list;
IssueHandler issue_handler;
list<Instruction> execute_list;

int instr_counter;
int cycle_count;
bool is_last_instruction;

bool tag_compare(Instruction instr1, Instruction instr2)
{
    return instr1.tag < instr2.tag;
}

bool advance_cycle();
void fetch();
void dispatch();
void issue();
void execute();
void retire();

int main(int argc, char *argv[]){

    N = strtoul(argv[1], 0, 10);
    S = strtoul(argv[2], 0, 10);
    tracefile = argv[3];

    reg_file_list = new Register[128];
    issue_handler = IssueHandler(S);
    instr_counter = 0;
    cycle_count = 0;
    is_last_instruction = false;

    trace.open(tracefile);
    if (!trace.is_open()){
        cout<<"Not able to open the file!!"<<endl;
        exit(0);
    }

    do{
        retire();
        execute();
        issue();
        dispatch();
        fetch();
    } while (advance_cycle());

    cout << "CONFIGURATION" << endl;
    cout << " superscalar bandwidth (N)      = " << dec << N << endl;
    cout << " dispatch queue size (2*N)      = " << dec << 2 * N << endl;
    cout << " schedule queue size (S)        = " << dec << S << endl;
    cout << "RESULTS" << endl;
    cout << " number of instructions = " << dec << instr_counter << endl;
    cout << " number of cycles       = " << dec << cycle_count << endl;
    cout << " IPC                    = " << fixed << setprecision(2) << (float)instr_counter / cycle_count << endl;

    return 0;
}

bool advance_cycle(){

    if (rob.empty() && is_last_instruction){
        return false;
    }else{
        cycle_count++;
        return true;
    }
}

void fetch(){
    int PC;
    Instruction instr;

    string line;
    int parse = 0;
    int element = 0;

    for(int count=0; count < N && dispatch_list.size() < 2 * N; count++){
        if (getline(trace, line)){
            string lines[6] = {"", "", "", "", "", ""};

            while (parse != line.length())
            {
                if (line[parse] != ' ')
                {
                    lines[element] += line[parse];
                }
                else
                {
                    lines[element] += '\0';
                    element++;
                }
                parse++;
            }
            element = 0;
            parse = 0;

            PC = strtoul(lines[0].c_str(), 0, 16);
            instr.opcode = strtoul(lines[1].c_str(), 0, 10);
            instr.dest = strtoul(lines[2].c_str(), 0, 10);
            instr.src1 = strtoul(lines[3].c_str(), 0, 10);
            instr.src2 = strtoul(lines[4].c_str(), 0, 10);

            instr.tag = instr_counter++;
            instr.current_state = IF;
            instr.IF_START = cycle_count;
            rob.push_back(instr);

            Instruction dispatch_list_entry = instr;
            dispatch_list.push_back(dispatch_list_entry);
        } else {
            is_last_instruction = true;
            trace.close();
            break;
        }
    }
}

void dispatch() {
    if (dispatch_list.size() > 0) {
        list<Instruction> dispatch_list_copy;
        list<Instruction>::iterator iter;
        list<Instruction>::iterator rob_iter;

        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->current_state == ID){
                for (iter = dispatch_list.begin(); iter != dispatch_list.end(); iter++){
                    Instruction *dispatch_list_entry = &*iter;
                    if (dispatch_list_entry->tag == instr->tag){
                        dispatch_list_copy.push_back(*dispatch_list_entry);
                        break;
                    }
                }
            }
        }

        for (iter = dispatch_list_copy.begin(); iter != dispatch_list_copy.end() && !issue_handler.is_full(); iter++){
            Instruction *dispatch_list_entry = &*iter;           
            dispatch_list_entry->sr1_name = dispatch_list_entry->src1;
            dispatch_list_entry->sr2_name = dispatch_list_entry->src2;
            dispatch_list_entry->is_empty = false;

            issue_handler.push(*dispatch_list_entry, reg_file_list);
            for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
                Instruction *instr = &*rob_iter;
                if (instr->tag == dispatch_list_entry->tag){
                    instr->current_state = IS;
                    instr->ID_END = cycle_count;
                    instr->IS_START = cycle_count;
                    break;
                }
            }
        }

        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->current_state == IS){
                for (iter = dispatch_list.begin(); iter != dispatch_list.end(); iter++){
                    Instruction *dispatch_list_entry = &*iter;
                    if (dispatch_list_entry->tag == instr->tag){
                        dispatch_list.pop_front();
                        break;
                    }
                }
            }
        }

        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->current_state == IF){
                for (iter = dispatch_list.begin(); iter != dispatch_list.end(); iter++){
                    Instruction *dispatch_list_entry = &*iter;
                    if (dispatch_list_entry->tag == instr->tag){
                        dispatch_list_entry->current_state = ID;
                        instr->current_state = ID;
                        instr->IF_END = cycle_count;
                        instr->ID_START = cycle_count;
                        break;
                    }
                }
            }
        }
    }
}

void issue(){

    if (issue_handler.size() > 0)
    {
        vector<Instruction> issue_list_copy = issue_handler.issue();

        sort(issue_list_copy.begin(), issue_list_copy.end(), tag_compare);

        list<Instruction>::iterator rob_iter;
        vector<Instruction>::iterator iter;
        int issue_count = 0;

        for (iter = issue_list_copy.begin(); iter != issue_list_copy.end() && issue_count++ < N; iter++){
            Instruction *issue_list_entry = &*iter;
            execute_list.push_back(*issue_list_entry);
            for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
                Instruction *instr = &*rob_iter;
                if (instr->tag == issue_list_entry->tag){
                    int latency = 0;
                    instr->current_state = EX;
                    instr->update_exec_time(latency);
                    instr->EX_START = cycle_count;
                    instr->IS_END = cycle_count;
                    break;
                }
            }
        }

        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->current_state == EX){
                for (iter = issue_list_copy.begin(); iter != issue_list_copy.end(); iter++){
                    Instruction *issue_list_entry = &*iter;
                    if (issue_list_entry->tag == instr->tag){
                        issue_handler.pop(issue_list_entry->tag);
                        break;
                    }
                }
            }
        }
    }
}

void execute(){

    if (execute_list.size() > 0){
        list<Instruction>::iterator rob_iter;
        list<Instruction>::iterator iter;
        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->is_exec_done(cycle_count)){
                instr->WB_START = cycle_count;
                instr->WB_END = cycle_count + 1;
                instr->EX_END = cycle_count;
                instr->current_state = WB;
            }
        }

        for (rob_iter = rob.begin(); rob_iter != rob.end(); rob_iter++){
            Instruction *instr = &*rob_iter;
            if (instr->current_state == WB){
                for (iter = execute_list.begin(); iter != execute_list.end(); iter++){
                    Instruction *exec_list_entry = &*iter;
                    if (exec_list_entry->tag == instr->tag){
                        if (exec_list_entry->dst_name != -1 && exec_list_entry->dst_name == reg_file_list[exec_list_entry->dest].name){
                            reg_file_list[exec_list_entry->dest].is_ready = true;
                        }
                        issue_handler.update(exec_list_entry->dst_name);
                        execute_list.erase(iter);
                        break;
                    }
                }
            }
        }
    }
}

void retire(){
    
    while (rob.size() > 0){
        list<Instruction>::iterator rob_iter = rob.begin();
        if (rob_iter->current_state == WB){
            cout << dec << rob_iter->tag << "  fu{" << rob_iter->opcode << "}";
            cout << " src{" << rob_iter->src1 << "," << rob_iter->src2 << "}";
            cout << " dst{" << rob_iter->dest << "}";
            cout << " IF{" << rob_iter->IF_START << "," << rob_iter->IF_END - rob_iter->IF_START << "}";
            cout << " ID{" << rob_iter->ID_START << "," << rob_iter->ID_END - rob_iter->ID_START << "}";
            cout << " IS{" << rob_iter->IS_START << "," << rob_iter->IS_END - rob_iter->IS_START << "}";
            cout << " EX{" << rob_iter->EX_START << "," << rob_iter->EX_END - rob_iter->EX_START << "}";
            cout << " WB{" << rob_iter->WB_START << "," << rob_iter->WB_END - rob_iter->WB_START << "}";
            cout << endl;
            rob.erase(rob_iter);
        } else {
            break;
        }
    }
}



