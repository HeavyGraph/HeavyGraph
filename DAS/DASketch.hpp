#ifndef _DASKETCH_H
#define _DASKETCH_H
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BOBHASH32.h"
#include "Param.h"
#include "hashTable.h"
#define CMM_d 4
#define TOP_d 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class DASketch {
private:
    struct cell { // cell struct, stores flow ID, strategic frequency Cs and real frequency Cr
        string ID;
        int Cs;
        int Cr;
    };
    struct bucket { // multiple cells in a bucket
        cell cells[TOP_d];
    };
    bucket *bk; // pointer to the bucket array
    struct node { int C; } HK[CMM_d][MAX_MEM+10]; // lower layer CMM sketch
    int K, M2; // K, M2: width of the upper bucket array and width of the lower CMM sketch
    int total; // total number of elements in CMM
public:
    DASketch(int M2, int K) :M2(M2), K(K) {
        bk = new bucket[M2];
        total = 0;
    }
    ~DASketch(){
        delete []bk;
    }
    void clear()
    {
        for (int i = 0; i < M2; i++)
            for (int j = 0; j < TOP_d; j++) {
                bk[i].cells[j].ID = "";
                bk[i].cells[j].Cs = 0;
                bk[i].cells[j].Cr = 0;
            }
        for (int i = 0; i < CMM_d; i++)
            for (int j = 0; j <= M2 + 5; j++)
                HK[i][j].C = 0;
        total = 0;
    }
    unsigned long long Hash(string ST)
    {
        return hfunc[0]((unsigned char*)ST.c_str(),ST.size());
    }
    int Query(const string& x){
        int h = Hash(x) % M2; // bucket array index
        for (int i = 0; i < TOP_d; i++) { // check for a matching cell
            if (bk[h].cells[i].ID == x) { // if a matching cell is found, increment Cs and Cr directly
                return bk[h].cells[i].Cs;
                break;
            }
        }
        return 0;
    }
    void Insert(const string &x,int weight=1)
    {
        int h = Hash(x) % M2; // bucket array index
        bool match = false; // flag to check if a cell is matched
        bool empty = false; // flag to check if there is an empty cell
        int minv = 0x7fffffff; // record the minimum Cs value
        int minp = -1; // record the position of the minimum Cs value
        for (int i = 0; i < TOP_d; i++) { // check for a matching cell
            if (bk[h].cells[i].ID == x) { // if a matching cell is found, increment Cs and Cr directly
                bk[h].cells[i].Cs+=weight;
                bk[h].cells[i].Cr+=weight;
                match = true;
                break;
            }
        }
        if (!match) { // check for an empty slot
            for (int i = 0; i < TOP_d; i++) {
                if (bk[h].cells[i].ID == "") { // if there is an empty slot, assign it to the new element and set Cs and Cr to 1
                    bk[h].cells[i].ID = x;
                    bk[h].cells[i].Cs = weight;
                    bk[h].cells[i].Cr = weight;
                    empty = true;
                    break;
                }
            }
        }
        if (!match && !empty) { // if no match and no empty slot, use replacement strategy
            for(int i = 0; i < TOP_d; i++){ // find the cell with the minimum value
                if(bk[h].cells[i].Cs<minv){
                    minv = bk[h].cells[i].Cs;
                    minp = i;
                }
            }
            double p = 1.0 / (minv + 1); // replacement probability
            double r = rand() / double(RAND_MAX); // generate a random number
            if (r < p) { // if replacement is successful, change the flow ID of the replaced cell to the new element, set Cs to minv+1, Cr to 1, and hash the Cr of the evicted element into the lower CMM sketch
                string evicted = bk[h].cells[minp].ID; // record the evicted element
                int evicted_cr = bk[h].cells[minp].Cr; // record the Cr of the evicted element
                bk[h].cells[minp].ID = x; // replace flow ID
                bk[h].cells[minp].Cs = minv + weight; // update Cs
                bk[h].cells[minp].Cr = weight; // update Cr
                unsigned long long hash[CMM_d]; // calculate the position of the evicted element in the lower CMM sketch
                for (int i = 0; i < CMM_d; i++)
                    hash[i]=Hash(evicted+std::to_string(i))%(M2-(2*CMM_d)+2*i+3);
                for(int i = 0; i < CMM_d; i++) { // hash the Cr of the evicted element into the lower CMM sketch
                    HK[i][hash[i]].C += evicted_cr;
                }
                total += evicted_cr; // update the total number of elements inserted into the lower layer
            }
            else { // if replacement fails, insert the element into the lower CMM sketch
                unsigned long long hash[CMM_d];
                for (int i = 0; i < CMM_d; i++)
                    hash[i]=Hash(x+std::to_string(i))%(M2-(2*CMM_d)+2*i+3);
                for(int i = 0; i < CMM_d; i++) {
                    HK[i][hash[i]].C++;
                }
                total++;
            }
        }
    }
    struct Node { string x; int y; } q[MAX_MEM + 10];
    static int cmp(Node i, Node j) { return i.y > j.y; }
    void work()
    {
        int CNT = 0;
        for (int i = 0; i < M2; i++)
            for (int j = 0; j < TOP_d; j++) {
                if (bk[i].cells[j].ID != "") {
                    q[CNT].x = bk[i].cells[j].ID;
                    q[CNT].y = bk[i].cells[j].Cs;
                    CNT++;
                }
            }
        sort(q, q + CNT, cmp);
    }
};
#endif