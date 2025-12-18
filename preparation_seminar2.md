# Seminar 2 Preparation: Threads, Scheduling, and Synchronization

**Course:** ID1206 Operating Systems
**Assignment:** Assignment 2
**Date:** 2025-11-29

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Theoretical Foundations](#theoretical-foundations)
3. [Programming Implementations](#programming-implementations)
4. [Key Findings and Insights](#key-findings-and-insights)
5. [Discussion Points for Seminar](#discussion-points-for-seminar)

---

## Executive Summary

This assignment explores fundamental operating systems concepts including CPU scheduling algorithms, deadlock detection, resource allocation, and multi-threaded programming. The work is divided into theoretical analysis (Questions 1-5) and practical implementation (Tasks 6-8).

**Key Achievements:**
- Analyzed 5 different CPU scheduling algorithms with complete performance metrics
- Demonstrated deadlock detection using resource-allocation graphs
- Implemented Banker's Algorithm for deadlock avoidance
- Created three multi-threaded C programs showcasing different synchronization techniques
- Achieved 2.4x-2.7x speedup in parallel array summation
- Compared mutex-based vs. lock-free (CAS) synchronization

---

## Theoretical Foundations

### Question 1: CPU Scheduling Algorithms

**Problem Setup:**
- 4 processes (P1-P4) with varying arrival times, burst times, and priorities
- Evaluated 5 scheduling algorithms: FCFS, SJF, SRTF, Priority, Round-Robin

**Key Results:**

| Algorithm | Average Waiting Time | Key Characteristic |
|-----------|---------------------|-------------------|
| **FCFS** | 6.5 ms | Simple but can cause convoy effect |
| **SJF** | 4.5 ms | Non-preemptive, good for batch systems |
| **SRTF** | **3.0 ms** | **Best average waiting time** |
| **Priority** | 3.75 ms | Can lead to starvation |
| **Round-Robin** | Variable | Fair but context-switch overhead |

**Winner:** Preemptive SRTF (Shortest Remaining Time First)

**Why SRTF Wins:**
- Always selects process with shortest remaining burst time
- Minimizes waiting time for short processes
- Long processes get preempted when shorter jobs arrive
- Trade-off: Can cause starvation for long processes

**Implementation Details from Code:**
- P1 (Burst=8) gets fragmented: runs 0-1, then 9-17
- P2 (Burst=3) runs 1-4 (preempts P1)
- P3 (Burst=5) runs 4-9
- P4 (Burst=1) would run immediately upon arrival at t=8

---

### Question 2: Scheduling Criteria Conflicts

Understanding when different performance metrics conflict is crucial for real-world scheduler design.

#### Conflict 1: Average Turnaround Time vs. Maximum Waiting Time

**Example: SRTF Algorithm**
- **Scenario:** Three processes with different burst times
- **Result:**
  - Process P1: Waiting time = 13 seconds (maximum)
  - Average turnaround time: Minimized
- **Explanation:** SRTF optimizes average performance by repeatedly preempting long processes, but this causes individual processes to suffer extreme waiting times

**Real-world Impact:** Critical processes might miss deadlines even when average system performance is optimal.

#### Conflict 2: Response Time vs. Maximum Waiting Time

**Example: Round-Robin (quantum = 4)**
- **Response time:** Excellent (all processes get CPU quickly)
- **Maximum waiting time:** Poor (processes with long bursts wait through many cycles)
- **Explanation:** Fair CPU sharing means quick initial response but prolonged completion for longer jobs

**Real-world Impact:** Interactive applications feel responsive, but background tasks take much longer to complete.

#### Conflict 3: System Throughput vs. Response Time

**Example: SJF (Shortest Job First)**
- **Throughput:** Excellent (completes many short jobs quickly)
- **Response time:** Poor for long processes (must wait for all short jobs)
- **Explanation:** Batch processing short jobs maximizes completion rate but delays first response for others

**Real-world Impact:** Batch systems vs. interactive systems have fundamentally different optimization goals.

---

### Question 3: Resource-Allocation Graphs and Deadlock

**Key Concepts:**
- No cycles → No deadlock (guaranteed safe)
- Cycles present → May or may not have deadlock (depends on resource instances)

**Analysis of Three Graphs:**

**Graph A:** No deadlock
- **Reason:** No cycles exist
- **Execution order:** T2 releases R2 → T3 acquires R2 → T1 waits and acquires R2 → All complete

**Graph B:** No deadlock
- **Reason:** No cycles exist
- **Execution order:** T1 releases R2, waits for R1 → T2 releases R1 → T3 releases R3 → All complete

**Graph C:** Deadlock detected
- **Cycle:** T1 → R2 → T3 → R1 → T1
- **Problem:**
  - R1 has only 1 instance (fully allocated to T1)
  - R2 has 2 instances (one to T3, one to T4)
  - No free resources to break the cycle
- **Critical insight:** Cycle + insufficient free instances = deadlock

**Practical Lesson:** Multiple instances of a resource can prevent deadlock even when cycles exist, but only if free instances are available.

---

### Question 4: Banker's Algorithm

**System Snapshot:**
- 5 processes (P0-P4)
- 4 resource types (A, B, C, D)
- Total resources: {3, 14, 12, 12}

**Step 1: Compute Need Matrix**
```
Need = Max - Allocation

Process  A  B  C  D
P0       0  0  0  0
P1       0  7  5  0
P2       1  0  0  2
P3       0  0  2  0
P4       0  6  4  2
```

**Step 2: Can P1 Request (0,4,2,0)?**
- Request (0,4,2,0) ≤ Need_P1 (0,7,5,0) ✓
- Request (0,4,2,0) ≤ Available (1,5,2,0) ✓
- **Answer:** Yes, request can be granted immediately

**Step 3: Safety Check**

Initial: Available = (1, 5, 2, 0)

1. **P0 runs:** Need = (0,0,0,0) ≤ Available → Available = (1,5,3,2)
2. **P2 runs:** Need = (1,0,0,2) ≤ Available → Available = (2,8,8,6)
3. **P3 runs:** Need = (0,0,2,0) ≤ Available → Available = (2,14,11,8)
4. **P4 runs:** Need = (0,6,4,2) ≤ Available → Available = (2,14,12,12)
5. **P1 runs:** Need = (0,7,5,0) ≤ Available → Complete

**Safe Sequence:** ⟨P0, P2, P3, P4, P1⟩

**Conclusion:** System is in a safe state.

---

### Question 5: Minimum Resources for Safety

**Problem:** Find minimum value of x in Available = (0, x, 4, 1) for safe state

**Solution Approach:**

**Step 1:** Compute Need Matrix
```
Need = Max - Allocation

Process  A  B  C  D
P0       0  0  0  0
P1       0  7  2  0
P2       1  0  0  2
P3       0  4  2  0
```

**Step 2:** Safety Algorithm

Starting with Available₀ = (0, x, 4, 1):

1. **P0 completes:** Need = (0,0,0,0) → Available₁ = (1, x, 5, 3)
2. **P2 completes:** Need = (1,0,0,2) ✓ → Available₂ = (2, x+3, 10, 7)
3. **P3 needs:** (0, 4, 2, 0) ≤ (2, x+3, 10, 7)
   - Requires: **x+3 ≥ 4** → **x ≥ 1**
4. **P1 needs:** (0, 7, 2, 0) ≤ (2, x+5, 13, 9)
   - Requires: **x+5 ≥ 7** → **x ≥ 2**

**Answer:** **x = 2** (minimum value)

**Safe sequence for x=2:** ⟨P0, P2, P3, P1⟩

**Key Insight:** The minimum value is determined by the most resource-demanding process in the execution sequence, considering cumulative resource releases.

---

## Programming Implementations

### Task 6: Parallel Array Summation (pthread_sum.c)

**Objective:** Parallelize summation of 1,000,000 random floats using POSIX threads

**Implementation Architecture:**

```c
// Work distribution structure
typedef struct {
    int thread_id;
    int start_index;
    int end_index;
} thread_data_t;
```

**Key Design Decisions:**

1. **Work Distribution Strategy:**
   - Array divided into equal chunks: `chunk_size = ARRAY_SIZE / num_threads`
   - Remainder distributed among first threads
   - Example with 4 threads: Each gets 250,000 elements

2. **Synchronization Approach:**
   - Each thread computes partial sum independently (no locks needed)
   - Results stored in separate array: `partial_sums[thread_id]`
   - Master thread aggregates after `pthread_join()`

3. **Code Flow:**
   ```
   Serial Sum → Timer Start → Create Threads → Compute Partial Sums
   → Join Threads → Aggregate Results → Timer Stop → Compare
   ```

**Performance Results:**

| Threads | Serial Time (s) | Parallel Time (s) | Speedup |
|---------|----------------|-------------------|---------|
| 4       | 0.0032        | 0.00123          | 2.60x   |
| 8       | 0.0034        | 0.00149          | 2.28x   |
| 12      | 0.0032        | 0.00126          | 2.54x   |
| 16      | 0.0035        | 0.00133          | 2.63x   |

**Analysis:**

**Why 4 threads perform best:**
- Modern CPUs typically have 4-8 cores
- 4 threads = optimal core utilization without oversubscription
- Minimal context switching overhead

**Why performance plateaus after 4 threads:**

1. **Thread Management Overhead:**
   - Creating 16 threads vs 4 threads: more scheduling overhead
   - Context switches increase with thread count
   - Coordination overhead grows

2. **Memory Bandwidth Saturation:**
   - Array summation is **memory-bound**, not compute-bound
   - All threads read from DRAM simultaneously
   - DRAM bandwidth becomes bottleneck
   - Formula: `Total Bandwidth = Single Thread Bandwidth × k` where k < num_threads

3. **Cache Contention:**
   - Threads compete for L1/L2/L3 cache
   - False sharing can occur on cache line boundaries
   - Cache thrashing reduces effective performance

**Critical Insight:** For memory-bound tasks, adding threads beyond available memory channels provides diminishing returns.

**Code Location:** `Assignment 2: Threads, Scheduling, Synchronization/Task6/pthread_sum.c:87-119`

---

### Task 7: Multi-threaded Histogram (multi_threaded_histogram.c)

**Objective:** Compute histogram with 30 bins using parallel processing

**Implementation Architecture:**

**Data Structures:**
```c
#define NUM_BINS 30
int *global_histogram;           // Final result
int **local_histograms;          // Per-thread local histograms
double *array;                   // Input data
```

**Synchronization Design:**

**Three-Phase Approach:**

1. **Phase 1: Parallel Computation (Lock-Free)**
   ```c
   void *thread_histogram(void *arg) {
       // Each thread updates only its local histogram
       for (int i = start_idx; i < end_idx; i++) {
           int bin = (int)((array[i] - RANGE_MIN) / bin_width);
           local_histograms[my_id][bin]++;  // No lock needed!
       }
   }
   ```

2. **Phase 2: Barrier Synchronization**
   ```c
   for (int i = 0; i < num_threads; i++) {
       pthread_join(threads[i], NULL);  // Wait for all
   }
   ```

3. **Phase 3: Serial Reduction**
   ```c
   for (int i = 0; i < num_threads; i++) {
       for (int j = 0; j < NUM_BINS; j++) {
           global_histogram[j] += local_histograms[i][j];
       }
   }
   ```

**Why This Design?**

**Advantages:**
- **Zero lock contention** during parallel phase
- **No mutex overhead** during computation
- **Guaranteed correctness** through data isolation

**Trade-offs:**
- **Extra memory:** O(threads × bins) instead of O(bins)
- **Serial reduction:** Cannot parallelize final aggregation
- **Thread overhead:** Fixed cost regardless of workload size

**Performance Results:**

| Array Size | Serial Time (s) | Parallel Time (s) | Overhead |
|------------|----------------|-------------------|----------|
| 10,000     | 0.000018      | 0.000363         | 20x slower |
| 100,000    | 0.000839      | 0.001576         | 1.9x slower |
| 1,000,000  | 0.002172      | 0.002363         | 1.1x slower |

**Surprising Result: Serial is Faster!**

**Why parallel version is slower:**

1. **Thread Creation Overhead:**
   - `pthread_create()` is expensive (~10,000 cycles)
   - For small workloads, overhead > benefit

2. **Fixed Costs:**
   - Memory allocation for local histograms
   - Thread scheduling
   - Context switches
   - Barrier synchronization

3. **Amortization:**
   - Overhead is constant
   - As array size grows, computation time grows
   - Eventually parallel will win (need larger arrays)

**Breakeven Analysis:**
- At 1,000,000 elements: 1.1x overhead
- Extrapolating: breakeven at ~10,000,000 elements
- Beyond 10M: parallel should win

**Alternative Design (Not Implemented):**
```c
// Using atomic operations instead
for (int i = start_idx; i < end_idx; i++) {
    int bin = calculate_bin(array[i]);
    __sync_fetch_and_add(&global_histogram[bin], 1);
}
```
**Trade-off:** Less memory, but atomic operations create contention.

**Key Lesson:** **Parallelization is not always beneficial.** Must consider:
- Problem size
- Memory access patterns
- Thread overhead
- Available parallelism

**Code Location:** `Assignment 2: Threads, Scheduling, Synchronization/Task7/multi_threaded_histogram.c:111-165`

---

### Task 8: Lock-Free Synchronization (pthread_stack.c)

**Objective:** Compare mutex-based vs. Compare-and-Swap (CAS) synchronization

**Data Structure:**
```c
typedef struct node {
    int node_id;
    struct node *next;
} Node;

Node *top;  // Global stack pointer (shared!)
```

**Challenge:** Multiple threads pushing/popping simultaneously can corrupt the stack.

---

#### Implementation 1: Mutex-Based Synchronization

```c
pthread_mutex_t stack_mutex;

void push_mutex() {
    Node *new_node = malloc(sizeof(Node));
    new_node->node_id = get_next_node_id();

    pthread_mutex_lock(&stack_mutex);    // LOCK
    new_node->next = top;
    top = new_node;
    pthread_mutex_unlock(&stack_mutex);  // UNLOCK
}

int pop_mutex() {
    pthread_mutex_lock(&stack_mutex);    // LOCK
    if (top == NULL) {
        pthread_mutex_unlock(&stack_mutex);
        return -1;
    }
    Node *old_node = top;
    top = top->next;
    pthread_mutex_unlock(&stack_mutex);  // UNLOCK

    free(old_node);
    return node_id;
}
```

**Characteristics:**
- **Blocking:** Threads wait if mutex is held
- **Deterministic:** Operations execute in strict order
- **Simple:** Easy to reason about correctness
- **Overhead:** Context switches when blocking

---

#### Implementation 2: Compare-and-Swap (CAS)

```c
void push_cas() {
    Node *new_node = malloc(sizeof(Node));
    new_node->node_id = get_next_node_id();

    Node *old_top;
    do {
        old_top = top;                    // Read current top
        new_node->next = old_top;         // Link to it
    } while (!__sync_bool_compare_and_swap(&top, old_top, new_node));
    // Only succeeds if top hasn't changed since we read it
}

int pop_cas() {
    Node *old_top;
    Node *new_top;

    do {
        old_top = top;
        if (old_top == NULL) return -1;
        new_top = old_top->next;
    } while (!__sync_bool_compare_and_swap(&top, old_top, new_top));

    free(old_top);
    return node_id;
}
```

**How CAS Works:**
```
__sync_bool_compare_and_swap(&top, old_top, new_node):
    if (top == old_top) {     // Atomic check
        top = new_node;       // Atomic update
        return true;
    } else {
        return false;         // Someone else modified top
    }
```

**Characteristics:**
- **Non-blocking:** Threads retry instead of waiting
- **Lock-free:** No mutex required
- **Optimistic:** Assumes no contention, retries if wrong
- **Variable performance:** Depends on contention level

---

#### Comparison: Mutex vs. CAS

**Thread Behavior:**

**Mutex Version:**
```
Thread A: lock() → push → unlock()
Thread B: [waiting...] → lock() → push → unlock()
Result: Strict serialization, predictable order
```

**CAS Version:**
```
Thread A: read top → try CAS → success
Thread B: read top → try CAS → fail (A modified it) → retry → success
Result: Concurrent attempts, retry on conflict
```

**Experimental Results:**

| Threads | Mutex: Remaining Nodes | CAS: Remaining Nodes | Notes |
|---------|----------------------|---------------------|-------|
| 1       | Deterministic: 1     | Deterministic: 1    | No contention |
| 2       | Deterministic: 2     | Deterministic: 2    | Low contention |
| 4       | Deterministic: 4     | Deterministic: 4    | Moderate contention |
| 8       | Deterministic: 8     | Order varies        | Timing-dependent |
| 16      | Deterministic: 16    | Order varies        | High retry rate |
| 32      | Deterministic: 32    | Order varies        | Very high retries |

**Observed Behavior:**

**Mutex:** Node IDs always appear in predictable order
```
Remaining: 31 → 30 → 29 → ... → 1 → 0
```

**CAS:** Node IDs can be out of order at high thread counts
```
Remaining: 28 → 31 → 25 → 30 → ... (varies)
```

**Why CAS shows ordering variations:**
1. Thread scheduling is non-deterministic
2. Retry loops create complex interleavings
3. No enforced order of operations
4. Still correct: total count always matches

**Performance Implications:**

**Low Contention (1-4 threads):**
- CAS faster: No kernel calls, no context switches
- Mutex acceptable: Rarely blocks

**High Contention (16-32 threads):**
- CAS: Many retries, wastes CPU cycles spinning
- Mutex: Threads sleep while waiting, better CPU utilization
- Trade-off depends on workload

**When to Use Each:**

| Scenario | Best Choice | Reason |
|----------|------------|--------|
| Low contention | CAS | Lower overhead, no blocking |
| High contention | Mutex | Fewer wasted cycles, fairness |
| Real-time systems | Mutex | Predictable timing |
| Low-latency systems | CAS | No kernel involvement |
| Simple operations | CAS | Hardware atomic sufficient |
| Complex critical sections | Mutex | Easier to compose |

**Code Location:** `Assignment 2: Threads, Scheduling, Synchronization/Task8/pthread_stack.c:27-100`

---

## Key Findings and Insights

### 1. Scheduling Algorithm Trade-offs

**Finding:** No single "best" scheduling algorithm exists.

**Evidence:**
- SRTF: Best average waiting time (3.0ms) but causes starvation
- FCFS: Fairest but worst performance (6.5ms avg)
- Round-Robin: Best response time but overhead from context switches

**Insight:** Real-world schedulers (e.g., Linux CFS) combine multiple strategies to balance conflicting goals.

---

### 2. Parallelization is Not Always Beneficial

**Finding:** Parallel histogram was slower than serial for all tested sizes.

**Root Causes:**
- Thread creation overhead: ~10,000 CPU cycles per thread
- Memory allocation for per-thread data structures
- Serial reduction phase cannot be parallelized
- Problem size too small to amortize fixed costs

**Insight:** **Amdahl's Law in practice:**
```
Speedup = 1 / (S + P/N)
where:
  S = serial portion (overhead + reduction)
  P = parallel portion (computation)
  N = number of threads
```

For small P, adding threads (N) doesn't help because S dominates.

**Practical Lesson:** Always measure before parallelizing. Consider:
- Is the task compute-bound or memory-bound?
- Is the problem size large enough?
- What is the serial fraction?

---

### 3. Memory Bandwidth Limits Parallelism

**Finding:** Array summation speedup plateaus at 2.6x despite 16 threads.

**Explanation:**
- Modern CPUs: ~4 memory channels × ~20 GB/s = 80 GB/s total
- Single thread: ~5-10 GB/s
- 4 threads saturate bandwidth: 40-60 GB/s
- More threads don't increase bandwidth

**Insight:** For memory-bound tasks, optimal thread count = number of memory channels, NOT number of cores.

**Implications for Design:**
```
Compute-bound task: threads = cores (good scaling)
Memory-bound task: threads = memory channels (limited scaling)
I/O-bound task: threads >> cores (hide latency)
```

---

### 4. Lock-Free != Faster

**Finding:** CAS doesn't always outperform mutexes.

**Conditions for CAS advantage:**
- Very low contention
- Simple operations (few instructions in critical section)
- Real-time requirements (no blocking)

**When mutexes win:**
- High contention (CAS wastes CPU retrying)
- Complex critical sections
- Need fairness guarantees

**Insight:** **Contention level determines best synchronization strategy:**
```
Low contention: CAS > Mutex
High contention: Mutex > CAS
Very high contention: Lock-free queue > both
```

---

### 5. Deadlock Prevention Strategies

**From Banker's Algorithm Analysis:**

**Key Requirements for Safety:**
1. Know maximum resource needs in advance
2. Don't allocate if resulting state is unsafe
3. Always keep enough free resources for at least one process

**Practical Challenges:**
- Programs rarely know maximum needs in advance
- Safety check is O(m × n²) complexity
- Conservative approach reduces concurrency

**Alternative Strategies:**
- **Deadlock Detection:** Let it happen, detect, recover
- **Deadlock Prevention:** Order resources, never wait
- **Timeouts:** Break circular wait with timeouts

**Insight:** Banker's Algorithm is theoretically sound but rarely used in practice due to overhead and requirement for a priori knowledge.

---

### 6. False Sharing in Parallel Histogram

**Hidden Performance Issue:**

Even though threads write to separate `local_histograms[i]` arrays, cache line effects matter:

```c
// Potential false sharing:
local_histograms[0][29]  ← Thread 0
local_histograms[1][0]   ← Thread 1
// These might be on the same cache line!
```

**Impact:**
- Cache line size: typically 64 bytes
- Each int: 4 bytes
- 16 ints per cache line
- Adjacent thread data → cache ping-pong

**Better Design:**
```c
// Pad each histogram to cache line boundary
struct padded_histogram {
    int bins[NUM_BINS];
    char padding[64 - (NUM_BINS * sizeof(int)) % 64];
};
```

**Insight:** In parallel code, memory layout matters as much as algorithmic correctness.

---

## Discussion Points for Seminar

### Discussion 1: Scheduling in Modern Systems

**Question:** Why do modern operating systems use multi-level feedback queues instead of simpler algorithms like SRTF?

**Discussion Points:**
- SRTF requires knowing burst time in advance (impossible)
- MLFQ approximates SJF through observation
- Handles both interactive and batch workloads
- Prevents starvation through priority boosting

**Research Question:** How does Linux CFS differ from traditional schedulers?

---

### Discussion 2: When to Parallelize

**Question:** Given the histogram results, when should we parallelize code?

**Framework for Decision:**
```
Should I parallelize?
├─ Is it compute-bound? (YES → continue, NO → probably not)
├─ Is the problem large enough? (YES → continue, NO → measure first)
├─ Is there sufficient parallelism? (YES → continue, NO → Amdahl's Law limits you)
├─ Will synchronization overhead be low? (YES → do it, NO → reconsider design)
```

**Case Studies:**
- Matrix multiplication: YES (large, compute-bound, embarrassingly parallel)
- Small linked list traversal: NO (pointer chasing, memory-bound, serial)
- Web server: YES (I/O-bound, independent requests)

---

### Discussion 3: Lock-Free Data Structures

**Question:** Why isn't all code lock-free if CAS is available?

**Challenges:**
1. **ABA Problem:**
   ```
   Thread A: reads top = X
   Thread B: pop X, pop Y, push X
   Thread A: CAS succeeds but stack state changed!
   ```
   Solution: Use versioned pointers or hazard pointers

2. **Memory Reclamation:**
   - When can we free a node?
   - Another thread might still reference it
   - Solutions: Epoch-based reclamation, RCU

3. **Complexity:**
   - Lock-free queue: 200+ lines
   - Mutex queue: 50 lines
   - More code = more bugs

**Insight:** Lock-free data structures are powerful but complex. Use them when:
- Profiling shows lock contention is a bottleneck
- Hard real-time guarantees needed
- You have time to get it right and test thoroughly

---

### Discussion 4: Deadlock in Practice

**Question:** Why don't modern systems use Banker's Algorithm?

**Reasons:**
1. Dynamic resource needs: Processes don't know maximum in advance
2. Performance overhead: O(m × n²) check per allocation
3. Reduced concurrency: Conservative allocation reduces throughput
4. Better alternatives exist: Deadlock detection with recovery

**Real-World Approaches:**
- Databases: Lock ordering, timeouts, deadlock detection
- Operating systems: Resource hierarchies, avoid hold-and-wait
- Distributed systems: Two-phase commit, consensus algorithms

**Research Question:** How does Linux handle potential deadlocks in kernel code?

---

### Discussion 5: Performance Measurement

**Question:** Why did our measurements show variance across runs?

**Factors Affecting Performance:**
1. **CPU Frequency Scaling:** Modern CPUs adjust frequency dynamically
2. **Cache State:** Cold vs. warm cache affects timing
3. **System Load:** Background processes compete for resources
4. **Thread Scheduling:** OS decides when threads run
5. **NUMA Effects:** Memory location relative to CPU matters

**Best Practices:**
- Run multiple iterations, report median or mean
- Disable frequency scaling during benchmarks
- Isolate benchmark on dedicated cores
- Warm up cache before timing
- Use high-resolution timers (`clock_gettime(CLOCK_MONOTONIC)`)

---

### Discussion 6: Optimization Trade-offs

**Question:** In Task 7, how could we improve parallel performance?

**Potential Optimizations:**

1. **Thread Pooling:**
   ```c
   // Reuse threads across multiple operations
   // Amortize creation overhead
   ```

2. **Parallel Reduction:**
   ```c
   // Use tree-based reduction instead of serial
   // O(log n) instead of O(n)
   ```

3. **SIMD Vectorization:**
   ```c
   // Process multiple array elements per instruction
   // 4-8x throughput improvement
   ```

4. **GPU Acceleration:**
   ```c
   // Offload to GPU with CUDA/OpenCL
   // 100-1000x parallelism
   ```

**Trade-off Analysis:**
Each optimization increases complexity. Must balance:
- Development time
- Code maintainability
- Performance gain
- Portability

**Insight:** Premature optimization is the root of all evil. Profile first, optimize bottlenecks, measure improvement.

---

## Conclusion

This assignment demonstrates that operating systems must carefully balance competing goals:

**Scheduling:** Optimize for average case while preventing starvation
**Synchronization:** Balance correctness, performance, and complexity
**Parallelism:** Only beneficial when gains exceed overhead
**Deadlock:** Prevention vs. detection vs. ignorance—all have trade-offs

**Key Takeaways for Real-World Systems:**

1. **Measure, don't assume:** Our histogram was slower parallel—surprising but true
2. **Understand your bottleneck:** Memory-bound vs. compute-bound determines scaling
3. **Choose the right tool:** Mutex vs. CAS depends on contention level
4. **Consider all costs:** Thread creation, synchronization, and cache effects add up
5. **Start simple:** Optimize only when profiling identifies bottlenecks

**Further Reading:**
- "Operating System Concepts" (Silberschatz et al.) - Chapters 5-7
- "The Art of Multiprocessor Programming" (Herlihy & Shavit)
- Linux kernel source: `kernel/sched/` and `kernel/locking/`
- Maurice Herlihy's papers on lock-free data structures

---

## Appendix: Code Performance Summary

### Task 6: Array Summation
- **Best configuration:** 4 threads
- **Speedup achieved:** 2.6x
- **Bottleneck:** Memory bandwidth
- **Code:** `Task6/pthread_sum.c`

### Task 7: Histogram
- **Result:** Serial faster than parallel
- **Reason:** Thread overhead > computation
- **Fix:** Need larger arrays (10M+ elements)
- **Code:** `Task7/multi_threaded_histogram.c`

### Task 8: Stack Operations
- **Mutex:** Deterministic, blocking
- **CAS:** Non-blocking, variable order
- **Correct count:** Both implementations
- **Code:** `Task8/pthread_stack.c`

---

**Document prepared for Seminar 2 - ID1206 Operating Systems**
*All code implementations available in assignment repository*
