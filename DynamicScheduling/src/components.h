#ifndef COMPONENTS
#define COMPONENTS

typedef enum { IF, ID, IS, EX, WB } state;


struct Register{

    public:
        bool is_ready = true;
        int name = -1;    

};

struct Instruction {
    
    public:

        int tag;
        state current_state;
        int opcode;
        int dest;
        int src1;
        int src2;

        int sr1_name;
        int sr2_name;
        int dst_name = -1;

        bool sr1_ready = false;        
        bool sr2_ready = false;
        bool is_empty = true;

        int IF_START;
        int IF_END;

        int ID_START;
        int ID_END;

        int IS_START;
        int IS_END;

        int EX_START;
        int EX_END;

        int WB_START;
        int WB_END;                        

        int exec_time = 0;

        void update_exec_time(int);
        bool is_exec_done(int);

};

#endif