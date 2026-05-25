#include "memory_hierarchy.h"
#include "prefetcher.h"
#include "repl_policy.h"
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;
static uint64_t set44_evictions = 0;
MainMemory::MainMemory(int lat) : latency(lat) {}

int MainMemory::access(uint64_t addr, char type, uint64_t cycle) {
    (void)addr;
    (void)type;
    (void)cycle;
    access_count++;
    return latency;
}

void MainMemory::printStats() {
    cout << "  [Main Memory] Total Accesses: " << access_count << endl;
}

CacheLevel::CacheLevel(string name, CacheConfig cfg, MemoryObject* next)
    : level_name(name), config(cfg), next_level(next) {
    policy = createReplacementPolicy(config.policy_name);
    prefetcher = createPrefetcher(config.prefetcher, config.block_size);

    uint64_t total_bytes = (uint64_t)config.size_kb * 1024;
    num_sets = total_bytes / (config.block_size * config.associativity);

    offset_bits = log2(config.block_size);
    index_bits = log2(num_sets);

    sets.resize(num_sets, vector<CacheLine>(config.associativity));

    cout << "Constructed " << level_name << ": "
         << config.size_kb << "KB, " << config.associativity << "-way, "
         << config.latency << "cyc, "
         << "[" << config.policy_name << " + " << prefetcher->getName() << "]" << endl;
}

CacheLevel::~CacheLevel() {
    delete policy;
    delete prefetcher;
}

uint64_t CacheLevel::get_index(uint64_t addr) {
    // TODO: Task 1
    // Compute the set index from the address.
    // Hint: remove block offset bits first, then keep only the index bits.
    uint64_t mask = (1<<index_bits) - 1;
    return ((addr >> offset_bits)& mask);
}

uint64_t CacheLevel::get_tag(uint64_t addr) {
    // TODO: Task 1
    // Compute the tag from the address.
    // Hint: shift away both block offset bits and set index bits.
    return addr >> (offset_bits + index_bits);
}

uint64_t CacheLevel::reconstruct_addr(uint64_t tag, uint64_t index) {
    // TODO: Task 1 / Task 2
    // Rebuild a block-aligned address from a tag and set index.
    // This helper is useful when writing back an evicted dirty line.
    uint64_t tag_con = tag<<(index_bits + offset_bits);
    uint64_t index_con = index<<offset_bits;
    return tag_con|index_con;
}

void CacheLevel::write_back_victim(const CacheLine& line, uint64_t index, uint64_t cycle) {
    // TODO: Task 1 / Task 2
    // Move dirty write-back logic into this helper.
    // Suggested steps:
    // 1. If the victim is not dirty, return immediately.
    // 2. If there is no next level, return immediately.
    // 3. Increment the write-back counter.
    // 4. Reconstruct the evicted block address from tag + index.
    // 5. Send a write access to the next level.
    if( line.dirty == 0 || next_level == NULL){return ;}//1,2
    write_backs++;//3
    uint64_t recon_addr = reconstruct_addr(line.tag, index);//4
    next_level->access(recon_addr, 'w', cycle);//5
}

int CacheLevel::access(uint64_t addr, char type, uint64_t cycle) {
    // TODO: Task 1
    // 1. Derive the address fields for the current cache geometry:
    //    - block offset bits
    //    - set index bits
    //    - tag bits
    // 2. Use the address to compute index/tag and select the set.
    // 3. Search all ways for a valid tag match.
    // 4. On hit:
    //    - increment hits
    //    - call policy->onHit(...)
    //    - update dirty bit for writes
    //    - clear is_prefetched if a prefetched line is consumed
    // 5. On miss:
    //    - increment misses
    //    - find an in(valid line or select a victim with policy->getVictim(...)
    //    - call write_back_victim(...) if the chosen victim is dirty
    //    - fetch the requested block from next_level and add that latency to lat
    //    - install the new cache line and call policy->onMiss(...)
    // 6. Your code should work correctly even if cache size, associativity,
    //    number of sets, or cache line size changes.
    // 7. Task 3: after demand access logic works, call the prefetcher here and
    //    install returned blocks through install_prefetch(...).
    uint64_t index = get_index(addr);
    uint64_t tag = get_tag(addr);
    bool is_hit = false;
    int way = -1;

    for(uint32_t i = 0; i < config.associativity; i++) {
        if(sets[index][i].valid && sets[index][i].tag == tag) {
            is_hit = true;
            way = i;
            break;
        }
    }
    //hit
    if (is_hit) {
        hits++;
        policy->onHit(sets[index], way, cycle);
        std::vector<uint64_t> pf_addrs = prefetcher->calculatePrefetch(addr, false);
        for (uint64_t pf_addr : pf_addrs) {
            install_prefetch(pf_addr, cycle);
            prefetch_issued++;
        }
        return config.latency;
    } else {
        misses++;
        int target_way = policy->getVictim(sets[index]);

        if (level_name == "L1" && index == 44 && sets[index][target_way].valid) {
            set44_evictions++;
        }

        if (sets[index][target_way].valid && sets[index][target_way].dirty) {
            write_back_victim(sets[index][target_way], index, cycle);
        }

        int lat = config.latency + next_level->access(addr, 'r', cycle);
        sets[index][target_way].tag = tag;
        sets[index][target_way].valid = true;
        sets[index][target_way].dirty = (type == 'w');
        policy->onMiss(sets[index], target_way, cycle);

        std::vector<uint64_t> pf_addrs = prefetcher->calculatePrefetch(addr, true);
        for (uint64_t pf_addr : pf_addrs) {
            install_prefetch(pf_addr, cycle);
            prefetch_issued++;
        }

        return lat;
    }
}

void CacheLevel::install_prefetch(uint64_t addr, uint64_t cycle) {
    // TODO: Task 3
    // Implement a prefetch fill path similar to the miss path in access(), but
    // treat prefetched lines as clean and mark is_prefetched = true.
    // If you evict a dirty victim during prefetch installation, reuse
    // write_back_victim(...) instead of duplicating that logic.
    uint64_t index = get_index(addr);
    uint64_t tag = get_tag(addr);
    for(size_t i = 0; i<sets[index].size();i++){
        if(sets[index][i].tag == tag && sets[index][i].valid){
            return ;
        }
    }
    int victim_way = policy->getVictim(sets[index]);

    if(sets[index][victim_way].valid && sets[index][victim_way].dirty){
        write_back_victim(sets[index][victim_way], index,cycle);
    }
    sets[index][victim_way].tag = tag;
    sets[index][victim_way].valid = true;
    sets[index][victim_way].dirty = false;
    sets[index][victim_way].is_prefetched = true;
    policy->onHit(sets[index], victim_way, cycle);
}

void CacheLevel::printStats() {
    uint64_t total = hits + misses;
    cout << "  [" << level_name << "] "
         << "Hit Rate: " << fixed << setprecision(2) << (total ? (double)hits / total * 100.0 : 0) << "% "
         << "(Access: " << total << ", Miss: " << misses << ", WB: " << write_backs << ")" << endl;
    cout << "      Prefetches Issued: " << prefetch_issued << endl;
}
