#include <iostream>
#include <list>
#include <vector>
#include "issue_handler.h"

using namespace std;

IssueHandler::IssueHandler(){ }

IssueHandler::IssueHandler(int size){
    S = size;
    issue_list = new Instruction[S];
}

bool IssueHandler::is_full(){

    bool is_full = true;
    for (int iter = 0; iter < S; iter++)
    {
        if (issue_list[iter].is_empty)
        {
            is_full = false;
            break;
        }
    }
    return is_full;
}

int IssueHandler::size() {

    int len = 0;
    for (int iter = 0; iter < S; iter++){
        if (!issue_list[iter].is_empty)
        {
            len++;
        }
    }
    return len;
}

void IssueHandler::pop(int tag){

    for (int iter = 0; iter < S; iter++)
    {
        if (issue_list[iter].tag == tag)
        {
            issue_list[iter].is_empty = true;
            issue_list[iter].sr1_ready = false;
            issue_list[iter].sr2_ready = false;
            break;
        }
    }
}

void IssueHandler::push(Instruction instr, Register *reg_file_list) {

    int iter;
    for (iter = 0; iter < S; iter++){
        if (issue_list[iter].is_empty){
            issue_list[iter] = instr;
            rename(iter, reg_file_list);
            break;
        }
    }
}

void IssueHandler::rename(int current, Register *reg_file_list){

    if (issue_list[current].sr1_name != -1){

        issue_list[current].sr1_ready = reg_file_list[issue_list[current].sr1_name].is_ready;
        issue_list[current].sr1_name = reg_file_list[issue_list[current].sr1_name].name;

    }else{
        issue_list[current].sr1_ready = true;
    }

    if (issue_list[current].sr2_name != -1){
        issue_list[current].sr2_ready = reg_file_list[issue_list[current].sr2_name].is_ready;
        issue_list[current].sr2_name = reg_file_list[issue_list[current].sr2_name].name;
    }else{
        issue_list[current].sr2_ready = true;
    }

    if (issue_list[current].dest != -1){
        issue_list[current].dst_name = issue_list[current].tag;
        reg_file_list[issue_list[current].dest].name = issue_list[current].dst_name;
        reg_file_list[issue_list[current].dest].is_ready = false;
    }
}

vector<Instruction> IssueHandler::issue(){
    
    vector<Instruction> temp_issue_list;

    for (int iter = 0; iter < S; iter++){
        if (issue_list[iter].sr1_ready && issue_list[iter].sr2_ready && !issue_list[iter].is_empty)
        {
            temp_issue_list.push_back(issue_list[iter]);
        }
    }

    return temp_issue_list;
}

void IssueHandler::update(int reg){

    for (int iter = 0; iter < S; iter++){
        if (issue_list[iter].sr1_name == reg){
            issue_list[iter].sr1_ready = true;
        }

        if (issue_list[iter].sr2_name == reg){
            issue_list[iter].sr2_ready = true;
        }
    }
}