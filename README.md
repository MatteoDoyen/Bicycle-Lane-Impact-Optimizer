# Bicycle Lane Impact Optimizer
## Overview
The Bicycle Lane Impact Optimizer is an innovative solution developed as part of a collaborative student research project under the guidance of Mr. Néron, Mr Repusseau and Ms. Rault. 

This solution is designed to maximize the benefits of introducing bicycle lanes within urban environments. Utilizing advanced algorithms, this software tool analyzes road networks to identify the most impactful roads for bicycle lane improvements under constrained budgets. 

The output is a strategic plan that prioritizes road improvements based on their potential impact, facilitating efficient resource allocation for urban planners and transportation authorities.


## System Requirements

### Windows
- Supported OS: Windows 10
- Required Tools: Cygwin (for POSIX compatibility), Make, GCC (GNU Compiler Collection), pthreads (POSIX Threads)

### Linux
- Compatible with most Linux distributions
- Required Packages: GCC, Make

## Dependencies
### External Libraries
- Unity: A lightweight, powerful unit testing framework for C.
- CJson: A robust C library for parsing JSON data.

### POSIX Threads
- Windows: POSIX Thread support is facilitated through Cygwin.
- Linux: Native support for pthreads.
### OpenMP

- Windows: OpenMP support is enabled through Cygwin by installing the appropriate GCC package that includes OpenMP.
- Linux: Most GCC installations include OpenMP support by default. Ensure your GCC version supports OpenMP.

## Installation Guide
### Setting Up the Environment
#### Windows

Cygwin Installation: Install Cygwin from the [official Cygwin website](https://www.cygwin.com/). During installation, select the gcc, make, pthread, and libgomp (for OpenMP support) packages.

#### Linux

Installing Dependencies: Use the following command to install the necessary packages:

```sh
sudo apt install make gcc -y
```
### Compilation Instructions
Navigate to the project directory.
Execute the make all command to compile the project. This will generate the executable file BikePathOptimizer.
### Execution
To run the Bicycle Lane Impact Optimizer, use the following command:
```sh
./BikePathOptimizer <configuration_file>
```
Where <configuration_file> is the path to a JSON configuration file specifying the parameters and data for the analysis.

## Output
The program generates a file containing:

- The IDs of the recommended roads for improvement.
- The estimated impact of improving each road.

This output enables decision-makers to strategically plan bicycle lane implementations with a clear understanding of their potential benefits.
