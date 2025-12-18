# Task 7: I/O System Calls - Performance Testing Results

## Assignment Requirements

This assignment tests I/O performance using multiple threads to read/write from a file. The program creates P threads to perform parallel I/O operations on N bytes of data, testing two different access patterns:
- **List1**: Sequential access with 16KB chunks
- **List2**: Random access with 128-byte chunks

## Test Results

### Test 1: N=16,000,000 bytes, P=1 thread

```
$ ./hw4_io_perf 16000000 1
List1: Write 15990784 bytes, use 1 threads, elapsed time 0.008412 s, write bandwidth: 1812.89 MB/s
List1: Read 15990784 bytes, use 1 threads, elapsed time 0.002206 s, read bandwidth: 6912.96 MB/s
List2: Write 16000000 bytes, use 1 threads, elapsed time 0.569697 s, write bandwidth: 26.78 MB/s
List2: Read 16000000 bytes, use 1 threads, elapsed time 0.072147 s, read bandwidth: 211.50 MB/s
```

**Analysis:**
- Sequential operations (List1) are extremely fast with single thread
- Sequential read is 3.8x faster than sequential write (6912.96 vs 1812.89 MB/s)
- Random operations (List2) are dramatically slower
- Sequential access is 67.7x faster than random for writes (1812.89 vs 26.78 MB/s)
- Sequential access is 32.7x faster than random for reads (6912.96 vs 211.50 MB/s)

---

### Test 2: N=16,000,000 bytes, P=2 threads

```
$ ./hw4_io_perf 16000000 2
List1: Write 15990784 bytes, use 2 threads, elapsed time 0.009009 s, write bandwidth: 1692.75 MB/s
List1: Read 15990784 bytes, use 2 threads, elapsed time 0.001489 s, read bandwidth: 10241.77 MB/s
List2: Write 16000000 bytes, use 2 threads, elapsed time 0.463869 s, write bandwidth: 32.89 MB/s
List2: Read 16000000 bytes, use 2 threads, elapsed time 0.044349 s, read bandwidth: 344.06 MB/s
```

**Analysis:**
- Sequential write slightly decreased with 2 threads: 1692.75 MB/s (down 6.6% from 1 thread)
- Sequential read significantly improved: 10241.77 MB/s (up 48.2% from 1 thread)
- Random write improved: 32.89 MB/s (up 22.8% from 1 thread)
- Random read dramatically improved: 344.06 MB/s (up 62.7% from 1 thread)

---

### Test 3: N=16,000,000 bytes, P=4 threads

```
$ ./hw4_io_perf 16000000 4
List1: Write 15990784 bytes, use 4 threads, elapsed time 0.007808 s, write bandwidth: 1953.12 MB/s
List1: Read 15990784 bytes, use 4 threads, elapsed time 0.001294 s, read bandwidth: 11785.16 MB/s
List2: Write 16000000 bytes, use 4 threads, elapsed time 0.591634 s, write bandwidth: 25.79 MB/s
List2: Read 16000000 bytes, use 4 threads, elapsed time 0.030437 s, read bandwidth: 501.32 MB/s
```

**Analysis:**
- Sequential write with 4 threads: 1953.12 MB/s (best write performance observed, 7.7% better than 1 thread)
- Sequential read continues to improve: 11785.16 MB/s (up 70.5% from 1 thread, 15.1% from 2 threads)
- Random write performance degraded: 25.79 MB/s (down 3.7% from 1 thread, down 21.6% from 2 threads)
- Random read continues to improve: 501.32 MB/s (up 137.0% from 1 thread, up 45.7% from 2 threads)

---

### Test 4: N=160,000,000 bytes, P=2 threads (10x more data)

```
$ ./hw4_io_perf 160000000 2
List1: Write 159989760 bytes, use 2 threads, elapsed time 0.069160 s, write bandwidth: 2206.16 MB/s
List1: Read 159989760 bytes, use 2 threads, elapsed time 0.013270 s, read bandwidth: 11497.97 MB/s
List2: Write 160000000 bytes, use 2 threads, elapsed time 3.474827 s, write bandwidth: 43.91 MB/s
List2: Read 160000000 bytes, use 2 threads, elapsed time 0.307651 s, read bandwidth: 495.98 MB/s
```

**Analysis:**
- With 10x more data, sequential operations scale well
- Sequential write: 2206.16 MB/s (30.3% better than Test 2 with same 2 threads)
- Sequential read: 11497.97 MB/s (similar to Test 2, 12.3% improvement)
- Random write: 43.91 MB/s (33.5% better than Test 2)
- Random read: 495.98 MB/s (44.2% better than Test 2)
- Larger dataset shows better performance due to amortized overhead

---

## Summary Table

| Test | N (bytes) | P (threads) | Seq Write (MB/s) | Seq Read (MB/s) | Rand Write (MB/s) | Rand Read (MB/s) |
|------|-----------|-------------|------------------|-----------------|-------------------|------------------|
| 1    | 16M       | 1           | 1812.89          | 6912.96         | 26.78             | 211.50           |
| 2    | 16M       | 2           | 1692.75          | 10241.77        | 32.89             | 344.06           |
| 3    | 16M       | 4           | 1953.12          | 11785.16        | 25.79             | 501.32           |
| 4    | 160M      | 2           | 2206.16          | 11497.97        | 43.91             | 495.98           |

---

## Key Observations and Explanations

### 1. Sequential vs Random Access Performance

**Observation:** Sequential access is dramatically faster than random access across all configurations.

**Performance Ratios (Sequential/Random):**
- Test 1 (1 thread): Write 67.7x faster, Read 32.7x faster
- Test 2 (2 threads): Write 51.5x faster, Read 29.8x faster
- Test 3 (4 threads): Write 75.7x faster, Read 23.5x faster
- Test 4 (2 threads, 10x data): Write 50.2x faster, Read 23.2x faster

**Explanation:**
1. **Disk seek overhead**: Random access requires the disk head to move between different locations, adding significant latency
2. **Prefetching benefits**: Sequential access allows the OS to prefetch data, loading blocks ahead of time
3. **Cache effectiveness**: Sequential patterns benefit more from CPU and disk caches due to spatial locality
4. **Request coalescing**: The I/O scheduler can merge sequential requests into larger, more efficient operations
5. **Read-ahead**: The kernel's read-ahead mechanism predicts sequential patterns and loads data proactively

### 2. Read vs Write Performance

**Observation:** Read operations consistently outperform write operations.

**Read/Write Ratios:**
- Sequential (Test 1): 3.8x faster
- Sequential (Test 2): 6.0x faster
- Sequential (Test 3): 6.0x faster
- Sequential (Test 4): 5.2x faster
- Random (Test 1): 7.9x faster
- Random (Test 2): 10.5x faster
- Random (Test 3): 19.4x faster
- Random (Test 4): 11.3x faster

**Explanation:**
1. **Page cache**: The OS page cache serves read requests from memory after the first read
2. **Write-through overhead**: Writes must eventually reach physical storage and update metadata
3. **Journaling overhead**: File systems with journaling (like ext4) must log writes, adding overhead
4. **No synchronization required**: Reads don't need to wait for durability guarantees
5. **Write buffering**: Although writes are buffered, they must still be flushed to disk eventually

### 3. Impact of Thread Count

**Observation:** More threads improve read performance but have mixed effects on writes.

**Threading Effects:**

**Sequential Read Performance:**
- 1 thread: 6912.96 MB/s (baseline)
- 2 threads: 10241.77 MB/s (+48.2%)
- 4 threads: 11785.16 MB/s (+70.5%)

**Sequential Write Performance:**
- 1 thread: 1812.89 MB/s (baseline)
- 2 threads: 1692.75 MB/s (-6.6%)
- 4 threads: 1953.12 MB/s (+7.7%)

**Random Read Performance:**
- 1 thread: 211.50 MB/s (baseline)
- 2 threads: 344.06 MB/s (+62.7%)
- 4 threads: 501.32 MB/s (+137.0%)

**Random Write Performance:**
- 1 thread: 26.78 MB/s (baseline)
- 2 threads: 32.89 MB/s (+22.8%)
- 4 threads: 25.79 MB/s (-3.7%)

**Explanation:**
1. **Read parallelism benefits**: Multiple threads can issue concurrent read requests, allowing better I/O queue depth
2. **OS I/O scheduler optimization**: Modern schedulers can reorder and optimize multiple concurrent requests
3. **Write serialization**: Writes to the same file may require more serialization to maintain consistency
4. **Lock contention**: File descriptor operations require kernel locks, causing contention with many threads
5. **Context switching overhead**: Too many threads (4 for random writes) can introduce overhead that outweighs benefits
6. **I/O queue depth**: Multiple threads increase queue depth, allowing the storage device to optimize operations

### 4. Effect of Dataset Size

**Observation:** Larger datasets (160M vs 16M) show improved performance, especially for writes.

**Comparing Test 2 (16M, 2 threads) vs Test 4 (160M, 2 threads):**
- Sequential write: 1692.75 → 2206.16 MB/s (+30.3%)
- Sequential read: 10241.77 → 11497.97 MB/s (+12.3%)
- Random write: 32.89 → 43.91 MB/s (+33.5%)
- Random read: 344.06 → 495.98 MB/s (+44.2%)

**Explanation:**
1. **Amortized overhead**: Fixed costs (thread creation, file opening) are spread over more work
2. **Better I/O scheduling**: Larger request queues give the scheduler more optimization opportunities
3. **Reduced edge effects**: Startup and shutdown overhead becomes proportionally smaller
4. **Sustained throughput**: The system reaches peak sustained performance rather than burst performance
5. **Cache warming**: The page cache and prefetch mechanisms become more effective over longer runs

### 5. Random Access Scaling

**Observation:** Random read performance benefits significantly from threading, while random write shows diminishing returns.

**Random Read Scaling (16M dataset):**
- 1 thread: 211.50 MB/s
- 2 threads: 344.06 MB/s (+62.7%)
- 4 threads: 501.32 MB/s (+137.0% from 1 thread)

**Random Write Scaling (16M dataset):**
- 1 thread: 26.78 MB/s
- 2 threads: 32.89 MB/s (+22.8%)
- 4 threads: 25.79 MB/s (-3.7% from baseline)

**Explanation:**
1. **Read request reordering**: Multiple random read requests can be reordered by the I/O scheduler for better efficiency
2. **Write dependencies**: Random writes may have ordering constraints or require metadata updates that limit parallelism
3. **Elevator algorithm**: Disk schedulers can optimize read request ordering to minimize seek time
4. **Write amplification**: Random writes may trigger metadata updates and journaling, causing more actual I/O than requested
5. **Thread overhead threshold**: For writes, the overhead of 4 threads exceeds the benefits for this workload size

---

## Conclusions

1. **Access pattern is the dominant factor**: Sequential access is 23-76x faster than random access, far outweighing any threading benefits

2. **Threading helps reads more than writes**: Read operations scale well with threads (up to 137% improvement), while writes show modest or negative scaling

3. **Optimal thread count varies by operation**:
   - Sequential reads: 4 threads showed best performance
   - Random reads: 4 threads showed best performance
   - Sequential writes: 1-4 threads all performed similarly
   - Random writes: 2 threads optimal, 4 threads degraded performance

4. **Larger datasets perform better**: 10x data size showed 12-44% better bandwidth, indicating fixed overhead amortization

5. **System design implications**:
   - Prioritize sequential access patterns when possible
   - Use multiple threads for read-heavy workloads
   - Be conservative with threads for write operations
   - Larger I/O operations amortize overhead better
   - The OS I/O scheduler and page cache are highly effective

6. **Real-world application**:
   - Database systems should optimize for sequential scans where possible
   - Applications should batch small random operations into larger sequential ones
   - Read-heavy applications benefit from moderate parallelism (2-4 threads)
   - Write-heavy applications should carefully tune thread count to avoid overhead
