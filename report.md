# Report

1. Team member:Dong Jiarui(124090102), Yang Xihua(124090797)
chosen seed information:124090102.

2. **Implementation Summary**
* **Task 1**: Implemented the core cache access logic, including address decomposition (offset, index, tag), hit/miss detection, and the LRU replacement policy. Handled write-back and write-allocate behaviors.

* **Task 2**: Extended the memory hierarchy by instantiating an L2 cache and connecting it between L1 and Main Memory. Implemented recursive access logic so that L1 misses are served by L2, and L2 misses are served by Main Memory.

* **Task 3**: Implemented advanced replacement policies (BIP and SRRIP) and a Stride Prefetcher. Conducted an extensive grid search (36 combinations) to optimize the AMAT for a personalized trace.

3. **Address Mapping Explanation**
* 1. **Block Offset**: Determined by the block size. 
      Offset bits=log2​(block size)
            These bits identify the specific byte within a cache line.

  2. **Set Index**: Determined by the number of sets. 
      Index bits=log2​(number of sets)
             These bits determine which set the address maps to.

  3. **Tag**: The remaining upper bits of the address. The tag is used to uniquely identify the memory block within the selected set.

* When the cache geometry changes , the number of offset and index bits will shift, thereby changing how the address is mapped to the cache.

4. **Task 1 Testing**

  ```
  make task1
  ./cache_sim trace_sanity.txt 16 4 64 1 100
  ./cache_sim trace_sanity.txt 32 4 32 1 100
  ./cache_sim trace_sanity.txt 64 8 128 1 100
  ```
the representative results
```
./cache_sim trace_sanity.txt 32 8 64 1 100
Constructed L1: 32KB, 8-way, 1cyc, [LRU + None]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 21.43% (Access: 56, Miss: 44, WB: 2)
      Prefetches Issued: 0
  [Main Memory] Total Accesses: 46

Metrics:
  Total Instructions: 56
  Total Cycles:       4456
  AMAT:               79.57 cycles
```
```
./cache_sim trace_sanity.txt 64 8 128 1 100
Constructed L1: 64KB, 8-way, 1cyc, [LRU + None]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 60.71% (Access: 56, Miss: 22, WB: 0)
      Prefetches Issued: 0
  [Main Memory] Total Accesses: 22

Metrics:
  Total Instructions: 56
  Total Cycles:       2256
  AMAT:               40.29 cycles
```

5. **Task 2 Hierarchy Explanation**
* how L1, L2, and memory interact: When the CPU needs data, it first checks the extremely fast but small L1 cache; if a miss occurs, it queries the larger, slower L2 cache; if missed again, it finally requests from the slow Main Memory. Once retrieved, the data block is allocated upwards into L2 and L1 to ensure lightning-fast access next time, functioning as a hierarchical filtering system that perfectly balances speed and capacity
* changes after adding L2: The addition of an L2 cache acts as a high-capacity intermediate buffer, drastically reducing the severe latency penalty of L1 misses by intercepting the vast majority of requests that would otherwise go to the slow main memory.
* results of task2
```
./cache_sim trace_sanity.txt 32 8 64 1 100 --enable-l2
Constructed L2: 128KB, 8-way, 4cyc, [LRU + None]
Constructed L1: 32KB, 8-way, 1cyc, [LRU + None]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 21.43% (Access: 56, Miss: 44, WB: 2)
      Prefetches Issued: 0
  [L2] Hit Rate: 50.00% (Access: 46, Miss: 23, WB: 0)
      Prefetches Issued: 0
  [Main Memory] Total Accesses: 23

Metrics:
  Total Instructions: 56
  Total Cycles:       2532
  AMAT:               45.21 cycles
``` 

6. **Task 3 Design Choices**
* We implemented LRU,BIP,SRRIP replacement policies. And NextLine,Stride prefetchers.
* No.

7. **Trace Analysis**
* what access patterns you observed in your personalized trace:
  1. Strong Sequentiality and Stable Strides: The accesses exhibit high regularity. A block stride of 1 (sequential) accounts for 60.91% of all accesses, while a stride of 64 accounts for 23.94%. There are also fixed backward jumps, such as -1088 and -576
  2. Phase-Based Behavior: The memory access is not uniformly distributed. For example, early windows like [1024, 2304] show broad scanning, but in windows [2560, 4096], unique blocks plummet to 14-40, indicating the program has entered a tight loop with intense short-term reuse.
  3. Severe Set-Conflict Hotspots: Due to the mapping of the 64-block stride, specific cache sets become extremely hot. Set 44 alone bears a disproportionate 20.93% of the total access pressure.

* How those patterns influenced your design decisions:
  1. Implementing a Stride Prefetcher: Given that strides of 1 and 64 account for over 84% of accesses, the data access is strongly regular. Therefore, introducing a Stride prefetcher is the most reasonable choice to accurately capture these constant strides and fetch data proactively.
  2. We may use LRU or BIP since some trace is Scan-heavy, but not all.We need to test. 

8. **Experimental Results**: We tried several choices and here is the result.
   
| num | L1 policy | L1 prefetcher | L2 policy | L2 prefetcher | AMAT (cycles) |
| :---: | :--- | :--- | :--- | :--- | :--- |
| 1 | LRU | NextLine | LRU | NextLine | 13.63 |
| 2 | LRU | NextLine | LRU | Stride | 3.40 |
| 3 | LRU | NextLine | BIP | NextLine | 29.99 |
| 4 | LRU | NextLine | BIP | Stride | 3.55 |
| 5 | LRU | NextLine | SRRIP | NextLine | 43.54 |
| 6 | LRU | NextLine | SRRIP | Stride | 3.62 |
| 7 | LRU | Stride | LRU | NextLine | 2.18 |
| 8 | LRU | Stride | LRU | Stride | <span style="background:#ffebee;color:#b71c1c;padding:2px 4px;">1.68</span> |
| 9 | LRU | Stride | BIP | NextLine | 3.06 |
| 10 | LRU | Stride | BIP | Stride | 1.86 |
| 11 | LRU | Stride | SRRIP | NextLine | 3.06 |
| 12 | LRU | Stride | SRRIP | Stride | 1.88 |
| 13 | BIP | NextLine | LRU | NextLine | 13.72 |
| 14 | BIP | NextLine | LRU | Stride | 3.45 |
| 15 | BIP | NextLine | BIP | NextLine | 31.96 |
| 16 | BIP | NextLine | BIP | Stride | 3.64 |
| 17 | BIP | NextLine | SRRIP | NextLine | 33.26 |
| 18 | BIP | NextLine | SRRIP | Stride | 3.75 |
| 19 | BIP | Stride | LRU | NextLine | 2.13 |
| 20 | BIP | Stride | LRU | Stride | 1.73 |
| 21 | BIP | Stride | BIP | NextLine | 2.80 |
| 22 | BIP | Stride | BIP | Stride | 2.01 |
| 23 | BIP | Stride | SRRIP | NextLine | 2.90 |
| 24 | BIP | Stride | SRRIP | Stride | 2.03 |
| 25 | SRRIP | NextLine | LRU | NextLine | 14.07 |
| 26 | SRRIP | NextLine | LRU | Stride | 3.84 |
| 27 | SRRIP | NextLine | BIP | NextLine | 33.71 |
| 28 | SRRIP | NextLine | BIP | Stride | 4.06 |
| 29 | SRRIP | NextLine | SRRIP | NextLine | 37.70 |
| 30 | SRRIP | NextLine | SRRIP | Stride | 4.12 |
| 31 | SRRIP | Stride | LRU | NextLine | 2.40 |
| 32 | SRRIP | Stride | LRU | Stride | 1.84 |
| 33 | SRRIP | Stride | BIP | NextLine | 3.76 |
| 34 | SRRIP | Stride | BIP | Stride | 2.02 |
| 35 | SRRIP | Stride | SRRIP | NextLine | 4.36 |
| 36 | SRRIP | Stride | SRRIP | Stride | 2.02 |


9. **Discussion on Best Configuration:**  
* The optimal configuration (L1: LRU+Stride, L2: LRU+Stride) achieved an AMAT of 1.68 cycles.

* Why it performs well:
   1. Stride Prefetching: As analyzed in the trace, the workload is dominated by fixed strides (stride 1 and 64). The Stride Prefetcher effectively eliminates most compulsory and capacity misses by fetching blocks before they are requested.
   2.  LRU vs BIP/SRRIP: While BIP and SRRIP are generally better for scan-heavy workloads, our trace contains a "tight loop" phase (unique blocks drop to 14-40). In this phase, strong temporal reuse occurs, and LRU is the most efficient policy for capturing this recency.
* Potential Failure Cases:  
This design might fail if the workload changes to a completely random access pattern (where prefetching would cause cache pollution) or a massive streaming scan that exceeds the L2 capacity (where BIP/SRRIP would be superior to LRU).



10. **External Resources and AI Usage**
https://www.doubao.com/thread/w5449a29f155a0b73

