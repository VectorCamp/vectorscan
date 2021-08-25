#include "nfa/truffle.h"
#include "benchmarks.hpp"
#include <iostream>
#include <chrono>
#include <time.h>
/*
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
*/


void truffle_benchmarks(int size, int loops, int M, bool has_match) {
    m128 lo, hi;
    char *kt1 = new char[size];
    memset(kt1,'b',size);
    double total_sec = 0;
    if (has_match){
        int pos = 0;
        for(int j=0; j<M; j++){
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < loops/M; i++) {
                truffleExec(lo, hi, (u8 *)kt1 + i, (u8 *)kt1 + strlen(kt1));
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> shuftiExec_elapsed_seconds = end-start;
            total_sec += shuftiExec_elapsed_seconds.count();
        }
        total_sec /= M;
        std::cout<<"\x1B[35m Case with match in random pos and size: "<<size<<" for "<<loops<<" loops ("<< M <<" random possisions checked):"<<"\x1B[36m truffleExec elapsetime: \x1B[0m"<<total_sec<<" bandwidth"<<(size/total_sec)<<std::endl;
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            truffleExec(lo, hi, (u8 *)kt1 + i, (u8 *)kt1 + strlen(kt1));
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> shuftiExec_elapsed_seconds = end-start;
        total_sec += shuftiExec_elapsed_seconds.count();
        std::cout<<"\x1B[35m Case with no match in random pos and size: "<<size<<" for "<<loops<<" loops:"<<"\x1B[36m truffleExec elapsetime: \x1B[0m"<<total_sec<<" bandwidth"<<(size/total_sec)<<std::endl;
    }
    delete [] kt1;
}


void rtruffle_benchmarks(int size, int loops, int M, bool has_match) {  
    m128 lo, hi;
    char *kt1 = new char[size];
    memset(kt1,'b',size);
    double total_sec = 0;
    if (has_match){
        int pos = 0;
        for(int j=0; j<M; j++){
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < loops/M; i++) {
                rtruffleExec(lo, hi, (u8 *)kt1 + i, (u8 *)kt1 + strlen(kt1));
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> shuftiExec_elapsed_seconds = end-start;
            total_sec += shuftiExec_elapsed_seconds.count();
        }
        total_sec /= M;
        std::cout<<"\x1B[35m Case with match in random pos and size: "<<size<<" for "<<loops<<" loops ("<< M <<" random possisions checked):"<<"\x1B[36m rtruffleExec elapsetime: \x1B[0m"<<total_sec<<" bandwidth"<<(size/total_sec)<<std::endl;
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            rtruffleExec(lo, hi, (u8 *)kt1 + i, (u8 *)kt1 + strlen(kt1));
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> shuftiExec_elapsed_seconds = end-start;
        total_sec += shuftiExec_elapsed_seconds.count();
        std::cout<<"\x1B[35m Case with no match in random pos and size: "<<size<<" for "<<loops<<" loops:"<<"\x1B[36m rtruffleExec elapsetime: \x1B[0m"<<total_sec<<" bandwidth"<<(size/total_sec)<<std::endl;
    }
    delete [] kt1;
}