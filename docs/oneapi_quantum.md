- We will develop the quantum circuit we have seen previously, i.e.,  Bernstein-Varizani to search an hidden element
- We will need to code the Pauli H and Z gates 
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

- Create a symbolic link to the project `ln -s QuantumFPGA/code/FQSim` and `cd FQSim`

- The project contains the following files:

```bash
$>tree
.
├── CMakeLists.txt
├── fpga_image
│   └── quantum.fpga
├── README.md
├── src
│   ├── bernstein-vazirani.cpp
│   ├── kernels.cpp
│   └── kernels.hpp
└── src-solution
    ├── bernstein-vazirani.cpp
    ├── kernels.cpp
    └── kernels.hpp
```

- **fpga_image** : contains the fpga image build prior to the workshop training to avoid waiting hardware synthesis. Indeed, the offline compiler will therefore extract the bitstream file `aocx` and reuse it if only if the device code did not change
- **src** : All files contain blank code that we are going to fill step by step
    * **bernstein-vazirani.cpp**: the source file with the Bernstein-Vazirani circuit.
    * **kernel.cpp**: the source file containing all code for the gates.
    * **kernel.hpp**: the header file containing the signature of function.
- **src-solution**:  the solution to fill all blank code. Replace `set(SOURCE_FILES src/bernstein-vazirani.cpp src/kernels.cpp)` by `set(SOURCE_FILES src/bernstein-vazirani.cpp src/kernels.cpp)` in the CMakeLists.txt file

## Building code

- We strongly recommend to compile and execute your code using the `Intel(R) FPGA Emulation Platform` which does not require any FPGA board on the system.

```bash
mkdir build-emu && cd build-emu
cmake ..
make fpga_emu
```

- Once your code runs on the `Intel(R) FPGA Emulation Platform` without errors:

```bash
mkdir build-fpga && cd build-fpga
cmake ..
make fpga
```

## Device code

- The highlighted code corresponds to the device code or kernel code running on the device

- Device code is mainly used to modify or accelerate operation related to the state vector

- We are not going to modify this code but we will use it to create the two gates

```cpp title="kernels.cpp" linenums="1" hl_lines="34 35 36 37 38 39 40 61 62 63"
--8<-- "./code/FQSim/src/kernels.cpp"
```












