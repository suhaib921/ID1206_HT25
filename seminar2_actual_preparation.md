# Seminar 2 Preparation: Deadlocks and Memory Management

## Part 1: Deadlocks

### What is a Deadlock?

A **deadlock** is a situation where a set of processes are blocked because each process is holding a resource and waiting for another resource acquired by some other process. No process can proceed, creating a circular wait condition.

**Real-world analogy**: Four cars arrive at a four-way intersection simultaneously. Each car is blocking the path of the next car in a circular pattern. No car can move forward without the other moving first.

### The Four Necessary Conditions for Deadlock (Coffman Conditions)

A deadlock can occur **only if** all four of these conditions hold simultaneously:

1. **Mutual Exclusion**: At least one resource must be held in a non-sharable mode. Only one process can use the resource at a time.
   - Example: A printer cannot be used by two processes simultaneously.

2. **Hold and Wait**: A process must be holding at least one resource and waiting to acquire additional resources that are currently held by other processes.
   - Example: Process A holds resource R1 and is waiting for R2, while continuing to hold R1.

3. **No Preemption**: Resources cannot be forcibly taken away from a process. They must be released voluntarily by the process holding them.
   - Example: The OS cannot force a process to release a mutex it has locked.

4. **Circular Wait**: A circular chain of processes exists, where each process holds at least one resource needed by the next process in the chain.
   - Example: P1 waits for P2, P2 waits for P3, P3 waits for P1.

**Key Insight**: If you can prevent even ONE of these conditions from occurring, deadlock is impossible.

---

## Section 7.3: Deadlock Characterization

### Resource-Allocation Graph

A visual tool to represent the state of resource allocation in a system:

- **Vertices**: Two types
  - Processes (circles): P1, P2, P3, ...
  - Resource types (rectangles): R1, R2, R3, ...
  - Dots inside rectangles represent instances of resources

- **Edges**: Two types
  - **Request edge**: P → R (process requests resource)
  - **Assignment edge**: R → P (resource is allocated to process)

**Deadlock Detection using the graph**:
- If the graph contains **no cycles**, then no deadlock exists
- If the graph contains a **cycle**:
  - If only **one instance per resource type**, then deadlock exists
  - If **multiple instances**, then deadlock *may* exist (need further analysis)

---

## Section 7.4: Methods for Handling Deadlocks

There are three fundamental approaches:

1. **Prevention/Avoidance**: Ensure the system never enters a deadlock state
2. **Detection and Recovery**: Allow deadlocks to occur, detect them, and recover
3. **Ignore the problem**: Pretend deadlocks never happen (ostrich algorithm) - used by most operating systems including UNIX/Linux!

**Why do most OSes ignore deadlocks?**
- Deadlock prevention/avoidance has high overhead
- Deadlocks are relatively rare in practice
- Detection and recovery is complex
- System restart is often acceptable for end-user systems

---

## Section 7.5: Deadlock Prevention

**Strategy**: Ensure that at least one of the four necessary conditions cannot hold.

### 1. Preventing Mutual Exclusion
- **Not practical for most resources**: Some resources (like printers, mutex locks) are inherently non-sharable
- **Possible for some resources**: Read-only files can be shared

### 2. Preventing Hold and Wait
**Approach**: Guarantee that whenever a process requests a resource, it does not hold any other resources.

**Protocol 1**: Require processes to request all resources at once before execution begins
- **Pros**: Simple to implement
- **Cons**:
  - Low resource utilization (resources held but not used)
  - Starvation possible (process needing many resources may wait indefinitely)
  - Hard to know all resources needed in advance

**Protocol 2**: Require a process to release all currently held resources before requesting new ones
- **Pros**: More flexible
- **Cons**:
  - Work may be lost when resources are released
  - Still has low resource utilization

### 3. Preventing No Preemption
**Approach**: Allow resources to be preempted (forcibly taken away).

**Protocol**:
- If a process requests a resource that cannot be immediately allocated, release all currently held resources
- The process can restart only when it can acquire both old and new resources### 3. Preventing No Preemption

**Applicability**:
- Works well for resources whose state can be saved and restored (CPU registers, memory)
- **Not applicable** to resources like printers and mutex locks where state cannot be easily saved

### 4. Preventing Circular Wait
**Approach**: Impose a total ordering on all resource types. Require that processes request resources in increasing order.

**Implementation**:
- Assign a unique number to each resource type: F(tape drive) = 1, F(disk drive) = 5, F(printer) = 12
- Each process can request resources only in increasing order of enumeration

**Example**:
- Process can request: disk drive (5), then printer (12) ✓
- Process cannot request: printer (12), then disk drive (5) ✗

**Analysis**:
- **Pros**: Practical and widely used
- **Cons**:
  - May not be possible to find an ordering that satisfies all applications
  - Resource utilization may decrease

---

## Section 7.6: Deadlock Avoidance

**Strategy**: Use additional information about how resources will be requested to decide whether to grant or delay a request, even if resources are currently available.

### Safe State Concept

A state is **safe** if the system can allocate resources to each process in some order and still avoid deadlock.

**Safe State**: A sequence of processes <P1, P2, ..., Pn> exists such that for each Pi:
- The resources that Pi can still request can be satisfied by currently available resources + resources held by all Pj where j < i
- If Pi's resources are not immediately available, Pi can wait until all Pj (j < i) finish and release resources

**Key Points**:
- Safe state → No deadlock
- Unsafe state → Possibility of deadlock
- Deadlock state → Subset of unsafe state

**Avoidance Strategy**: Ensure system never enters an unsafe state.

### Resource-Allocation-Graph Algorithm

**Applicability**: Only for systems with **single instance** of each resource type.

**Mechanism**: Add a new type of edge:
- **Claim edge** (dashed line): Pi → Rj indicates Pi *may* request Rj in the future
- When Pi actually requests Rj, claim edge converts to request edge
- When resource is allocated, request edge converts to assignment edge
- When resource is released, assignment edge converts back to claim edge

**Algorithm**: Request is granted only if converting the request edge to an assignment edge does not create a cycle (including claim edges).

### Banker's Algorithm

**Applicability**: Systems with **multiple instances** of resource types.

**Name origin**: Models a banker managing loans (resources) to customers (processes) ensuring the bank never commits resources such that it cannot satisfy all customers' needs.

**Data Structures** (n = number of processes, m = number of resource types):
- **Available[m]**: How many resources are free in the system right now.
- **Max[n][m]**: How many resources each process may need in total.
- **Allocation[n][m]**: How many resources each process currently holds.
- **Need[n][m]**: How many more resources each process still requires to finish. Remaining resource need of each process = Max - Allocation

**Safety Algorithm**:
1. Initialize Work = Available and Finish[i] = false for all i
2. Find a process i such that:
   - Finish[i] == false
   - Need[i] ≤ Work (process can finish with available resources)
3. If found: Work = Work + Allocation[i], Finish[i] = true, go to step 2
4. If all Finish[i] == true, system is in safe state

**Resource-Request Algorithm**:
When process Pi requests resources Request[i]:
1. If Request[i] ≤ Need[i], go to step 2; else error (exceeded maximum claim)
2. If Request[i] ≤ Available, go to step 3; else Pi must wait
3. **Pretend** to allocate resources:
   - Available = Available - Request[i]
   - Allocation[i] = Allocation[i] + Request[i]
   - Need[i] = Need[i] - Request[i]
4. Run Safety Algorithm:
   - If safe state → allocate resources to Pi
   - If unsafe state → Pi must wait and restore old state

**Limitations**:
- Requires processes to declare maximum resource needs in advance
- Number of processes must be fixed
- Resources cannot vary
- High overhead for checking safety

---

## Section 7.7: Deadlock Detection and Recovery

**When to use**: Systems that do not use prevention or avoidance.

### Detection Algorithm

**Single instance per resource type**: Use wait-for graph (variant of resource-allocation graph)
- Remove resource nodes, collapse edges
- Edge Pi → Pj means Pi is waiting for Pj to release a resource
- Cycle detection = deadlock detection

**Multiple instances per resource type**: Algorithm similar to Banker's algorithm

**Data Structures**:
- Available[m]: Currently available resources
- Allocation[n][m]: Current allocation
- Request[n][m]: Current outstanding requests

**Algorithm**:
1. Initialize Work = Available, Finish[i] = false if Allocation[i] ≠ 0
2. Find process i such that:
   - Finish[i] == false
   - Request[i] ≤ Work
3. If found: Work = Work + Allocation[i], Finish[i] = true, repeat step 2
4. If Finish[i] == false for some i, then system is deadlocked (those processes are deadlocked)

**When to invoke detection algorithm?**
- Every time a request cannot be granted (high overhead but detects immediately)
- Periodically (depends on deadlock frequency and number of affected processes)
- When CPU utilization drops below threshold (processes may be deadlocked)

### Recovery from Deadlock

**Option 1: Process Termination**

1. **Abort all deadlocked processes**
   - **Pros**: Simple
   - **Cons**: Expensive (all computation is lost)

2. **Abort one process at a time until deadlock cycle is eliminated**
   - **Pros**: Minimal processes killed
   - **Cons**: High overhead (must run detection algorithm after each abortion)

**Selection criteria** (which process to terminate):
- Priority of the process
- How long has process computed and how much longer it needs
- Resources the process has used
- Resources process needs to complete
- How many processes will need to be terminated
- Is process interactive or batch?

**Option 2: Resource Preemption**

Select a victim → Rollback → Starvation

1. **Selecting a victim**: Which resources and processes to preempt (minimize cost)

2. **Rollback**:
   - **Total rollback**: Abort process and restart
   - **Partial rollback**: Rollback only as far as necessary to break deadlock (requires checkpointing)

3. **Starvation**:
   - Problem: Same process may always be picked as victim
   - Solution: Include number of rollbacks in cost factor

---

## Part 2: Memory Management in Linux

### Process Memory Layout (Foundation)

Before understanding memory reclaim and OOM, we need to understand how process memory is organized.

**Memory Segments**: Each process has its memory divided into distinct regions:

```
High Address
┌─────────────────────┐
│    Stack            │ ← Local variables, function calls, grows downward
├─────────────────────┤
│         ↓           │
│    (free space)     │
│         ↑           │
├─────────────────────┤
│    Heap             │ ← Dynamically allocated memory (malloc), grows upward
├─────────────────────┤
│    BSS (Uninit.)    │ ← Uninitialized global/static variables
├─────────────────────┤
│    Data (Init.)     │ ← Initialized global/static variables
├─────────────────────┤
│    Text             │ ← Program code (read-only)
└─────────────────────┘
Low Address
```

#### Real Example from Assignment 1

```c
#define N 1024              // Text segment (constant)

double sum;                 // BSS (uninitialized global)
double *a;                  // BSS (uninitialized pointer)

double getSum(double *x) {  // Text segment (function code)
    int i;                  // Stack (local variable)
    double s = 0;           // Stack (local variable)

    for (i = 0; i < N; i++)
        s += x[i];
    return s;
}

int main() {
    int i;                  // Stack (local variable)
    a = (double*) malloc(N*sizeof(double));  // Heap (dynamic allocation)

    for (i = 0; i < N; i++)
        a[i] = i;

    sum = getSum(a);        // sum stored in Data/BSS
    printf("sum = %.2f\n", sum);
    return 0;
}
```

**Memory placement breakdown**:
- **Text segment**: Function code (`main`, `getSum`), constants (`N`)
- **Data segment (initialized)**: Global `N = 1024`
- **BSS segment (uninitialized)**: Global `sum`, global pointer `a`
- **Heap**: Array allocated by `malloc(N*sizeof(double))`
- **Stack**: Local variables `i`, `s` in both functions

**Why this matters for memory management**:
1. **Stack** is automatically managed (grows/shrinks with function calls) - easy to reclaim
2. **Heap** persists until explicitly freed - potential for memory leaks
3. **Text segment** is read-only and shared between processes running the same program
4. **Data/BSS** is writable but fixed size
5. Memory leaks occur when heap memory is allocated but never freed

---

### Memory Reclaim

**Purpose**: Free up memory when the system is running low by reclaiming pages that can be recreated or moved to disk.

#### What Can Be Reclaimed?

The kernel distinguishes between:
- **Reclaimable memory**: Pages that can be freed because they either:
  - Cache data available elsewhere (e.g., page cache of disk files)
  - Can be swapped out (anonymous memory like process heap/stack)
- **Unreclaimable memory**:
  - Kernel data structures
  - DMA buffers
  - Pinned pages
  - (Exception: some kernel metadata caches can be reclaimed if reloadable from storage)

#### Reclaim Triggers: Watermark System

Linux uses a three-level watermark system for each memory zone:

1. **High watermark**: System is comfortable, no reclaim needed
2. **Low watermark**: When free pages drop here, `kswapd` daemon wakes up and starts background reclaim
3. **Min watermark**: Critical threshold - triggers **direct reclaim** where allocating processes themselves must free memory before they can proceed

#### Reclaim Strategies

**1. Asynchronous Reclaim (kswapd)**
- Background daemon that wakes when memory hits low watermark
- Scans pages and frees them without blocking applications
- Tries to keep free memory above low watermark
- Preferred method because it doesn't block applications

**2. Synchronous Reclaim (Direct Reclaim)**
- Occurs when memory drops to min watermark
- The process requesting memory must participate in reclaiming pages
- **Blocks the allocation** until enough memory is freed
- Performance impact: causes latency for the requesting process

#### What Happens During Reclaim?

For each page being reclaimed:
- **Clean pages**: If data exists on disk (page cache), just free the page
- **Dirty pages**: Must write to disk first (writeback), then free
- **Anonymous pages**: Must write to swap space, then free

---

### Swapping in Linux

**Background**: Traditionally, Linux avoided swapping due to poor performance on rotating hard drives. With SSDs and fast storage, swapping is being reconsidered.

#### Memory Types

1. **File-backed pages**:
   - Correspond to files on disk
   - Can be dropped and re-read from disk
   - Efficient to reclaim (large contiguous I/O)

2. **Anonymous pages**:
   - Process runtime data (stack, heap, malloc'd memory)
   - No backing file on disk
   - Require swap space to be reclaimed
   - Historically slow on HDDs due to scattered I/O patterns

#### The Swappiness Parameter

**swappiness**: A tunable parameter (range 0-200, traditionally 0-100) that controls the balance between reclaiming page cache vs. swapping out anonymous memory.

- **Low swappiness (e.g., 10)**: Strongly prefer reclaiming page cache over swapping
- **High swappiness (e.g., 100)**: More willing to swap out anonymous pages
- **Default**: Usually 60

**Traditional view**: Keep swappiness low or disable swap entirely due to poor HDD performance.

#### Modern Reconsideration (Fast Storage)

With SSDs and persistent memory:
- Random I/O is much faster, making swap more viable
- Swap can extend memory during moderate load (not just emergency overflow)
- **Cost-based reclaim**: Modern proposals track:
  - **Rotations**: How many times a page is scanned during reclaim
  - **Refaults**: How many times a reclaimed page must be brought back
- Automated tuning based on workload rather than manual swappiness setting

**Performance example**: PostgreSQL testing showed improvement from 81 to 105 transactions/second with better swap policies.

---

### Out of Memory (OOM) Killer

**Purpose**: When the system runs completely out of memory and cannot reclaim enough to continue, the OOM Killer sacrifices one or more processes to free memory and save the overall system.

#### Why OOM Killer Exists

Linux uses **memory overcommitment**: The kernel may allocate more virtual memory to processes than physically exists.

**Example**: System with 2GB RAM might allocate 2.5GB to processes, betting that not all processes will use their full allocation simultaneously.

**Problem**: When processes actually try to use all allocated memory, the system runs out. The OOM killer must decide which process to terminate.

#### How Process Selection Works

**OOM Score**: Each process has an `oom_score` (visible at `/proc/$PID/oom_score`)

The scoring considers:
- Memory usage (higher usage = higher score)
- Runtime (newly started processes have higher scores)
- Whether the process is critical to system operation
- Process priority and niceness

**The process with the highest `oom_score` is killed.**

#### Checking OOM Score

```bash
# Check OOM score for a specific process
cat /proc/$PID/oom_score

# Check all processes' OOM scores
for pid in /proc/[0-9]*; do
    printf "%s\t%s\n" "$(cat $pid/oom_score 2>/dev/null)" "$(cat $pid/cmdline 2>/dev/null)";
done | sort -rn | head -20
```

#### Adjusting OOM Killer Behavior

**oom_score_adj**: A value from -1000 to +1000 that adjusts the likelihood of a process being killed

```bash
# Make a process less likely to be killed
echo -100 > /proc/$PID/oom_score_adj

# Make a process more likely to be killed
echo 100 > /proc/$PID/oom_score_adj

# Make a process immune (requires root, use carefully!)
echo -1000 > /proc/$PID/oom_score_adj
```

**Important limitations**:
- Adjustments **do not guarantee** a process won't be killed during severe memory crisis
- Settings **reset when the process restarts** - must automate for persistence
- No true "prevention" exists, only influencing selection likelihood

#### Practical Considerations

**For memory-intensive applications** (databases, caches like Redis/Neo4j):
- These are prime candidates for OOM killer during memory pressure
- Should consider:
  - Setting negative `oom_score_adj` to reduce termination risk
  - Monitoring `/var/log/kern.log` for OOM killer events
  - Proper memory limits and resource planning
  - Using cgroups for memory isolation

**Detecting OOM Killer Activity**:
```bash
# Check kernel logs for OOM events
dmesg | grep -i "killed process"
grep -i "out of memory" /var/log/kern.log
```

**OOM Killer messages include**:
- Which process was killed
- Memory state at the time
- OOM score of killed process
- Memory usage of all processes at that moment

---

### Resource Leaks and Zombie Processes

Understanding resource management problems helps explain why OOM situations occur and why proper cleanup is critical.

#### Zombie Processes: A Resource Leak Example

A **zombie process** is a terminated process that still occupies system resources because its parent hasn't collected its exit status.

**Real Example from Assignment 1**:

```c
int main() {
    int pid = fork();

    if(pid == 0) {
        // Child process
        printf("I'm the child %d\n", getpid());
        printf("PID%d says goodbye\n", getpid());
        // Child exits immediately
    } else {
        // Parent process
        printf("My child is called %d\n", pid);
        sleep(10);  // Parent sleeps for 10 seconds
    }

    printf("PID%d says goodbye\n", getpid());
    return 0;
}
```

**Timeline of what happens**:

```
Time: 0s
├─ Parent: forks, creates child 48002
├─ Parent: prints "My child is called 48002"
└─ Parent: goes to sleep for 10 seconds

Time: ~0.1s
├─ Child: prints "I'm the child 48002"
├─ Child: prints "PID48002 says goodbye"
└─ Child: EXITS → becomes ZOMBIE (state Z, shows as <defunct>)

Time: 0s - 10s
├─ Parent: Sleeping (state S)
└─ Child: Zombie (state Z) - OCCUPYING RESOURCES

Time: 10s
├─ Parent: wakes up, prints "PID48001 says goodbye"
└─ Parent: exits → zombie child gets reaped
```

**What resources do zombies consume?**
- **Process table entry**: Occupies a slot in the process table
- **PID**: The PID cannot be reused while zombie exists
- **Exit status**: Kernel must preserve the exit code for parent to collect
- **Minimal memory**: Stack/heap are freed, but kernel data structures remain

**Connection to OOM and System Exhaustion**:

1. **PID exhaustion**: If a parent never calls `wait()` and keeps creating children:
   ```c
   // BAD CODE - Creates zombie army
   while(1) {
       if(fork() == 0) exit(0);  // Child exits immediately
       // Parent never calls wait() → zombies accumulate
   }
   // Result: Eventually runs out of PIDs!
   ```

2. **Process table exhaustion**: The system has a limited number of process table entries (`/proc/sys/kernel/pid_max`)

3. **How to prevent zombies**:
   - **Option 1**: Parent calls `wait()` or `waitpid()`
     ```c
     int status;
     wait(&status);  // Reaps the zombie
     ```

   - **Option 2**: Ignore `SIGCHLD` signal (automatic reaping)
     ```c
     signal(SIGCHLD, SIG_IGN);
     ```

   - **Option 3**: Double fork technique (orphan becomes init's problem)

**Orphan vs. Zombie**:
- **Zombie**: Child is dead, parent is alive but hasn't reaped it
- **Orphan**: Parent is dead, child is alive → automatically adopted by `init` (PID 1)

**Practical check**:
```bash
# Find zombie processes
ps aux | grep 'Z'
ps aux | grep 'defunct'

# Count zombies
ps aux | awk '$8=="Z" {count++} END {print count}'
```

**Why this matters for the seminar**:
- Demonstrates how **resource leaks** (not just memory) can exhaust system resources
- Shows the importance of **proper cleanup** (like `wait()` for processes, `free()` for memory)
- Zombie accumulation is a form of **resource deadlock** (PIDs locked but unusable)
- Connects to OOM discussion: improper resource management leads to exhaustion

---

### Process Memory Isolation (Fork and Copy-on-Write)

Understanding how processes maintain separate memory spaces is crucial for memory management.

#### Memory After `fork()`: Not Truly Shared

**Example from Assignment 1**:

```c
#define N 5
int value = 0;  // Global variable

void *thread_func(void *param) {
    value++;
    pthread_exit(0);
}

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        pthread_attr_t attr;
        pthread_t tid;

        for(int i=0; i<N; i++) {
            pthread_create(&tid, &attr, thread_func, NULL);
            pthread_join(tid, NULL);
        }
        printf("CHILD: value=%d\n", value);   // Prints 5
    } else {
        // Parent process
        wait(NULL);
        printf("PARENT: value=%d\n", value);  // Prints 0
    }
}
```

**Output**:
```
CHILD: value=5
PARENT: value=0
```

**Why different values?**

After `fork()`:
1. Child gets a **copy** of the parent's memory space
2. Both have variable `value` at the same virtual address
3. But they point to **different physical memory** (Copy-on-Write)
4. Changes in child's memory **do not affect** parent's memory

**Copy-on-Write (COW) mechanism**:
```
Before fork():
Parent Process
├─ value = 0 (at address 0x1000, points to physical page A)

Immediately after fork():
Parent Process
├─ value = 0 (at address 0x1000, points to physical page A) [read-only]
Child Process
├─ value = 0 (at address 0x1000, points to physical page A) [read-only]
   └─ Both share the SAME physical page (efficient!)

After child modifies value:
Parent Process
├─ value = 0 (at address 0x1000, points to physical page A)
Child Process
├─ value = 5 (at address 0x1000, points to physical page B) [NEW copy created]
```

**Memory implications**:
1. **Initial efficiency**: Fork is cheap because memory is shared (read-only)
2. **COW overhead**: First write to a page triggers a copy → memory usage increases
3. **Memory pressure**: If child modifies large portions of memory, nearly doubles memory usage
4. **Relevance to OOM**: Forking processes under memory pressure can trigger OOM killer

**Why threads share memory but processes don't**:
- **Threads**: Live in the **same process** → share heap, data, text (separate stacks only)
- **Processes**: Separate address spaces → changes isolated via COW

---

## Summary and Key Discussion Points

### Deadlocks
1. **Four necessary conditions** must all hold for deadlock to occur
2. **Prevention**: Break one of the four conditions (resource ordering is most practical)
3. **Avoidance**: Banker's algorithm ensures safe states but has high overhead
4. **Detection**: Periodically check for cycles in wait-for graph
5. **Recovery**: Terminate or rollback processes
6. **Real world**: Most OSes ignore the problem due to rarity and overhead of prevention

### Memory Management
1. **Process memory layout**: Text, Data, BSS, Heap, Stack - each has different reclaim characteristics
2. **Reclaim hierarchy**: kswapd (background) → direct reclaim (blocking)
3. **Swapping evolution**: HDD (avoid) → SSD (reconsider) with cost-based decisions
4. **OOM Killer**: Last resort when reclaim fails; selects victim by oom_score
5. **Resource leaks**: Zombie processes show how non-memory resources can be exhausted
6. **Copy-on-Write**: Fork efficiency vs. memory pressure under COW
7. **Trade-offs**: Performance vs. reliability, prevention cost vs. recovery cost

### Questions to Consider for Seminar

**Deadlocks**:
- When is deadlock prevention preferable to avoidance or detection?
- Why do modern operating systems mostly ignore deadlocks?
- Can zombie processes cause a form of deadlock? (PIDs are locked but unusable)
- How does resource ordering prevention relate to database transaction ordering?

**Memory Management**:
- How does the watermark system balance proactive reclaim vs. system overhead?
- In what scenarios would you want high vs. low swappiness?
- How would you protect a critical application from the OOM killer?
- What are the trade-offs between memory overcommitment and system stability?
- Which memory segments (text/data/heap/stack) are easiest to reclaim? Why?
- How does Copy-on-Write affect memory usage when forking under memory pressure?
- Can zombie processes contribute to system resource exhaustion? How?
- What's the difference between a memory leak and a zombie process leak?

### Practical Examples from Your Course

**From Assignment 1 you've already worked with**:
1. **Memory layout** - Understanding where variables live (text/data/heap/stack)
2. **Process creation** - Fork creates 2^(2N) processes, showing exponential resource usage
3. **Zombie processes** - Practical example of resource leaks (PIDs, process table entries)
4. **Process isolation** - Fork + COW demonstrates separate memory spaces
5. **IPC overhead** - FIFO/pipe communication shows synchronization costs

**Connections to make**:
- Assignment 1's zombie example → Resource exhaustion → OOM conditions
- Assignment 1's memory layout → Which segments are reclaimable
- Assignment 1's fork patterns → COW memory pressure → Potential OOM triggers
- Assignment 2's thread synchronization → Deadlock prevention via resource ordering
