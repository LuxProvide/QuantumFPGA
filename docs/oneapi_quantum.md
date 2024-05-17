- We will develop the quantum circuit we have seen previously, i.e.,  Bernstein-Varizani to search an hidden element
- We will need the Pauli H and Z gates 
- We will also need a function to measure (sample) from the distribution obtained using the state vector amplitudes

## Setup

- We need first to obtain an interactive job on the fpga partition and load some modules

```bash
# Get one FPGA node with two FPGA cards
salloc -A <ACCOUNT> -t 02:00:00 -q default -p fpga -N1
# Load the PyOpenCL module 
module load env/staging/2023.1
module load CMake intel-fpga 520nmx
```

- Clone the repository if not already done: `git clone https://github.com/LuxProvide/QuantumFPGA`

- Create a symbolic link to the project `ln -s QuantumFPGA/code/QFSIM` and `cd QFSIM`


