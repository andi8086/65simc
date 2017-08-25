#include "sim.h"

uint8_t memory[65536];

struct CPU cpu;

void group0(int a);
void group1(int a);
void group2(int a);
void group3(int a);
void group4(int a);
void group5(int a);
void group6(int a);
void group7(int a);
void groupORA(int a);
void groupAND(int a);
void groupEOR(int a);
void groupADC(int a);    
void groupSTA(int a);     
void groupLDA(int a);     
void groupCMP(int a); 
void groupSBC(int a);    
void groupASL(int a); 
void groupROL(int a);    
void groupLSR(int a);   
void groupROR(int a); 
void groupSX(int a); 
void groupLX(int a);  
void groupDEC(int a); 
void groupINC(int a); 
void groupSLO(int a); 
void groupRLA(int a); 
void groupSRE(int a); 
void groupRRA(int a); 
void groupSAX(int a); 
void groupLAX(int a); 
void groupDCP(int a); 
void groupISC(int a); 

struct op_ccaaa opcodes[] = {
    {0, 0, {aNULL, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {7, 3, 3, 4, 2, 4, 2, 4}, group0},
    {0, 1, {aabs, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {6, 3, 4, 4, 2, 4, 2, 4}, group1},
    {0, 2, {aNULL, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {6, 3, 3, 3, 2, 4, 2, 4}, group2},
    {0, 3, {aNULL, azp, aNULL, aind, arel, azpx, aNULL, aabx}, {6, 3, 4, 5, 2, 4, 2, 4}, group3},
    {0, 4, {aimm, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 5}, group4},
    {0, 5, {aimm, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group5},
    {0, 6, {aimm, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group6},
    {0, 7, {aimm, azp, aNULL, aabx, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group7},
    {1, 0, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupORA},
    {1, 1, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupAND},
    {1, 2, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupEOR},
    {1, 3, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupADC},
    {1, 4, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 6, 4, 5, 5}, groupSTA},
    {1, 5, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupLDA},
    {1, 6, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupCMP},
    {1, 7, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupSBC},
    {2, 0, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupASL},
    {2, 1, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupROL},
    {2, 2, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupLSR},
    {2, 3, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupROR},
    {2, 4, {aimm, azp, aNULL, aabs, aNULL, azpy, aNULL, aaby}, {2, 3, 2, 4, 0, 4, 2, 5}, groupSX},
    {2, 5, {aimm, azp, aNULL, aabs, aNULL, azpy, aNULL, aaby}, {2, 3, 2, 4, 0, 4, 2, 4}, groupLX},
    {2, 6, {aimm, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {2, 5, 2, 6, 0, 6, 0, 7}, groupDEC},
    {2, 7, {aimm, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {2, 5, 2, 6, 0, 6, 2, 7}, groupINC},
    {3, 0, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupSLO},
    {3, 1, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupRLA},
    {3, 2, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupSRE},
    {3, 3, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6 , 8, 6, 7, 7}, groupRRA},
    {3, 4, {aizx, azp, aimm, aabs, aizy, azpy, aaby, aaby}, {6, 3, 2, 4, 6, 4, 5, 5}, groupSAX},
    {3, 5, {aizx, azp, aimm, aabs, aizy, azpy, aaby, aaby}, {6, 3, 2, 4, 5, 4, 4, 4}, groupLAX},
    {3, 6, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupDCP},
    {3, 7, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupISC} 
};

int main(int argc, char* argv[])
{
    bool done;
    uint8_t cc, idx;
    enum amode a;
    uint8_t op, cycles;

    cpu.PC = 0x200;
    cpu.S = 0xFF;
    cpu.F = 0;

    done = true;
    while(!done) {
        op = memory[cpu.PC];
        cc = op & 3;
        op >>= 2;
        op |= cc;
        idx = op >> 3 & 32;
        a = opcodes[idx].addr_modes[op & 7];
        cycles = opcodes[idx].cycles[op & 7];
        opcodes[idx].func(a);
    }

    return 0;

}
