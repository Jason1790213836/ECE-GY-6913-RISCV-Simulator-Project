# ECE-GY-6913-RISCV-Simulator-Project
A cycle-accurate RISC-V processor simulator featuring a five-stage pipeline with RAW hazard handling, control hazard resolution, and CPI analysis.

This project implements a RISC-V instruction set simulator with both
a single-cycle processor and a five-stage pipelined processor.
The pipelined core supports RAW hazard resolution via forwarding and stalling,
as well as control hazard handling using static not-taken branch prediction.
Performance metrics such as CPI and IPC are measured and reported
