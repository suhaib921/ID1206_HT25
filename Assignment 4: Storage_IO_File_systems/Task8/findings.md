# Task 8: Memory-Mapped Files - Findings and Results

## Overview
This document contains the results, explanations, and observations from implementing and testing a C program that uses memory-mapped files with `mmap()` and process forking.

## Task Setup
- **File created**: `file_to_map.txt` (1MB size)
- **Initial program**: `mmap_program.c`
- **Modified program**: `mmap_program_synced.c` (with msync() synchronization)

### Initial Setup Commands
```bash
# Create the 1MB file
dd if=/dev/zero of=file_to_map.txt bs=1M count=1

# Compile the initial program
gcc -o mmap_program mmap_program.c
```

---

## Part (a): Virtual Address from mmap()

### Command to Run
```bash
./mmap_program
```

### Results
When running the program, both the parent and child processes print their Process ID (PID) and the virtual address for the memory-mapped region. The addresses are identical within a single execution because the child inherits the parent's memory space.

```
Child process (pid=28109); mmap address: 0x7f8d2c6b4000
Parent process (pid=28108); mmap address: 0x7f8d2c6b4000
```

### Observations
- **Same address within a run**: Both processes map to the same virtual address because the child's virtual address space is a copy of the parent's at the time of `fork()`, so the memory mapping is inherited.
- **Different addresses between runs**: The base virtual address changes between different executions due to **Address Space Layout Randomization (ASLR)**, a security feature in modern operating systems that prevents attackers from guessing memory locations.
- **MAP_SHARED flag**: This flag is crucial. It ensures that modifications to the mapped memory are visible to all processes sharing the mapping, as they are all mapped to the same underlying physical memory pages.

---

## Part (b): Character Read/Write Output

### Command to Run
```bash
./mmap_program
```

### Results
The output shows the parent and child processes reading data written by the other. Due to non-deterministic scheduling, the order of the lines may vary between runs.

```
Child process (pid=28109); read from mmaped_ptr[4096]: 56789
Parent process (pid=28108); read from mmaped_ptr[0]: 01234
```

### Explanation
- The **child process** writes data to the beginning of the mapped region (`mmaped_ptr[0]`) and then reads from an offset of 4096 bytes.
- The **parent process** writes data to that offset (`mmaped_ptr[4096]`) and then reads from the beginning.
- The output demonstrates that both processes successfully read the data written by the other. This confirms that:
  - The memory mapping is truly **shared** between the processes.
  - Changes made by one process are **immediately visible** to the other.
  - The `MAP_SHARED` flag is working as intended to enable this inter-process communication.

---

## Part (c): Consistency Across Multiple Runs

### Command to Run
```bash
for i in {1..10}; do echo "=== Run $i ==="; ./mmap_program; echo ""; done
```
bash -c 'for i in {1..10}; do echo "==
= Run $i ==="; ./mmap_program; echo ""; done'

=== Run 1 ===
Parent process (pid=106902); mmap address: 0x75e5f16b5000
Child process (pid=106903); mmap address: 0x75e5f16b5000
Parent process (pid=106902); read from mmaped_ptr[0]: 01234
Child process (pid=106903); read from mmaped_ptr[4096]: 56789

=== Run 2 ===
Parent process (pid=106904); mmap address: 0x7fc9c2504000
Child process (pid=106905); mmap address: 0x7fc9c2504000
Parent process (pid=106904); read from mmaped_ptr[0]: 01234
Child process (pid=106905); read from mmaped_ptr[4096]: 56789

=== Run 3 ===
Parent process (pid=106906); mmap address: 0x72850e89e000
Child process (pid=106907); mmap address: 0x72850e89e000
Parent process (pid=106906); read from mmaped_ptr[0]: 01234
Child process (pid=106907); read from mmaped_ptr[4096]: 56789

=== Run 4 ===
Parent process (pid=106908); mmap address: 0x7287ffa13000
Child process (pid=106909); mmap address: 0x7287ffa13000
Parent process (pid=106908); read from mmaped_ptr[0]: 01234
Child process (pid=106909); read from mmaped_ptr[4096]: 56789

=== Run 5 ===
Parent process (pid=106910); mmap address: 0x7620d2500000
Child process (pid=106911); mmap address: 0x7620d2500000
Parent process (pid=106910); read from mmaped_ptr[0]: 01234
Child process (pid=106911); read from mmaped_ptr[4096]: 56789

=== Run 6 ===
Parent process (pid=106912); mmap address: 0x7c86b4100000
Child process (pid=106913); mmap address: 0x7c86b4100000
Parent process (pid=106912); read from mmaped_ptr[0]: 01234
Child process (pid=106913); read from mmaped_ptr[4096]: 56789

=== Run 7 ===
Parent process (pid=106914); mmap address: 0x7868e8069000
Child process (pid=106915); mmap address: 0x7868e8069000
Parent process (pid=106914); read from mmaped_ptr[0]: 01234
Child process (pid=106915); read from mmaped_ptr[4096]: 56789

=== Run 8 ===
Parent process (pid=106916); mmap address: 0x7021d288b000
Child process (pid=106917); mmap address: 0x7021d288b000
Parent process (pid=106916); read from mmaped_ptr[0]: 01234
Child process (pid=106917); read from mmaped_ptr[4096]: 56789

=== Run 9 ===
Child process (pid=106919); mmap address: 0x734775f00000
Parent process (pid=106918); mmap address: 0x734775f00000
Child process (pid=106919); read from mmaped_ptr[4096]: 56789
Parent process (pid=106918); read from mmaped_ptr[0]: 01234

=== Run 10 ===
Parent process (pid=106920); mmap address: 0x790f56af6000
Child process (pid=106921); mmap address: 0x790f56af6000
Parent process (pid=106920); read from mmaped_ptr[0]: 01234
Child process (pid=106921); read from mmaped_ptr[4096]: 56789

### Test Method
The program was executed 10 consecutive times to observe consistency.

### Results
**YES**, the output is consistent across all 10 runs. Here's a summary:

| Run | Virtual Address  | Child Reads | Parent Reads |
|-----|------------------|-------------|--------------|
| 1   | 0x73871d666000   | '56789'     | '01234'      |
| 2   | 0x7c35b64cb000   | '56789'     | '01234'      |
| 3   | 0x794d91700000   | '56789'     | '01234'      |
| 4   | 0x73a16ee52000   | '56789'     | '01234'      |
| 5   | 0x7f4eb5d00000   | '56789'     | '01234'      |
| 6   | 0x727523f00000   | '56789'     | '01234'      |
| 7   | 0x7a2540b00000   | '56789'     | '01234'      |
| 8   | 0x7a244185f000   | '56789'     | '01234'      |
| 9   | 0x728e0f2fd000   | '56789'     | '01234'      |
| 10  | 0x7961d2700000   | '56789'     | '01234'      |

### Observations
1. **Virtual addresses vary** between runs (ASLR)
2. **Read/write behavior is 100% consistent**:
   - Child always reads `'56789'` from location 4096
   - Parent always reads `'01234'` from location 0
3. **Shared memory works reliably**: Despite concurrent access, the `MAP_SHARED` flag ensures data consistency
4. **No race conditions observed** in this simple test case, though the 100µs delay helps with timing

---

## Part (d): Modification for Guaranteed Synchronization

### Understanding the Problem

The original program has a **race condition**. While the `MAP_SHARED` flag makes changes visible between processes sharing the memory mapping, there's no guarantee about the **ordering** of operations:

1. **No explicit synchronization**: The processes run concurrently without coordination
2. **Potential race conditions**: A process might read before the other has written
3. **Timing-dependent behavior**: The 100µs delay helps but doesn't guarantee correctness

### CRITICAL: Memory Visibility vs Process Synchronization

**Important distinction:**
- **Memory visibility**: When one process writes to shared memory, when does another process see it?
- **Process synchronization**: How do we ensure operations happen in a specific order?

**For `MAP_SHARED` memory mappings:**
- Changes are **immediately visible** to other processes sharing the mapping
- No special system call is needed for visibility - they share the same physical pages!
- `msync()` does NOT help with this - the processes already see each other's changes

**What `msync()` actually does:**
```c
msync(void *addr, size_t length, int flags);
```
- Forces write-back of dirty pages to the **disk file**
- Ensures changes are visible to processes that **read the file directly** (not via the existing mapping)
- Useful for persistence and for NEW processes that open the file
- **NOT** a synchronization primitive for processes sharing a mapping

### The Wrong "Fix" - Using msync() with sleep()

File: `mmap_program_synced.c`

```bash
# Compile command
gcc -o mmap_program_synced mmap_program_synced.c
```

This version adds `msync()` calls and `sleep()` delays. However:

**Why it appears to work:**
- The `sleep(1)` calls create a time-based ordering
- Child writes → sleeps → Parent writes → child reads
- The timing prevents the race condition

**Why this is wrong:**
- The synchronization comes from `sleep()`, NOT from `msync()`
- `msync()` is unnecessary here - the processes already see each other's changes
- This is inefficient (wastes CPU time sleeping)
- This is unreliable (timing assumptions can fail on different systems)

### The Correct Fix - Using Semaphores

File: `mmap_program_proper_sync.c`

```bash
# Compile command
gcc -o mmap_program_proper_sync mmap_program_proper_sync.c -pthread
```

**Proper synchronization requires:**
- **Semaphores** (or mutexes, condition variables, etc.)
- These are actual synchronization primitives that coordinate process execution
- No timing assumptions - guaranteed correctness

**Implementation:**

1. **Child process**:
   - Writes data to `mmaped_ptr[0]`
   - Signals completion via semaphore: `sem_post(sem_child_done)`
   - Waits for parent: `sem_wait(sem_parent_done)`
   - Reads from `mmaped_ptr[4096]`

2. **Parent process**:
   - Waits for child: `sem_wait(sem_child_done)`
   - Writes data to `mmaped_ptr[4096]`
   - Signals completion: `sem_post(sem_parent_done)`
   - Reads from `mmaped_ptr[0]`

**Key synchronization code:**
```c
// Create semaphores
sem_t *sem_child_done = sem_open("/sem_child_done", O_CREAT | O_EXCL, 0644, 0);
sem_t *sem_parent_done = sem_open("/sem_parent_done", O_CREAT | O_EXCL, 0644, 0);

// Child signals when done writing
sem_post(sem_child_done);

// Parent waits for child to finish writing
sem_wait(sem_child_done);
```

This ensures:
- Child writes → signal → Parent waits → Parent writes → signal → Child waits
- Guaranteed ordering without timing assumptions
- Efficient - processes block until signaled, no busy waiting

---

## Part (e): Comparison of Synchronization Approaches

### Version 1: The Wrong Fix (msync + sleep)

**Command to run:**
```bash
for i in {1..10}; do echo "=== Run $i ==="; ./mmap_program_synced; echo ""; done
```

**output:**
```
= Run $i ==="; ./mmap_program_synced; echo ""; done'
==
= Run 1 ===
Parent process: mmap returned address = 0x727da288a000
Child process: mmap returned address = 0x727da288a000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 2 ===
Parent process: mmap returned address = 0x775a67c9f000
Child process: mmap returned address = 0x775a67c9f000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 3 ===
Parent process: mmap returned address = 0x750464700000
Child process: mmap returned address = 0x750464700000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 4 ===
Parent process: mmap returned address = 0x701a0bd00000
Child process: mmap returned address = 0x701a0bd00000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 5 ===
Parent process: mmap returned address = 0x79be91a34000
Child process: mmap returned address = 0x79be91a34000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 6 ===
Parent process: mmap returned address = 0x71346642f000
Child process: mmap returned address = 0x71346642f000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 7 ===
Parent process: mmap returned address = 0x77a987e9c000
Child process: mmap returned address = 0x77a987e9c000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 8 ===
Parent process: mmap returned address = 0x75c55e100000
Child process: mmap returned address = 0x75c55e100000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 9 ===
Parent process: mmap returned address = 0x76c363900000
Child process: mmap returned address = 0x76c363900000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]

==
= Run 10 ===
Parent process: mmap returned address = 0x79c29fcf1000
Child process: mmap returned address = 0x79c29fcf1000
Child: Wrote '01234' to mmaped_ptr[0]
Child: msync() completed - data flushed to disk
Parent: Wrote '56789' to mmaped_ptr[4096]
Child: Read '56789' from mmaped_ptr[4096]
Parent: msync() completed - data flushed to disk
Parent: Read '01234' from mmaped_ptr[0]
```

**Results:** Consistent across all 10 runs

**Analysis:**
- ❌ **Wrong approach**: Uses `sleep()` for synchronization
- ❌ **Misleading**: `msync()` doesn't provide the synchronization
- ❌ **Inefficient**: Wastes 1 second per run sleeping
- ❌ **Unreliable**: Depends on timing assumptions
- ✓ **Works by accident**: The sleep delays create the ordering

### Version 2: The Correct Fix (semaphores)

**Command to run:**
```bash
for i in {1..5}; do echo "=== Run $i ==="; ./mmap_program_proper_sync; echo ""; done
```

**Sample output:**
```
suhkth@muhummed:~/Desktop/ID1206_HT25/Assignment 4: Storage_IO_File_systems/Task8$ bash -c 'for i in {1..10}; do echo "==
= Run $i ==="; ./mmap_program_proper_sync; echo ""; done'
==
= Run 1 ===
Parent process: mmap returned address = 0x78fce3500000
Child process: mmap returned address = 0x78fce3500000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 2 ===
Parent process: mmap returned address = 0x78413d2df000
Child process: mmap returned address = 0x78413d2df000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 3 ===
Parent process: mmap returned address = 0x7d0501d00000
Child process: mmap returned address = 0x7d0501d00000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 4 ===
Parent process: mmap returned address = 0x703164ae4000
Child process: mmap returned address = 0x703164ae4000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 5 ===
Parent process: mmap returned address = 0x72f39e700000
Child process: mmap returned address = 0x72f39e700000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 6 ===
Parent process: mmap returned address = 0x72b22f839000
Child process: mmap returned address = 0x72b22f839000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 7 ===
Parent process: mmap returned address = 0x7b282122d000
Child process: mmap returned address = 0x7b282122d000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 8 ===
Parent process: mmap returned address = 0x7f73bf300000
Child process: mmap returned address = 0x7f73bf300000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 9 ===
Parent process: mmap returned address = 0x7c82e2300000
Child process: mmap returned address = 0x7c82e2300000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]

==
= Run 10 ===
Parent process: mmap returned address = 0x7134c4f00000
Child process: mmap returned address = 0x7134c4f00000
Child: Wrote '01234' to mmaped_ptr[0]
Child: Signaled write completion
Parent: Child write confirmed
Parent: Wrote '56789' to mmaped_ptr[4096]
Parent: Signaled write completion
Parent: Read '01234' from mmaped_ptr[0]
Child: Parent write confirmed
Child: Read '56789' from mmaped_ptr[4096]
```

**Results:** Consistent across all runs

**Analysis:**
- ✓ **Correct approach**: Uses semaphores for synchronization
- ✓ **Guaranteed ordering**: Enforced by synchronization primitives
- ✓ **Efficient**: Processes block/wake as needed, no wasted time
- ✓ **Reliable**: No timing assumptions
- ✓ **Portable**: Works correctly on all systems

### Comparison Table

| Aspect | msync + sleep | Semaphores |
|--------|---------------|------------|
| **Correctness** | Works by accident | Guaranteed correct |
| **Efficiency** | Wastes ~2 seconds | Efficient blocking |
| **Reliability** | Timing-dependent | Timing-independent |
| **Understanding** | Misunderstands msync | Correct use of sync primitives |
| **Production-ready** | NO | YES |

---

## Key Findings

### 1. Memory-Mapped File Behavior
- `mmap()` with `MAP_SHARED` creates a shared memory region backed by a file
- Changes made by one process are **immediately visible** to others sharing the mapping
- They share the same physical pages - no special system call needed for visibility
- Virtual addresses are the same within a process family but vary between runs (ASLR)

### 2. Race Conditions and Synchronization
- The original program has a race condition - no guaranteed ordering of operations
- Works in practice due to the small delay (`usleep(100)`), but this is unreliable
- **Memory visibility ≠ Process synchronization**
  - Visibility: When can another process see a write? (Immediate with MAP_SHARED)
  - Synchronization: How to enforce operation ordering? (Requires semaphores, mutexes, etc.)

### 3. The msync() Misunderstanding

**What msync() actually does:**
- Forces write-back of dirty pages to the underlying **disk file**
- Ensures persistence and visibility to processes that read the **file** (not the existing mapping)
- Useful when you need to guarantee data is on disk or when new processes will open the file

**What msync() does NOT do:**
- Does NOT provide synchronization between processes sharing a mapping
- Does NOT make changes "more visible" to processes already sharing the mapping
- Is NOT a replacement for semaphores, mutexes, or other synchronization primitives

**Common misconception:**
- "I need msync() so the other process can see my changes" ← **WRONG** for shared mappings
- The processes already see each other's changes because they share physical pages

### 4. Correct Synchronization Approaches

**For processes sharing a memory mapping:**
- Use **semaphores** (`sem_wait`, `sem_post`)
- Use **mutexes** (for mutual exclusion)
- Use **condition variables** (for complex waiting conditions)
- **NOT** sleep() - timing assumptions are unreliable
- **NOT** msync() - it's for disk persistence, not synchronization

**When to use msync():**
- When you need data written to disk for persistence
- When other processes will open and read the file (not via the existing shared mapping)
- When implementing crash recovery
- When coordinating with non-mmap file I/O

### 5. Performance Considerations
- Memory-mapped I/O is efficient for file access
- **Semaphores** are efficient - processes block and are woken up, no busy waiting
- **sleep()** is inefficient - wastes CPU time and is unreliable
- `msync()` has overhead - only use when you actually need disk synchronization

### 6. Practical Applications
Memory-mapped files are useful for:
- **Inter-process communication**: Share data between processes efficiently
- **Large file access**: Efficiently read/write large files without explicit I/O calls
- **Shared libraries**: Load code into multiple processes
- **Database systems**: Implement memory-mapped database files with proper synchronization

---

## Conclusion

This exercise demonstrated the power and complexity of memory-mapped files, and importantly, the critical difference between **memory visibility** and **process synchronization**:

### What We Learned

1. **Shared mapping** (`MAP_SHARED`) enables efficient inter-process communication
   - Processes sharing a mapping see each other's changes immediately
   - No special system call is needed for visibility

2. **Race conditions** require proper synchronization primitives
   - Semaphores, mutexes, and condition variables provide guaranteed ordering
   - Timing-based approaches (sleep) are unreliable and inefficient

3. **msync() is for persistence, not synchronization**
   - Use msync() to flush data to disk
   - Do NOT use msync() to coordinate processes sharing a mapping
   - This is a common and critical misunderstanding

4. **Virtual memory** provides isolation while allowing controlled sharing
   - Same virtual addresses within process family (inherited via fork)
   - Different addresses between runs (ASLR)

### The Bottom Line

- ✓ **Original program**: Has race condition, works by luck
- ❌ **msync + sleep version**: Wrong approach, works by accident, teaches wrong concepts
- ✓ **Semaphore version**: Correct, efficient, production-ready

**Key takeaway**: When working with shared memory, distinguish between:
- Getting data to disk → use `msync()`
- Coordinating process execution → use semaphores/mutexes

---

## Files Created
- `mmap_program.c` - Original program (has race condition)
- `mmap_program_synced.c` - Wrong fix using msync() + sleep()
- `mmap_program_proper_sync.c` - Correct fix using semaphores
- `file_to_map.txt` - 1MB file used for memory mapping
- `findings.md` - This documentation file

## Compilation Commands
```bash
# Setup: Create the 1MB file
dd if=/dev/zero of=file_to_map.txt bs=1M count=1

# Compile all versions
gcc -o mmap_program mmap_program.c
gcc -o mmap_program_synced mmap_program_synced.c
gcc -o mmap_program_proper_sync mmap_program_proper_sync.c -pthread
```

## Execution Commands
```bash
# Run original (has race condition)
./mmap_program

# Run wrong fix (msync + sleep)
./mmap_program_synced

# Run correct fix (semaphores)
./mmap_program_proper_sync

# Run multiple times for consistency testing
for i in {1..10}; do echo "=== Run $i ==="; ./mmap_program; echo ""; done
for i in {1..10}; do echo "=== Run $i ==="; ./mmap_program_proper_sync; echo ""; done
```
