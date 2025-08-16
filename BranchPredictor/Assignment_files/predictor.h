#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <iomanip>
#include<vector>

typedef unsigned long ulong;

using namespace std;

class predictor{

    private:
    int m,n;
    vector<int> counter_table;
    int gbhr;

    int miss_predictions;
    int total_predictions;

    public:

    predictor(int m){
        this->m = m;
        int len = (int) pow(2, (double) m);
        counter_table.resize(len);

        miss_predictions = 0;
        total_predictions = 0;

        for (int i = 0; i < len; i++) counter_table[i] = 2; 

    }

    predictor(int m, int n){
        this->m = m;
        this->n = n;
        int len = (int) pow(2, (double) m);
        counter_table.resize(len);

        gbhr = 0;
        miss_predictions = 0;
        total_predictions = 0;
        for (int i = 0; i < len; i++) counter_table[i] = 2;
    }

    void predict_bimodal(ulong address, bool is_taken){
        total_predictions++;
        
        int mask = (int) pow(2, (double) m) - 1;
        int index = (address >> 2) & mask;

        int counter = counter_table[index];
        bool predict_taken = counter >=2 ? true : false;
    
        if(predict_taken == is_taken){
            if(is_taken){
                counter_table[index] = counter < 3 ? counter+1 : 3; 
            }else{
                counter_table[index] = counter > 0 ? counter-1 : 0; 
            }
        }else{
            miss_predictions++;
            if(is_taken){
                counter_table[index] = counter < 3 ? counter+1 : 3; 
            }else{
                counter_table[index] = counter > 0 ? counter-1 : 0; 
            }            
        }

    }


    void predict_gshare(ulong address, bool is_taken){
        total_predictions++;
        
        int index;

        int mask = (int) pow(2, (double) m) - 1;
        int masked_pc = (address >> 2) & mask; // Get m PC bits except 2 LSBs

        if(n > 0){
            int pc_lsb_mask = (int) pow(2, (double) m-n) - 1;
            int m_pc_xor_gbhr = gbhr ^ (masked_pc >> (m - n));
            m_pc_xor_gbhr = m_pc_xor_gbhr << (m - n);
            int pc_lsb = masked_pc & pc_lsb_mask;
            index = m_pc_xor_gbhr | pc_lsb;
        }else{
            index = masked_pc;
        }

        int counter = counter_table[index];
        bool predict_taken = counter >=2 ? true : false;
    
        if(predict_taken == is_taken){
            if(is_taken){
                counter_table[index] = counter < 3 ? counter+1 : 3;
                shift_update_gbhr(1);
                 
            }else{
                counter_table[index] = counter > 0 ? counter-1 : 0;
                shift_update_gbhr(0); 
            }
        }else{
            miss_predictions++;
            if(is_taken){
                counter_table[index] = counter < 3 ? counter+1 : 3;
                shift_update_gbhr(1); 
            }else{
                counter_table[index] = counter > 0 ? counter-1 : 0;
                shift_update_gbhr(0); 
            }            
        }
    
    }

  void shift_update_gbhr(int i){
    if (n > 0) {
      gbhr = gbhr >> 1; 
      gbhr = gbhr | (i << (n - 1));
    }
  }


  void show_bimodal(){

    cout << "OUTPUT\nnumber of predictions: " << total_predictions << "\nnumber of mispredictions: " << miss_predictions << fixed << setprecision(2) << "\nmisprediction rate: " << (float)miss_predictions*100/total_predictions << "%\n";

    cout << "FINAL BIMODAL CONTENTS\n";

    for(int i = 0; i < counter_table.size(); i++){
        cout << i << "\t" << counter_table[i] << "\n";
    }

  }
  
  void show_gshare(){

    cout << "OUTPUT\nnumber of predictions: " << total_predictions << "\nnumber of mispredictions: " << miss_predictions << fixed << setprecision(2) << "\nmisprediction rate: " << (float)miss_predictions*100/total_predictions << "%\n";

    cout << "FINAL GSHARE CONTENTS\n";

    for(int i = 0; i < counter_table.size(); i++){
        cout << i << "\t" << counter_table[i] << "\n";
    }

  }


  float get_miss_prediction_rate(){
    float miss_pred_rate = (float)miss_predictions*100/total_predictions;
    return miss_pred_rate;
  }

};