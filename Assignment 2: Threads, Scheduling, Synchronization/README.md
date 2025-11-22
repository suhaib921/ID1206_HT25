# Operating Systems Assignment

This repository contains answers, explanations, and programming implementations for an Operating Systems assignment. The tasks include CPU scheduling, deadlock analysis, Banker's algorithm, and multi-threaded programming in C using POSIX threads.

---

## **Question 1 — Scheduling Algorithms (1 pt)**

Consider the following set of processes, with the length of the CPU burst is given in milliseconds. A smaller priority number implies a higher priority.


| Process | Arrival Time | Burst Time | Priority |
|--------|--------------|------------|----------|
| P1     | 0            | 8          | 4        |
| P2     | 1            | 3          | 3        |
| P3     | 2            | 5          | 2        |
| P4     | 8            | 1          | 0        |

A smaller priority number implies a higher priority.

For each algorithm below:

- FCFS  
- SJF  
- Preemptive SRTF  
- Non-preemptive Priority Scheduling  
- Round-Robin (quantum = 4)

You must:

1. Draw the **Gantt chart** to illustrate the execution of these schedule.  
2. Compute the **turnaround time** of each process  
3. Compute the **waiting time** of each process  
4. Identify which algorithm yields the **minimum average waiting time**

---

## **Question 2 — Scheduler Criteria Conflicts (1 pt)**

Provide one concrete example illustrating when the following scheduling criteria conflict:

1. Average turnaround time vs. Maximum waiting time  
2. Response time vs. Maximum waiting time  
3. System throughput vs. Response time  

---

## **Question 3 — Resource-Allocation Graph (1 pt)**

Recall that a resource-allocation graph without cycles has no deadlock, and a resource-allocation graph with cycles may or may not have deadlocks. In the following resource-allocation graphs:
![Banker's Algorithm Table](bankers_)


Tasks:

1. Please describe thread states in each graph. For example, thread T0 is requesting an instance of resource R0, thread T1 is holding an instance of resource R1.

2. Which graph(s) have no deadlocks? Please illustrate the order in which the threads may complete execution. For example, thread T0 releases an instance of resource R0, and then an instance of resource R0 is allocated to thread T1.

3. Which graph may have deadlocks? Recall that a cycle in the resource-allocation graph does not necessarily mean deadlock. For those situations that are deadlocked, provide the cycle of threads and resources.
   (e.g., `{T0 → R0 → T1 → R1 → T0}`)

---

## **Question 4 — Banker's Algorithm (1 pt)**

Given a system snapshot:

![Banker's Algorithm Table](bankers_table.png)

1. List the **number of instances** per resource  
2. Compute the **Need matrix**  
3. Evaluate whether the request `(0, 4, 2, 0)` from process **P1** can be granted immediately  
4. Run the **Banker's Safety Algorithm**  
5. Determine if the system is in a **safe state**  
6. Identify a **safe sequence**

---

## **Question 5 — Minimum Resources for Safety (1 pt)**

Given the system snapshot:

- Determine the **minimum value of x** for which the state remains safe  
- Use the **Need matrix** and check which sequence of processes can complete when `x` is large

---

# Programming Section

---

## **Question 6 — Threaded Array Summation (1 pt)**

Modify `pthread_sum.c` to parallelize summation of an array.

### Requirements:

- Generate an array of **1,000,000 random floats in [0,1]**
- Compute:
  - **Serial sum**  
  - **Parallel sum** using `num_threads = argv[1]`
- Each thread sums a **portion** of the array
- Master thread aggregates results
- Add timers for:
  - Serial execution time  
  - Parallel execution time
- Run with **4, 8, 12, and 16 threads**
- Include:
  - Screenshots of output  
  - A bar chart comparing performance  
  - A short analysis

---

## **Question 7 — Multi-threaded Histogram (1 pt)**

Extend the program to compute a histogram with **30 bins**.

### Requirements:

- Array length given by `argv[2]`
- Values are random doubles in `[0.0, 1.0]`
- Implement:
  - **Serial histogram** (print histogram + time)
  - **Parallel histogram** (each thread handles a segment)
- Use shared data structures with proper synchronization
- Run:

