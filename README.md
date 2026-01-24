# ECE-GY-6913-RISCV-Simulator-Project
A cycle-accurate RISC-V processor simulator featuring a five-stage pipeline with RAW hazard handling, control hazard resolution, and CPI analysis.

This project implements a RISC-V instruction set simulator with both
a single-cycle processor and a five-stage pipelined processor.
The pipelined core supports RAW hazard resolution via forwarding and stalling,
as well as control hazard handling using static not-taken branch prediction.
Performance metrics such as CPI and IPC are measured and reported

# Feature

Single-Cycle Processor （Phase1）

Executes one instruction per clock cycle

Simple baseline implementation for performance comparison

Five-Stage Pipelined Processor （Phase2）

Instruction Fetch (IF), Instruction Decode/Register Read (ID/RR), Execute (EX), Memory Access (MEM), Write Back (WB)

Pipeline registers with NOP bits for stalling and flushing

Supports EX→ID and MEM→ID forwarding

Stalls automatically when forwarding cannot resolve RAW hazards

Control Hazard Handling

Branches predicted not-taken

Branch resolution in ID/RR stage

Speculative instructions discarded and NOP inserted on misprediction

Performance Monitoring

Measures Total Cycles, CPI, and IPC

Compare single-cycle vs pipelined execution

# This is the description for the difference between the single cycle cpu and the five-stage processor.

This is the pipeline with the exception diagram.

<img width="553" height="311" alt="image" src="https://github.com/user-attachments/assets/5b268c3a-e139-4cdd-9b41-a92208e2a247" />

2: The first single cycle test code is only four cycles, but each cycle is really long. Compared to the five-stage pipeline, which has 10 cycles, its cycle number is bigger but the cycle time is smaller due to its division into five stages.

The second test code in the five-stage pipeline is only like 0.46, but the single-cycle stage is like 37. As I said before, the pipeline is much better because every cycle duration is greatly reduced.

The third test code is 0–52 cycles in the pipeline because it implemented the forwarding module and hazard detection module. The single-cycle of this test is 33 cycles. I think the pipeline is better because the CPI is 1.5, but the cycle time is reduced.

For the extra point, I think we can implement branch anticipation, which can greatly save the flush time and further reduce the cycle count.
