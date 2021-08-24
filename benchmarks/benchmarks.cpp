#include "benchmarks.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
int main(){
    int sizes[]=   { 10000, 16000, 32000, 64000, 120000, 1232896, 1600000, 2000000, 2500000, 3500000, 100000000, 150000000, 250000000, 350000000, 500000000};
    int iters[]=   { 10000, 16000, 32000, 64000, 120000,    5000,    3000,    3000,    3000,    2000,        25,        25,         3,         3,         2};
    int exp_len[]= { 10000, 16000, 32000, 64000, 120000,  600000, 1000000, 1000000, 1500000, 3500000,   1000000,  10000000,  20000000,  30000000,  40000000};
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa";
    std::cout<<std::endl <<"\x1B[33m shuftiExec Benchmarks(kbytes) \x1B[0m"<<std::endl;
    for (int i = 0; i < 5; i++) { 
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m rshuftiExec Benchmarks(kbytes) \x1B[0m"<<std::endl;
    for (int i = 0; i < 5; i++) { 
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m truffleExec Benchmarks(kbytes) \x1B[0m"<<std::endl;
    for (int i = 0; i < 5; i++) { 
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m rtruffleExec Benchmarks(kbytes) \x1B[0m"<<std::endl;
     for (int i = 0; i < 5; i++) { 
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m shuftiExec Benchmarks(Mbytes) \x1B[0m"<<std::endl;
    for (int i = 5; i < 10; i++) { 
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m rshuftiExec Benchmarks(Mbytes) \x1B[0m"<<std::endl;
    for (int i = 5; i < 10; i++) { 
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m truffleExec Benchmarks(Mbytes) \x1B[0m"<<std::endl;
     for (int i = 5; i < 10; i++) { 
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m rtruffleExec Benchmarks(Mbytes) \x1B[0m"<<std::endl;
     for (int i = 5; i < 10; i++) { 
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m shuftiExec Benchmarks(Gbytes) \x1B[0m"<<std::endl;
    for (int i = 10; i < 15; i++) { 
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        shufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
        // run time 2.5 min
    }
    std::cout<<std::endl <<"\x1B[33m rshuftiExec Benchmarks(Gbytes) \x1B[0m"<<std::endl;
    for (int i = 10; i < 15; i++) { 
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rshufti_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m truffleExec Benchmarks(Gbytes) \x1B[0m"<<std::endl;
     for (int i = 10; i < 15; i++) { 
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        truffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    std::cout<<std::endl <<"\x1B[33m rtruffleExec Benchmarks(Gbytes) \x1B[0m"<<std::endl;
     for (int i = 10; i < 15; i++) { 
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],false);
        rtruffle_benchmarks(sizes[i],iters[i],exp_len[i],true);
    }
    /*noodle_benchmarks(120000, 32000, "aaaA", 4, 1); ---> kill
      noodle_benchmarks(2500000, 5000, "AaAAaaaA", 8, 1); ---> kill
      γενικά όταν βάζω ένα string μεγέθους > 4 για nocase = 1 κάνει kill.
    */
    std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(kbytes) \x1B[0m"<<std::endl;
    for (int char_len = 1; char_len < 9; char_len++) {
        char *str = new char[char_len];
        for (int j=0; j<char_len; j++) {
            srand (time(NULL));
            int key = rand() % + 36 ;
            str[char_len] = charset[key];
            str[char_len + 1] = '\0';
        }
        for (int i=0; i<5; i++){
            noodle_benchmarks(sizes[i], iters[i], str,char_len,0);
        }
        delete [] str;    
    }
    std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(Mbytes) \x1B[0m"<<std::endl;
    for (int char_len = 1; char_len < 9; char_len++) {
        char *str = new char[char_len];
        for (int j=0; j<char_len; j++) {
            srand (time(NULL));
            int key = rand() % + 36 ;
            str[char_len] = charset[key];
            str[char_len + 1] = '\0';
        }
        for (int i=5; i<10; i++){
            noodle_benchmarks(sizes[i], iters[i], str,char_len,0);
        }
        delete [] str;    
    }
    std::cout<<std::endl <<"\x1B[33m noodle Benchmarks(Gbytes) \x1B[0m"<<std::endl;
    for (int char_len = 1; char_len < 9; char_len++) {
        char *str = new char[char_len];
        for (int j=0; j<char_len; j++) {
            srand (time(NULL));
            int key = rand() % + 36 ;
            str[char_len] = charset[key];
            str[char_len + 1] = '\0';
        }
        for (int i=10; i<15; i++){
            noodle_benchmarks(sizes[i], iters[i], str,char_len,0);
        }
        delete [] str;    
    }
    return 0;
}