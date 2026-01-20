# ECE-GY-6913-RISCV-Simulator-Project
A cycle-accurate RISC-V processor simulator featuring a five-stage pipeline with RAW hazard handling, control hazard resolution, and CPI analysis.

This project implements a RISC-V instruction set simulator with both
a single-cycle processor and a five-stage pipelined processor.
The pipelined core supports RAW hazard resolution via forwarding and stalling,
as well as control hazard handling using static not-taken branch prediction.
Performance metrics such as CPI and IPC are measured and reported

# Feature

Single-Cycle Processor

Executes one instruction per clock cycle

Simple baseline implementation for performance comparison

Five-Stage Pipelined Processor

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
