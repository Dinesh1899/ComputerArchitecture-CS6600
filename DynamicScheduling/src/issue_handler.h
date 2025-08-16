#ifndef ISSUEHANDLER
#define ISSUEHANDLER

#include "components.h"
#include <vector>


using namespace std;

class IssueHandler {
    private:
        int S;
        Instruction *issue_list;
    public:
        IssueHandler();
        IssueHandler(int);
        
        int size();
        bool is_full();
        void push(Instruction, Register *);
        void rename(int, Register *);
        void pop(int);
        void update(int);
        vector<Instruction> issue();
};

#endif
