#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iomanip>
#include <cassert>
#include <map>
#include "predictor.h"

int PREDICTOR_TYPE;

typedef unsigned long ulong;
using namespace std;


bool is_branch_taken(char status){
  if(status == 't') return true;
  else return false;
}

int main(int argc, char *argv[]){
  int m;  // No. of lower order PC bits used for indexing into Prediction Counter Table
  int n;  // No. of bits used for the global branch history register
  
  char *type;  // bimodal or gshare
  char *trace_file_path; // Trace file path
  FILE *tracefile;

  ulong address;
  char branch_status;

  type = argv[1];
  //cout<<"Type is: "<<type<<endl;
  if(strcmp(type, "bimodal") == 0){
    //cout<<"Type is: "<<type<<endl;
    m = static_cast<int>(strtoul(argv[2], 0, 10));
    trace_file_path = argv[3];
    tracefile = fopen(trace_file_path, "r+");
    predictor* pr = new predictor(m);

    while(1) {
      if(fscanf(tracefile, "%lx %c", &address, &branch_status) != EOF)
	      pr->predict_bimodal(address, is_branch_taken(branch_status));
	    else 
        break;
      }
      cout << "COMMAND\n./bpsim " << argv[1] << " " <<  m << " " << trace_file_path << endl; 
      pr->show_bimodal();   
  }else if(strcmp(type, "gshare") == 0){
    m = static_cast<int>(strtoul(argv[2], 0, 10));
    n = static_cast<int>(strtoul(argv[3], 0, 10));
    trace_file_path = argv[4];
    tracefile = fopen(trace_file_path, "r+");
    predictor* pr = new predictor(m, n);

    while(1) {
      if(fscanf(tracefile, "%lx %c", &address, &branch_status) != EOF)
	      pr->predict_gshare(address, is_branch_taken(branch_status));
	    else 
        break;
      }
      cout << "COMMAND\n./bpsim " << argv[1] << " " << m << " " << n  << " " << trace_file_path << endl;
      pr->show_gshare();
  }else{
    cout << "Invalid Input, type should be either gshare or bimodal" << endl;
    return 1;
  }

    return 0;
}
