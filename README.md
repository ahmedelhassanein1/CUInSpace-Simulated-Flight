# CUInSpace Simulated Flight

A multithreaded rocket systems simulation written in C, modelling resource consumption, production, and coordination between onboard systems. Developed as a course project for COMP 2401: Introduction to Systems Programming at Carleton University.

The simulation models several interconnected rocket subsystems that operate concurrently, exchange shared resources, and respond to resource availability through a centralized event-driven manager.

## Overview

The simulation represents a rocket as a collection of systems that consume and produce resources over time. Each system operates in its own thread, while a manager monitors system events and adjusts operating modes based on resource levels and capacity constraints.

The simulation terminates when the rocket reaches its configured distance capacity or its oxygen supply is depleted.

### Simulated Systems

| System | Resource Input | Resource Output |
|---|---|---|
| Propulsion | Fuel | Distance |
| Life Support | Energy | Oxygen |
| Crew Capsule | Oxygen | None |
| Generator | Fuel | Energy |

Each system follows a configurable recipe defining its input requirements, output quantities, and processing time.

## Key Features

- **Multithreaded execution:** Runs individual rocket systems alongside a centralized manager using POSIX threads (`pthreads`).
- **Event-driven coordination:** Uses a shared, priority-ordered linked-list queue to communicate resource shortages, capacity limits, and system status changes.
- **Resource management:** Tracks shared resource quantities and maximum capacities, with synchronized access to prevent concurrent resource modifications.
- **Dynamic memory management:** Uses dynamically allocated structures for resources and systems, along with resizable arrays and a linked-list event queue.
- **Adaptive operating modes:** Adjusts system processing speeds in response to resource availability and capacity events.
- **Terminal-based monitoring:** Displays resource levels, subsystem operating modes, and a rolling event log as the simulation runs.

## Technical Concepts

This project applies several core systems programming concepts in C:

- **Concurrency and synchronization:** POSIX threads and semaphores coordinate concurrent operations on shared resources and the event queue.
- **Dynamic data structures:** A linked-list priority queue and dynamically resized arrays manage events, resources, and systems.
- **Memory lifecycle management:** Explicit allocation, initialization, cleanup, and deallocation of dynamically allocated structures.
- **Modular program design:** Separates simulation management, system execution, event handling, resource management, and terminal display logic across multiple source files.

## Architecture

The simulation is organized into several components:

| Component | Responsibility |
|---|---|
| `main.c` | Initializes the simulation, starts worker threads, joins threads, and performs final cleanup. |
| `manager.c` | Processes queued events, monitors simulation conditions, and adjusts subsystem operating modes. |
| `system.c` | Executes individual system recipes, consumes and produces resources, and generates events. |
| `event.c` | Implements event initialization and priority-ordered queue operations. |
| `resource.c` | Manages resource storage, transfers, recipes, and dynamically resized resource arrays. |
| `display.c` | Renders resource levels, system modes, and event activity in the terminal. |
| `defs.h` | Defines shared data structures, constants, event statuses, and function interfaces. |

The manager and subsystem threads communicate through shared state and a centralized event queue. Semaphores protect resource transfers and queue operations, while events allow the manager to respond to changes in the simulation.

## Requirements

- Linux or another compatible POSIX environment
- GCC
- GNU Make
- POSIX threads and semaphores

The project was developed in a Linux environment and uses POSIX-specific APIs. A Linux virtual machine, Linux installation, or compatible WSL environment is suitable for building and running it.

## Build and Run

Clone the repository and enter the project directory:

```bash
git clone https://github.com/ahmedelhassanein1/CUInSpace-Simulated-Flight.git
cd CUInSpace-Simulated-Flight
```

**Optional:** To use a custom folder name when cloning, specify it at the end of the `git clone` command. For example:

```bash
git clone https://github.com/ahmedelhassanein1/CUInSpace-Simulated-Flight.git Simulated-Flight
cd Simulated-Flight
```

Compile the program using the provided Makefile:

```bash
make
```

Run the simulation:

```bash
./proj2
```

The program displays the current resource levels, operating modes, and events as the simulation progresses.

To remove the generated executable and object files:

```bash
make clean
```

## Project Context

This project was developed individually as part of COMP 2401: Introduction to Systems Programming at Carleton University.

The implementation builds on concepts covered in the course, including linked lists, dynamic arrays, resource management, and multithreaded program design.
