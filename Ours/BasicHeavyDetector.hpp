#ifndef _BASICHEAVYDETECTOR_H
#define _BASICHEAVYDETECTOR_H

#include "DetectorAbstract.h"
#include "GraphFilter.hpp"
#include "hashTable.h"

#pragma pack(1)
#define OVERFLOW_DETECT
#define OVERFLOW_CORRECT

using VERTEX_ID_TYPE = uint32_t;
using FP_TYPE = uint8_t;
using WEIGHT_TYPE = uint32_t;
std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> _answer;

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE,typename FP_TYPE>
class BasicBubbleArrays{
    class EdgeEntry{
        public:
            bool IsEmpty(){return _count == 0;}
            EdgeEntry(){};
            EdgeEntry(VERTEX_ID_TYPE u):_u(u){};
            EdgeEntry(VERTEX_ID_TYPE u,WEIGHT_TYPE count):_u(u),_count(count){};
            bool Equal(VERTEX_ID_TYPE u){return u == _u;}
            void Insert(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                auto prev_count = _count;
                #endif
                _count+=weight;
                #ifdef OVERFLOW_CORRECT
                if(_count < prev_count){
                    _count=0;
                    _count--;
                }
                #endif
                #ifdef OVERFLOW_DETECT
                assert(_count>=prev_count);
                #endif
            }
            void InsertId(VERTEX_ID_TYPE u,WEIGHT_TYPE weight){_u = u;_count = weight;}
            bool operator>(const EdgeEntry &e) { return _count > e._count; }
            void Clear(){_u = 0;_count = 0;}
            void Loss(WEIGHT_TYPE weight){_count-=weight;}
            VERTEX_ID_TYPE GetId(){return _u;}
            WEIGHT_TYPE GetCount(){return _count;}
            EdgeEntry& operator=(const EdgeEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_u = other._u;
                return *this;
            }
        public:
            VERTEX_ID_TYPE _u = 0;
            WEIGHT_TYPE _count = 0;
    };
    class VertexEntry{
        public:
            VertexEntry(){_edge_entries.resize(edge_bucket_size);};
            VertexEntry(VERTEX_ID_TYPE v):_v(v){_edge_entries.resize(edge_bucket_size);};
            VertexEntry(VERTEX_ID_TYPE v,WEIGHT_TYPE count):_v(v),_count(count){_edge_entries.resize(edge_bucket_size);};
            void SetId(VERTEX_ID_TYPE id){_v = id;assert(_answer.find(this->_v)!=_answer.end());}
            bool IsEmpty()const{return _count == 0;}
            bool Equal(VERTEX_ID_TYPE v){return v == _v;}
            WEIGHT_TYPE GetCount(){return _count;}
            const std::vector<EdgeEntry> GetEntry(){return this->_edge_entries;}
            VertexEntry& operator=(const VertexEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_v = other._v;
                this->_edge_entries = other._edge_entries;
                return *this;
            }
            void Insert(VERTEX_ID_TYPE u, WEIGHT_TYPE weight){
                    #ifdef OVERFLOW_DETECT
                    auto prev_count = _count;
                    #endif
                    _count+=weight;
                    #ifdef OVERFLOW_CORRECT
                    if(_count < prev_count){
                        _count=0;
                        _count--;
                    }
                    #endif
                    #ifdef OVERFLOW_DETECT
                    assert(_count>=prev_count);
                    #endif
                    assert(_count!=0);
                    EdgeInsert(u,weight);
            }
            void EdgeInsert(VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight){
                Check();
                if(_answer.find(this->_v)!=_answer.end()){
                    assert(_answer[this->_v]._edges.find(_u)!=_answer[this->_v]._edges.end());
                }else{
                    assert(false);
                }
                int index = -1;
                for(int i = 0;i<edge_bucket_size;i++){
                    if(_edge_entries[i].Equal(_u)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertId(_u,_weight);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    Check();
                    return;
                }
                if(_weight > _edge_entries[edge_bucket_size-1].GetCount()){
                    _edge_entries[edge_bucket_size-1].InsertId(_u,_weight);
                    index = edge_bucket_size-1;
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    Check();
                    return;
                }else{
                    _edge_entries[edge_bucket_size-1].Loss(_weight);
                }
                Check();
            }
            void Loss(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                assert(_count>=weight);
                #endif
                _count-=weight;
            }
            VERTEX_ID_TYPE GetId(){return _v;}
            void Clear(){
                _count = 0;
                _v = 0;
                for(auto& elem:this->_edge_entries){
                    elem.Clear();
                }
            }
            void Check()const{
                return;
                if(!this->IsEmpty()){
                    if(_answer.find(this->_v)!=_answer.end()){
                        for(auto elem:_edge_entries){
                            if(!elem.IsEmpty()){
                                assert(_answer[this->_v]._edges.find(elem._u)!=_answer[this->_v]._edges.end());
                            }
                        }
                    }else{
                        assert(false);
                    }
                }
            }
        private:
            VERTEX_ID_TYPE _v = 0;
            WEIGHT_TYPE _count = 0;
            std::vector<EdgeEntry> _edge_entries;
    };
    class VertexBucket{
        public:
            VertexBucket(){
                _entries.resize(vertex_bucket_size);
            }
            bool Equal(int index, VERTEX_ID_TYPE id) {
                return _entries[index].Equal(id);
            }
            void Insert(int index,const VertexEntry& entry){
                entry.Check();
                this->_entries[index] = entry;
                this->_entries[index].Check();
            }
            void Insert(int index,VERTEX_ID_TYPE u,WEIGHT_TYPE weight){
                this->_entries[index].Check();
                _entries[index].Insert(u,weight);
                this->_entries[index].Check();
            }
            VertexEntry& GetEntry(int index){
                return _entries[index];
            }
            WEIGHT_TYPE GetEntryCount(int index){
                return _entries[index].GetCount();
            }
            bool IsEmpty(int index){
                return _entries[index].IsEmpty();
            }
            void SetId(int index,VERTEX_ID_TYPE id){
                _entries[index].SetId(id);
            }
            void BubbleSort(int index){
                while(true){
                    if(index <= 0 ){
                        break;
                    }
                    if(_entries[index].GetCount() <= _entries[index-1].GetCount()){
                        break;
                    }
                    std::swap(_entries[index],_entries[index-1]);
                    index--;
                }
            }
            void DownStairs(int index){
                int cur_index = vertex_bucket_size-1;
                _entries[cur_index].Clear();
                if(cur_index<=index){
                    return;
                }
                while(cur_index > index){
                    _entries[cur_index].Check();
                    _entries[cur_index-1].Check();
                    _entries[cur_index] = _entries[cur_index-1];
                    _entries[cur_index].Check();
                    _entries[cur_index-1].Check();
                    cur_index--;
                }
            }
            void Remove(int index){
                _entries[index].Clear();
            }
            void Clear(int index){
                this->_entries[index].Clear();
            }
            std::vector<VertexEntry> _entries;
    };
    private:
        std::vector<std::vector<VertexBucket>> _buckets;
        int _topk;
        int _f_max;
        int _thres;
        int _bucket_num;
        static uint32_t Hash(VERTEX_ID_TYPE id){return (*hfunc[1])((unsigned char*)(&id), sizeof(id));}
        static uint32_t Hash(uint8_t fp){return (*hfunc[1])((unsigned char*)(&fp), sizeof(fp));} 
        static uint32_t HashFp(VERTEX_ID_TYPE id){return (*hfunc[0])((unsigned char*)(&id), sizeof(id));}
        template<typename V, typename W, typename F>
        friend class  BasicHeavyDetector;
        const static FP_TYPE fp_mask = ((uint64_t)1<<FP_BITS)-1;
    public:
        BasicBubbleArrays(int bucket_num,int thres,int topk){
            _buckets.resize(BUBBLE_ARRAY_SIZE,std::vector<VertexBucket>(bucket_num));
            _f_max = 0;
            _thres = thres;
            _topk = topk;
            _bucket_num = bucket_num;
        }
        bool Check(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1 
            uint32_t hash_value[2] = {hash_key, hash_key + (fp&fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            for(int i = 0;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    bucket0.Insert(i,_u,_weight);
                    if(i == 0 && bucket0.GetEntryCount(0)>_f_max){
                        _f_max = bucket0.GetEntryCount(0);
                        _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                        return true;
                    }
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return true;
                }
            }
            for(int i = 0;i<vertex_bucket_size;i++){
                if(bucket1.Equal(i,_v)){
                    bucket1.Insert(i,_u,_weight);
                    if(i == 0 && bucket1.GetEntryCount(0)>_f_max){
                        _f_max = bucket1.GetEntryCount(0);
                        _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                        return true;
                    }
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return true;
                }
            }
            return false;
        }
        bool KickOut(int kick_num,uint32_t hash_value,VertexBucket& cur_bucket,int entry_index,int array_index){
            if(kick_num == 0){
                return false;
            }
            auto v = cur_bucket.GetEntry(entry_index).GetId();
            uint32_t fp = HashFp(v);
#ifdef TEST1
            uint32_t next_hash_value = hash_value;
            if (array_index == 0) {
                next_hash_value += (fp&fp_mask);
            } else {
                next_hash_value -= (fp&fp_mask);
            }
#else
            uint32_t next_hash_value = hash_value ^ Hash(static_cast<uint8_t>(fp & fp_mask));
#endif 
            VertexBucket &next_bucket = _buckets[1 - array_index][next_hash_value % _bucket_num];
            if (cur_bucket.GetEntryCount(entry_index) >
                next_bucket.GetEntryCount(0)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index));
                return true;
            }

            if (KickOut(kick_num - 1, next_hash_value, next_bucket, 0,
                        1 - array_index)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index));
                return true;
            }
            return false;
        }      
        void Insert(VERTEX_ID_TYPE _v,VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight){
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1
            uint32_t hash_value[2] = {hash_key, hash_key + (fp & fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            if(bucket0.Equal(0,_v)){
                bucket0.Insert(0,_u,_weight);
                if(bucket0.GetEntryCount(0)>_f_max){
                    _f_max = bucket0.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket0.IsEmpty(0)){
                bucket0.Clear(0);
                bucket0.SetId(0,_v);
                bucket0.Insert(0,_u,_weight);
                return;
            }
            if(bucket1.Equal(0,_v)){
                bucket1.Insert(0,_u,_weight);
                if(bucket1.GetEntryCount(0)>_f_max){
                    _f_max = bucket1.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket1.IsEmpty(0)){
                bucket1.Clear(0);
                bucket1.SetId(0,_v);
                bucket1.Insert(0,_u,_weight);
                return;
            }
            for(int i = 1;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    bucket0.Insert(i,_u,_weight);
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket0.IsEmpty(i)) {
                    bucket0.Clear(i);
                    bucket0.SetId(i,_v);
                    bucket0.Insert(i,_u,_weight);
                    return;
                }
                if(bucket1.Equal(i,_v)){
                    bucket1.Insert(i,_u,_weight);
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket1.IsEmpty(i)) {
                    bucket1.Clear(i);
                    bucket1.SetId(i,_v);
                    bucket1.Insert(i,_u,_weight);
                    return;
                }
            }
            if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                if (bucket0.GetEntryCount(vertex_bucket_size - 1) <
                    bucket1.GetEntryCount(vertex_bucket_size - 1)) {
                    if(bucket0.GetEntryCount(vertex_bucket_size-1)<_weight){
                        bucket0.Clear(vertex_bucket_size-1);
                        bucket0.SetId(vertex_bucket_size-1,_v);
                        bucket0.Insert(vertex_bucket_size-1,_u,_weight);
                        bucket0.BubbleSort(vertex_bucket_size-1);
                        if(bucket0.GetEntryCount(1)>_thres){
                            if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                                bucket0.Remove(1);
                            }
                        }
                    }else{
                        bucket0.GetEntry(vertex_bucket_size-1).Loss(_weight);
                    }
                } else {
                    if(bucket1.GetEntryCount(vertex_bucket_size-1)<_weight){
                        bucket1.Clear(vertex_bucket_size-1);
                        bucket1.SetId(vertex_bucket_size-1,_v);
                        bucket1.Insert(vertex_bucket_size-1,_u,_weight);
                        bucket1.BubbleSort(vertex_bucket_size-1);
                        if(bucket1.GetEntryCount(1)>_thres){
                            auto value = Hash(bucket1.GetEntry(1).GetId());
                            auto fp = HashFp(bucket1.GetEntry(1).GetId());
                            if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                                bucket1.Remove(1);
                            }
                        }
                    }else{
                        bucket1.GetEntry(vertex_bucket_size-1).Loss(_weight);
                    }
                }
            }else{
                if (bucket0.GetEntryCount(vertex_bucket_size - 1) <
                    bucket1.GetEntryCount(vertex_bucket_size - 1)) {
                    bucket0.GetEntry(vertex_bucket_size-1).Loss(1);
                }else{
                    bucket1.GetEntry(vertex_bucket_size-1).Loss(1);
                }
            }
        }
        void SetAnswer(const std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& answer){_answer = answer;}
};

template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE, typename FP_TYPE>
class BasicHeavyDetector:public DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
    private:
        GraphFilter<VERTEX_ID_TYPE,FP_TYPE,WEIGHT_TYPE>* _filter;
        BasicBubbleArrays<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>* _arrays;
    public:
        BasicHeavyDetector(int mem){
            auto filter_mem = mem * filterMemoryRate;
            auto array_mem = mem*(1-filterMemoryRate);
            int entryAmount = filter_mem / sizeof(typename GraphFilter<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>::GraphEntry);
            int width = static_cast<int>(sqrt(entryAmount * widthheightRate));
            int height = width / widthheightRate;
            assert(width* height <= entryAmount);
            _filter = new GraphFilter<VERTEX_ID_TYPE,FP_TYPE,WEIGHT_TYPE>(width,height);
            edge_bucket_size = edge_top_k;
            // assert(edge_bucket_size % vertex_bucket_size==0);
            auto bucket_num = array_mem / 2 / (vertex_bucket_size * (sizeof(WEIGHT_TYPE)+sizeof(VERTEX_ID_TYPE)+edge_bucket_size*(sizeof(WEIGHT_TYPE)+sizeof(VERTEX_ID_TYPE))));
            _arrays = new BasicBubbleArrays<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>(bucket_num,graph_thres,vertex_top_k); 
        }
        ~BasicHeavyDetector(){
            delete _filter;
            delete _arrays;
        }
        void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            if(_arrays->Check(_v,_u,_weight)){
                return;
            }
            if(_filter->Insert(_v,_u,_weight)){
                WEIGHT_TYPE sum = 0;
                auto res = _filter->NodeValQuery(_v,sum);
                if(sum >= freq_filter_thres){
                    _arrays->Insert(_v,_u,sum);
                    for(auto elem:res){
                        elem->Clear();
                    }
                    return;
                }
            }
            return;
        }
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query(){
            std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> res;
            for(int i = 0;i<BUBBLE_ARRAY_SIZE;i++){
                for(int j = 0;j<_arrays->_bucket_num;j++){
                    for(int k = 0;k<vertex_bucket_size;k++){
                        if(!_arrays->_buckets[i][j].GetEntry(k).IsEmpty()){
                            // _arrays->_buckets[i][j].GetEntry(k).Check();
                            Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE> elem(_arrays->_buckets[i][j].GetEntry(k).GetId(),_arrays->_buckets[i][j].GetEntry(k).GetCount());
                            for(auto edge:_arrays->_buckets[i][j].GetEntry(k).GetEntry()){
                                if(!edge.IsEmpty()){
                                    elem.Insert(edge.GetId(),edge.GetCount());
                                }
                            }
                            elem._sum = _arrays->_buckets[i][j].GetEntry(k).GetCount();
                            if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag ==  FREQ_VERTEX_TOPK_EDGE_HEAVY){
                                elem.QueryHeavy(edge_frequency_heavy_threshold);
                            }else{
                                elem.QueryTopK(edge_top_k);
                            }
                            res.emplace_back(elem);
                        }else{
                            break;
                        }
                    }
                }
            }
            std::sort(res.begin(),res.end(),[](auto const &a, auto const &b) {
                return a._sum > b._sum;
            });
            int max_index = 0;
            if (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
                for(auto elem:res){
                    auto thres = (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK)?vertex_frequency_heavy_threshold:vertex_degree_heavy_threshold;
                    if(elem._sum >= thres){
                        max_index++;
                    }else{
                        break;
                    }
                }
            }else{
                max_index = std::min(vertex_top_k,_arrays->_bucket_num*2*vertex_bucket_size);
            }
            return std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>(res.begin(),res.begin()+max_index);
        };
};

#endif