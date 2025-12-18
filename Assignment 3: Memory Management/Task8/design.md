# Task 8: Page Reclamation - Design Description

## Summary (50 words)
Two-threaded simulation of Linux's active/inactive page reclamation. Player thread accesses random pages, setting reference bits and managing lists with 70%/20% threshold. Checker thread periodically scans active list, clearing reference bits. Single mutex ensures thread safety. Demonstrates accuracy vs. overhead trade-off in page replacement algorithms.

---

## Architecture Overview

### Data Structures

#### Page Node Structure
```c
typedef struct page {
    int page_id;           // Unique identifier (0 to N-1)
    int reference_bit;     // Hardware-simulated reference bit (0 or 1)
    int total_referenced;  // Statistics counter
    struct page *next;     // Pointer for linked list
} Node;
```

#### List Structure
```c
typedef struct list {
    Node *head;    // Front of the list (oldest pages)
    Node *tail;    // Rear of the list (newest pages)
    int size;      // Current number of pages
} List;
```

---

## Thread Designs

### Player Thread Flowchart

```
┌─────────────────────────────────────────────────────────┐
│                    START PLAYER THREAD                   │
└────────────────────┬────────────────────────────────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ For i = 0 to 999      │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ page_id =             │
         │ reference_string[i]   │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ Lock mutex            │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────────┐
         │ Find page in active list  │────── Found ────────┐
         └───────────┬───────────────┘                      │
                     │                                      │
                  Not Found                                 │
                     │                                      │
                     ▼                                      │
         ┌───────────────────────────┐                     │
         │ Find page in inactive     │─── Found ───────────┤
         │         list              │                     │
         └───────────┬───────────────┘                     │
                     │                                     │
                  Not Found                                │
                     │                                     │
                     ▼                                     │
         ┌───────────────────────┐                        │
         │ Create new page node  │                        │
         └───────────┬───────────┘                        │
                     │                                     │
                     └──────────────┬──────────────────────┘
                                    ▼
                        ┌───────────────────────┐
                        │ Set reference_bit = 1 │
                        └───────────┬───────────┘
                                    │
                                    ▼
                        ┌───────────────────────┐
                        │ Add to rear of        │
                        │   ACTIVE list         │
                        └───────────┬───────────┘
                                    │
                                    ▼
                        ┌───────────────────────┐
                        │ active_list->size     │
                        │    > 0.7 * N ?        │
                        └───────────┬───────────┘
                                    │
                             Yes    │    No
                        ┌───────────┴───────────┐
                        ▼                       ▼
        ┌───────────────────────────┐    ┌──────────────┐
        │ pages_to_move = 0.2 * N   │    │ Unlock mutex │
        └───────────┬───────────────┘    └──────┬───────┘
                    │                            │
                    ▼                            │
        ┌───────────────────────────┐           │
        │ For j = 0 to              │           │
        │   pages_to_move:          │           │
        │  - Remove from front of   │           │
        │    active list            │           │
        │  - Add to rear of         │           │
        │    inactive list          │           │
        └───────────┬───────────────┘           │
                    │                            │
                    ▼                            │
        ┌───────────────────────────┐           │
        │ Unlock mutex              │           │
        └───────────┬───────────────┘           │
                    │                            │
                    └────────────┬───────────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ usleep(10 μs)          │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ Loop continues         │
                    │ until i = 1000         │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ Lock mutex             │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ Set player_done = 1    │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ Unlock mutex           │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │ pthread_exit(0)        │
                    └────────────────────────┘
```

---

### Checker Thread Flowchart

```
┌─────────────────────────────────────────────────────────┐
│                   START CHECKER THREAD                   │
└────────────────────┬────────────────────────────────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ Infinite Loop:        │
         │ while (1)             │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ usleep(M microseconds)│
         │                       │
         │ [Wait for M μs]       │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ Lock mutex            │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ curr = active_list->  │
         │        head           │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────────┐
         │ While (curr != NULL)  │◄──────────────────┐
         └───────────┬───────────┘                   │
                     │                               │
                  curr != NULL                       │
                     │                               │
                     ▼                               │
         ┌───────────────────────┐                  │
         │ curr->reference_bit   │                  │
         │      == 1 ?           │                  │
         └───────────┬───────────┘                  │
                     │                               │
              Yes    │    No                         │
         ┌───────────┴───────────┐                  │
         ▼                       ▼                   │
┌────────────────────┐  ┌────────────────────┐     │
│ curr->total_       │  │ Move to next node: │     │
│   referenced++     │  │ curr = curr->next  │─────┤
└────────┬───────────┘  └────────────────────┘     │
         │                                          │
         ▼                                          │
┌────────────────────┐                             │
│ curr->             │                             │
│   reference_bit=0  │                             │
└────────┬───────────┘                             │
         │                                          │
         ▼                                          │
┌────────────────────┐                             │
│ curr = curr->next  │─────────────────────────────┘
└────────────────────┘
         │
         │ (curr == NULL, exit while loop)
         │
         ▼
┌────────────────────────┐
│ Check player_done flag │
└────────────┬───────────┘
             │
      Yes    │    No
┌────────────┴────────────┐
▼                         ▼
┌────────────────────┐  ┌────────────────────┐
│ Unlock mutex       │  │ Unlock mutex       │
└────────┬───────────┘  └────────┬───────────┘
         │                       │
         ▼                       │
┌────────────────────┐          │
│ break (exit loop)  │          │
└────────┬───────────┘          │
         │                       │
         ▼                       │
┌────────────────────┐          │
│ pthread_exit(0)    │          │
└────────────────────┘          │
                                 │
                                 │ (Loop back to top)
                                 │
                                 └──────────┐
                                            │
                                            ▼
                               ┌────────────────────────┐
                               │ usleep(M microseconds) │
                               │ [Next iteration]       │
                               └────────────────────────┘
```

---

## Key Design Elements

### 1. Synchronization
- **Single mutex** protects both lists and player_done flag
- **Coarse-grained locking** for simplicity and correctness
- No deadlock risk with single lock

### 2. List Management
- **FIFO demotion**: Oldest pages in active → inactive
- **MRU promotion**: New accesses → rear of active list
- **O(1) operations** at head/tail using double-ended list

### 3. Page Aging Strategy
- Checker scans **only active list** (inactive pages already cold)
- **Clear-on-scan**: Reference bits reset each scan
- **Periodic execution**: Configurable M parameter

### 4. Threshold Policy
```
Trigger:  active_list->size > 70% of N
Action:   Move 20% of N pages from active → inactive
Method:   Remove from front of active, add to rear of inactive
```

### 5. Reference Tracking
- **Binary reference bit**: 1 = accessed, 0 = not accessed
- **Lossy tracking**: Multiple accesses between scans = one count
- **Statistics**: total_referenced accumulates detections

---

## Algorithm Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Add to rear | O(1) | - |
| Remove from front | O(1) | - |
| Find page | O(n) | - |
| Scan active list | O(active_size) | - |
| Total memory | - | O(N) |

---

## Termination Protocol

```
Player Thread:               Checker Thread:
  [Finish 1000 accesses]      [In main loop]
         │                           │
         ▼                           ▼
  Lock mutex                   usleep(M μs)
         │                           │
         ▼                           ▼
  player_done = 1              Lock mutex
         │                           │
         ▼                           ▼
  Unlock mutex                 Scan active list
         │                           │
         ▼                           ▼
  pthread_exit(0)              Check player_done
                                     │
                                     ▼
                                player_done == 1?
                                     │
                                   Yes
                                     │
                                     ▼
                                Unlock mutex
                                     │
                                     ▼
                                   break
                                     │
                                     ▼
                                pthread_exit(0)
```

---

## Design Trade-offs

### Strengths
✅ Realistic simulation of Linux page reclamation
✅ Thread-safe with proper synchronization
✅ Configurable parameters (N, M)
✅ Observable statistics for analysis
✅ Simple and understandable

### Limitations
❌ O(n) page lookup (could use hash table)
❌ Coarse-grained locking limits parallelism
❌ Uniform random workload (not realistic locality)
❌ No actual page eviction implemented
❌ Fixed 1000 access sequence

---

## Real-World Mapping

| Simulation Component | Real Linux Equivalent |
|---------------------|----------------------|
| Player thread | User processes accessing memory |
| Checker thread | kswapd daemon |
| Active list | /proc/meminfo: Active(anon) + Active(file) |
| Inactive list | /proc/meminfo: Inactive(anon) + Inactive(file) |
| reference_bit | PTE accessed bit (hardware) |
| M parameter | vm.stat_interval sysctl |
| 70%/20% threshold | vm.watermark_scale_factor |

---

## Conclusion

This design effectively demonstrates the **two-list LRU approximation** used in modern operating systems. The checker thread's periodic scanning models how OS kernels balance accuracy (frequent scanning) against overhead (CPU cycles). The configurable M parameter allows exploration of this fundamental trade-off in memory management.
