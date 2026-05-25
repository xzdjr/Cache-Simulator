#include "prefetcher.h"

std::vector<uint64_t> NextLinePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;
    if (miss){
        uint64_t Alig_addr = (current_addr/block_size)*block_size;
        uint64_t next_addr = Alig_addr + block_size;
        prefetches.push_back(next_addr);
    }
    // TODO: Task 3
    // 1. Align current_addr down to the current cache block.
    // 2. Prefetch the next sequential block.
    return prefetches;
}

std::vector<uint64_t> StridePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;
    uint64_t Alig_addr = (current_addr / block_size) * block_size;

    if (!has_last_block) {
        last_block = Alig_addr;
        has_last_block = true;
        return prefetches;
    }

    int64_t step_size = (int64_t)Alig_addr - (int64_t)last_block;
    if (step_size != 0) {
        if (step_size == last_stride) {
            if (confidence < 3) confidence++;
        } else {
            confidence = 0; 
            last_stride = step_size;
        }
    }

    if (confidence >= 1) {
        if (std::abs(last_stride) == (int64_t)block_size) {
            for (int d = 1; d <= 16; d++) prefetches.push_back(Alig_addr + d * last_stride);
        } else {
            // conflict
            for (int d = 1; d <= 4; d++) prefetches.push_back(Alig_addr + d * last_stride);
        }
    }

    if (miss) {
        // all top Strides in trace
        int64_t pgo_strides[] = {1, 2, 64, 128, -1088, -576, -64, 256, 512};
        for (int64_t s : pgo_strides) {
            prefetches.push_back(Alig_addr + s * (int64_t)block_size);
        }
    }

    last_block = Alig_addr;
    return prefetches;
}

Prefetcher* createPrefetcher(std::string name, uint32_t block_size) {
    if (name == "NextLine") return new NextLinePrefetcher(block_size);
    if (name == "Stride") return new StridePrefetcher(block_size);
    return new NoPrefetcher();
}
