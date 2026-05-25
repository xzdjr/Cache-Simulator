#include "repl_policy.h"
// =========================================================
// TODO: Task 1 / Task 3 replacement policies
// Implement LRU first, then extend with SRRIP / BIP.
// =========================================================

void LRUPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    set[way].last_access = cycle;
    // TODO: mark the hit line as most recently used.
}

void LRUPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    set[way].last_access = cycle;
    // TODO: initialize a newly inserted line as MRU.
}

int LRUPolicy::getVictim(std::vector<CacheLine>& set) {
    int index = 0;
    uint64_t min = UINT64_MAX;
    for(size_t i = 0; i<set.size();i++){
        if(set[i].last_access<min){
            index = i;
            min = set[i].last_access;
        }
    }
    return index;
}

void SRRIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    set[way].rrpv = 0;
    // TODO: typically promote the line to RRPV=0.
}

void SRRIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    set[way].rrpv = 3;
    // TODO: insert with a long re-reference interval, e.g. RRPV=2.
}

int SRRIPPolicy::getVictim(std::vector<CacheLine>& set) {
    while(true){
        for(size_t i = 0;i<set.size();i++){
            if (set[i].rrpv == 3){
                return i;
            }
        }
        for(size_t i = 0;i<set.size();i++){
            set[i].rrpv++;
        }
    }
    // TODO: search for RRPV==3, otherwise age all lines and retry.
}

void BIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    set[way].last_access = cycle;
    // TODO: hits still become MRU.
}

void BIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    insertion_counter++;
    if(insertion_counter>=throttle){
        insertion_counter = 0;
        set[way].last_access = cycle;
    }else{
        set[way].last_access = 0;
    }
    // TODO: mostly insert at LRU position, but occasionally insert at MRU.
    // Hint: use insertion_counter and throttle.
}

int BIPPolicy::getVictim(std::vector<CacheLine>& set) {
    int index = 0;
    uint64_t min = UINT64_MAX;
    for(size_t i = 0; i<set.size();i++){
        if(set[i].last_access<min){
            index = i;
            min = set[i].last_access;
        }
    }
    return index;
}

ReplacementPolicy* createReplacementPolicy(std::string name) {
    if (name == "SRRIP") return new SRRIPPolicy();
    if (name == "BIP") return new BIPPolicy();
    return new LRUPolicy();
}
