#include "Dataset.h"
#include "CorrectDetector.hpp"
#include "SpaceSaving.hpp"
#include "HeavyDetector.hpp"
#include <iomanip>
#include <stdio.h>
#include <chrono>
#include "util.h"

#undef WEIGHT

using VERTEX_ID_TYPE = uint32_t;
using FP_TYPE = uint8_t;
using WEIGHT_TYPE = uint32_t;

std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> vertex;

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
void Compare(std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& truth,std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& detect,bool alldrop = true){
    std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> truth_map;
    uint32_t sum = 0;
    for(auto elem:truth){
        truth_map[elem._v] = elem;
        for(auto edge:elem._edges){
            sum += edge.second;
        }
    }
    uint32_t normal_sum = 0;
    for(auto elem:vertex){
        normal_sum += elem.second._sum;
    }
    normal_sum-=sum;
    uint32_t correct = 0;
    uint32_t edge_correct = 0;
    uint32_t edge_find = 0;
    uint32_t d_sum = 0;
    uint32_t normal = 0;
    for(auto elem:detect){
        if(truth_map.find(elem._v)!=truth_map.end()){
            correct++;
            if(alldrop){
                normal += truth_map[elem._v]._sum;
                for(auto edge:truth_map[elem._v]._edges){
                    d_sum+=edge.second;
                    normal -= edge.second;
                }
            }else{
                for(auto edge:elem._edges){
                    if(truth_map[elem._v]._edges.find(edge.first)!=truth_map[elem._v]._edges.end()){
                        d_sum+=truth_map[elem._v]._edges[edge.first];
                    }else{
                        normal += vertex[elem._v]._edges[edge.first];
                    }
                }
            }
        }
    }
    std::cout<<"drop ddos "<<d_sum<<"/"<<sum<<" ,ATC is "<<double(d_sum)/double(sum) <<std::endl;
    std::cout<<"drop normal "<<normal<<"/"<<normal_sum<<" ,DC is "<<double(normal)/double(normal_sum)<<std::endl;
    
    return;
}

int main(void){
    std::string datasetPath="./Data/ddos_caida.dat";
    Dataset dataset(datasetPath,-1);
    CorrectDetector<VERTEX_ID_TYPE,WEIGHT_TYPE> detector;
    for(int i = 0;i<dataset.dataset.size();i++){
        detector.Insert(dataset.dataset[i].u,dataset.dataset[i].v,dataset.dataset[i].weight);
    }
    assert(mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK);
    detector.CountDegree();
    vertex_top_k = detector._vertex_degree_sum * 0.25;
    edge_top_k = 2;
    vertex = detector.GetVertex();
    auto output = detector.Query();
    ParamInitialize();
    auto ss = new Spacesaving<VERTEX_ID_TYPE,WEIGHT_TYPE>(vertex_top_k,0);
    for(int i = 0;i<dataset.dataset.size();i++){
        ss->Insert(dataset.dataset[i].u);
    }
    auto output1 = ss->Query();
    std::cout<<"Perfect:"<<std::endl;
    Compare(output,output);
    std::cout<<"SpaceSaving:"<<std::endl;
    Compare(output,output1);
    int mem = vertex_top_k * 44; 
    HeavyDetector<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE> heavy(mem);
    for(int i = 0;i<dataset.dataset.size();i++){
        heavy.Insert(dataset.dataset[i].u,dataset.dataset[i].v,dataset.dataset[i].weight);
    }
    auto output2 = heavy.Query();
    std::cout<<"HeavyGraph:"<<std::endl;
    Compare(output,output2,false);
    return 0;
}