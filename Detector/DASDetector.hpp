#ifndef _DASDETECTOR_H
#define _DASDETECTOR_H

#include "DASketch.hpp"
template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE >
class DASDetector:public DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
    
    private:
        DASketch* v_sketch;
        DASketch* e_sketch;
        Ssummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* ss;
        int thresh = 3;
    public:
        DASDetector(int memory){
            int DA_M;
            for (DA_M=1; (128 + (sizeof(VERTEX_ID_TYPE)*3) * 8)*DA_M*TOP_d+64*DA_M*CMM_d<=memory*1024*8; DA_M++);
            if (DA_M%2==0) {
                DA_M--;
            }
            v_sketch = new DASketch(DA_M,0);
            e_sketch = new DASketch(DA_M,0);
            ss = new Ssummary<VERTEX_ID_TYPE,WEIGHT_TYPE>(vertex_top_k);
            ss->Clear();
        }
        ~DASDetector(){
            delete v_sketch;
            delete e_sketch;
            delete ss;
        }
        void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            
            std::string s_v(sizeof(_v), '\0');
            memcpy(s_v.data(), &_v, sizeof(_v));
            
            std::string s_u(sizeof(_u), '\0');
            memcpy(s_u.data(), &_u, sizeof(_u));
            
            s_u += s_v;
            v_sketch->Insert(s_v,_weight);
            e_sketch->Insert(s_u,_weight);
            int maxv = v_sketch->Query(s_v);
            int edge_maxv = e_sketch->Query(s_u);
            bool mon = false;
            int p = ss->Find(_v);
            if (p) mon = true;
            if (!mon)
            {
                #ifdef WEIGHT
                if (maxv > ss->Getmin() || ss->tot < vertex_top_k)
                #else
                if (maxv - (ss->Getmin()) == 1 || ss->tot < vertex_top_k)
                #endif
                {
                    int i = ss->Getid();
                    ss->Add2(ss->Location(_v), i);
                    ss->vertex[i] = _v;
                    ss->sum[i] = maxv;
                    ss->tmp[i] = maxv;
                    ss->Link(i, 0);
                    while (ss->tot > vertex_top_k)
                    {
                        int t = ss->Right[0];
                        int tmp = ss->head[t];
                        ss->Cut(ss->head[t]);
                        ss->Recycling(tmp);
                    }

                    if(ss->e[i]==nullptr){
                        ss->e[i] = new Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>();
                    }
                    auto es = (ss->e[i]);
                    bool emon = false;
                    int ep = es->Find(_u);
                    if (ep) emon = true;

                    if(!emon){
                        #ifdef WEIGHT
                        if (edge_maxv > es->Getmin() || es->tot < edge_top_k)
                        #else
                        if (edge_maxv - (es->Getmin()) == 1 || es->tot < edge_top_k)
                        #endif
                        {
                            int i = es->Getid();
                            es->Add2(es->Location(_u), i);
                            es->vertex[i] = _u;
                            es->sum[i] = edge_maxv;
                            es->tmp[i] = edge_maxv;
                            es->Link(i, 0);
                            while (es->tot > edge_top_k)
                            {
                                int t = es->Right[0];
                                int tmp = es->head[t];
                                es->Cut(es->head[t]);
                                es->Recycling(tmp);
                            }
                        }
                    }
                    else if (edge_maxv > es->sum[ep])
                    {
                        int tmp = es->Left[es->sum[ep]];
                        es->Cut(ep);
                        if (es->head[es->sum[ep]]) tmp = es->sum[ep];
                        es->sum[ep] = edge_maxv;
                        es->Link(ep, tmp);
                    }
                }
            }
            else if (maxv > ss->sum[p])
            {
                int tmp = ss->Left[ss->sum[p]];
                ss->Cut(p);
                if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
                ss->sum[p] = maxv;
                ss->Link(p, tmp);
                assert(p == ss->Find(_v));
                if(ss->e[p]==nullptr){
                    ss->e[p] = new Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>();   
                }
                auto es = ss->e[p];
                bool emon = false;
                int ep = es->Find(_u);
                if (ep) emon = true;

                if(!emon){
                    #ifdef WEIGHT
                    if (edge_maxv > es->Getmin() || es->tot < edge_top_k)
                    #else
                    if (edge_maxv - (es->Getmin()) == 1 || es->tot < edge_top_k)
                    #endif
                    {
                        int i = es->Getid();
                        es->Add2(es->Location(_u), i);
                        es->vertex[i] = _u;
                        es->sum[i] = edge_maxv;
                        es->tmp[i] = edge_maxv;
                        es->Link(i, 0);
                        while (es->tot > edge_top_k)
                        {
                            int t = es->Right[0];
                            int tmp = es->head[t];
                            es->Cut(es->head[t]);
                            es->Recycling(tmp);
                        }
                    }
                }
                else if (edge_maxv > es->sum[ep])
                {
                    int tmp = es->Left[es->sum[ep]];
                    es->Cut(ep);
                    if (es->head[es->sum[ep]]) tmp = es->sum[ep];
                    es->sum[ep] = edge_maxv;
                    es->Link(ep, tmp);
                }
            }
        }

        struct Node { VERTEX_ID_TYPE x; int y; int thre;Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* e=nullptr;}; 
        static int Cmp(Node i, Node j) { return i.y > j.y; }
        void QueryEdge(Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>& v,Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* e,int thres = 0){
            if (e==nullptr){
                return;
            }
            unordered_map<int, int> mp;
            int jump=0, x_num=0;
            std::vector<Node> q;
            q.resize(MAX_EDGE_MEM+10);
            int CNT = 0;
            for (int i = N_E; i && i!=e->Left[i]; i = e->Left[i]){
                for (int j = e->head[i]; j && j!=e->Next[j]; j = e->Next[j])
                {
                    q[CNT].x = e->vertex[j]; q[CNT].y = e->sum[j]; 
                    q[CNT].thre=e->sum[j]-e->tmp[j];
                    CNT++;
                    mp[(*hfunc[0])((unsigned char*)(&(e->vertex[j])),sizeof(e->vertex[j])) >> 24]++;
                }
            }
            sort(q.begin(), q.begin() + CNT, Cmp);
            if constexpr (mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
                auto max_index = std::min(edge_top_k,CNT);
                for(int k = 0;k+jump<max_index;k++){
                    while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                        x_num++;
                        jump++;
                    }
                    v.Insert(q[k+jump].x,q[k+jump].y);
                }
            }else{
                for(int k = 0;k+jump<CNT  ;k++){
                    while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                        x_num++;
                        jump++;
                    }
                    if(q[k+jump].y>=thres){
                        v.Insert(q[k+jump].x,q[k+jump].y);
                    }
                }
            }
        }
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query()
        {
            unordered_map<int, int> mp;
            int jump=0, x_num=0;
            std::vector<Node> q;
            q.resize(MAX_MEM+10);
            std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> res;
            int CNT = 0;
            for (int i = N; i; i = ss->Left[i]){
                for (int j = ss->head[i]; j; j = ss->Next[j])
                {
                    q[CNT].x = ss->vertex[j]; q[CNT].y = ss->sum[j]; 
                    q[CNT].thre=ss->sum[j]-ss->tmp[j];
                    q[CNT].e = ss->e[j];
                    CNT++;
                    mp[(*hfunc[0])((unsigned char*)(&(ss->vertex[j])),sizeof(ss->vertex[j])) >> 24]++;
                }
            }
            sort(q.begin(), q.begin() + CNT, Cmp);
            auto max_index = std::min(vertex_top_k,CNT);
            for(int k = 0;k+jump<max_index;k++){
                while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                    x_num++;
                    jump++;
                }
                Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE> elem(q[k+jump].x,0);
                QueryEdge(elem,q[k+jump].e,edge_frequency_heavy_threshold);
                elem._sum = q[k+jump].y;
                if constexpr (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK ){
                    if(elem._sum >= vertex_frequency_heavy_threshold){
                        res.emplace_back(elem);
                    }
                }else{
                    res.emplace_back(elem);
                }
            }
            return res;
        }
};

#endif