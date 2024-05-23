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
module load git-lfs
module load CMake intel-fpga 520nmx
```

- Clone the repository if not already done: `git lfs clone https://github.com/LuxProvide/QuantumFPGA`

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
- **src-solution**:  the solution to fill all blank code. Replace `set(SOURCE_FILES src/bernstein-vazirani.cpp src/kernels.cpp)` by `set(SOURCE_FILES src-solution/bernstein-vazirani.cpp src-solution/kernels.cpp)` in the CMakeLists.txt file

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

!!! warning "Using Direct Memory Access (DMA)"

    * `module load jemalloc/
    * DMA is enabled between host and the device if buffer data have a 64-byte alignment.
    * Using pyopencl, we strongly recommend you to load our `jemalloc` module which provides such default alignment:
    ```bash
    module load jemalloc/5.3.0-gcccore-12.3.0
    export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
    LD_PRELOAD=${JEMALLOC_PRELOAD} ./exe
    ```

## Device code

- The highlighted code corresponds to the device code or kernel code running on the device

- Device code is mainly used to modify or accelerate operation related to the state vector. We are going to explain it in a few minutes.

- We are not going to modify this code but we will use it to create the two required gates

```cpp title="kernels.cpp" linenums="1" hl_lines="34 35 36 37 38 39 40 61 62 63"
--8<-- "./code/FQSim/src/kernels.cpp"
```

### General approach to apply a quantum gate

- Let's consider a multiqubit register $ |\psi \rangle = \sum\limits_{i=0}^{2^N-1} \alpha_i |i \rangle $ which is a mixture of pure state

- $ |i \rangle $ is the decimal representation of the pure state i 

- As you can observe it, the number of pure states constituting the state vector is growing exponentially. 

- We can take advantage of the FPGA to apply gate in parallel 

```cpp title="kernel code" linenums="1"

int nth_cleared(int n, int target)
{
    int mask = (1 << target) - 1;
    int not_mask = ~mask;

    return (n & mask) | ((n & not_mask) << 1);
}

void apply_gate(sycl::queue &queue, std::complex<float> *stateVector_d,const unsigned int numStates, 
                  const int target, 
                  const std::complex<float> A,
                  const std::complex<float> B,
                  const std::complex<float> C,
                  const std::complex<float> D)
{
  queue.parallel_for<class Gate>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
			  int global_id = item.get_id(0);
		      const int zero_state = nth_cleared(global_id,target);
    		  const int one_state = zero_state | (1 << target);
              std::complex<float> zero_amp = stateVector_d[zero_state];
              std::complex<float> one_amp = stateVector_d[one_state];
			  stateVector_d[zero_state] = A * zero_amp + B * one_amp;
			  stateVector_d[one_state] =  C * zero_amp + D * one_amp;

          }).wait();
}
```

**Example:  applying a general U gate to qubit 2**:

- let's consider $U=\begin{pmatrix} u_1 & u_2 \\ u_3 & u_4 \end{pmatrix}$

- Apply $U$ on qubit 2 is $ I \otimes U \otimes I $ with $ I $ being the identity matrix

$$
\begin{aligned}
I \otimes U \otimes I|\psi_1 \psi_2 \psi_3 \rangle & =  |\psi_1 \rangle & \otimes & \hspace{2em} U|\psi_2 \rangle  & \otimes & | \psi_3 \rangle \\
                              & = \begin{pmatrix} \alpha_1  \\ \beta_1 \end{pmatrix} & \otimes & \begin{pmatrix} u_1 & u_2 \\ u_3 & u_4 \end{pmatrix} \begin{pmatrix} \alpha_2  \\ \beta_2 \end{pmatrix} & \otimes & \begin{pmatrix} \alpha_3  \\ \beta_3 \end{pmatrix} \\
                              & = \begin{pmatrix} \alpha_1  \\ \beta_1 \end{pmatrix} & \otimes &  \begin{pmatrix} u_1 \alpha_2 + u_2 \beta_2 \\ u_3 \alpha_2  + u_4 \beta_2 \end{pmatrix} & \otimes & \begin{pmatrix} \alpha_3  \\ \beta_3 \end{pmatrix} \\
                              & = \begin{pmatrix} 
                              \alpha_1 { \color{red}{       u_1 \alpha_2 + u_2 \beta_2 }}  \alpha_3  \\
                              \alpha_1 { \color{blue}{     u_1 \alpha_2 + u_2 \beta_2  }}  \beta_3 \\ 
                              \alpha_1 { \color{red}{       u_3 \alpha_2  + u_4 \beta_2}}  \alpha_3  \\ 
                              \alpha_1 { \color{blue}{     u_3 \alpha_2  + u_4 \beta_2 }}  \beta_3  \\ 
                              \beta_1  { \color{green}{   u_1 \alpha_2 + u_2 \beta_2   }}  \alpha_3  \\ 
                               \beta_1 { \color{purple}{  u_1 \alpha_2 + u_2 \beta_2   }}  \beta_3  \\
                               \beta_1 {  \color{green}{  u_3 \alpha_2  + u_4 \beta_2  }}  \alpha_3  \\
                                \beta_1{  \color{purple}{ u_3 \alpha_2  + u_4 \beta_2  }}  \beta_3
                                \end{pmatrix} 
                              & \begin{matrix} |000 \rangle + \\ |001 \rangle + \\ |010 \rangle + \\|011 \rangle + \\|100 \rangle + \\|101 \rangle + \\|110 \rangle + \\|111 \rangle  \end{matrix}
\end{aligned}
$$

- As you can see to apply a gate U with its 4 complex coefficient, we can proceed by pairs and therefore divide the search by two 

- Nonetheless, we are not applying the tensor product $\otimes$ every time which would be inefficient 

- Starting from specific state vector, we will apply the gate like following:

$$
\begin{aligned}
                               \begin{pmatrix} \alpha_1 {\color{red}{\alpha_2}   } \alpha_3 \\
                                               \alpha_1 { \color{blue}{\alpha_2} } \beta_3  \\
                                               \alpha_1 { \color{red}{\beta_2}   } \alpha_3  \\
                                               \alpha_1 { \color{blue}{\beta_2}  } \beta_3   \\ 
                                               \beta_1  {\color{green}{\alpha_2} } \alpha_3  \\
                                               \beta_1  {\color{purple}{\alpha_2}}  \beta_3   \\
                                               \beta_1  {\color{green}{\beta_2}  }  \alpha_3   \\
                                               \beta_1  {\color{purple}{\beta_2} }  \beta_3    \\
                               \end{pmatrix}
                              & \rightarrow \begin{pmatrix} 
                              \alpha_1 { \color{red}{       u_1 \alpha_2 + u_2 \beta_2 }}  \alpha_3  \\
                              \alpha_1 { \color{blue}{     u_1 \alpha_2 + u_2 \beta_2  }}  \beta_3 \\ 
                              \alpha_1 { \color{red}{       u_3 \alpha_2  + u_4 \beta_2}}  \alpha_3  \\ 
                              \alpha_1 { \color{blue}{     u_3 \alpha_2  + u_4 \beta_2 }}  \beta_3  \\ 
                              \beta_1  { \color{green}{   u_1 \alpha_2 + u_2 \beta_2   }}  \alpha_3  \\ 
                               \beta_1 { \color{purple}{  u_1 \alpha_2 + u_2 \beta_2   }}  \beta_3  \\
                               \beta_1 {  \color{green}{  u_3 \alpha_2  + u_4 \beta_2  }}  \alpha_3  \\
                                \beta_1{  \color{purple}{ u_3 \alpha_2  + u_4 \beta_2  }}  \beta_3
                                \end{pmatrix} 
\end{aligned}
$$

- Let's explain now the two previous functions

    - **nth_cleared**: finds the Nth number where a given binary digit is set to 0. To do so, the hint is ..
    - **apply_gate**:  apply a general one qubit gate by finding in parallel all pure vector with digit 2 set to 0. For each of these vector, we can easily find the one with digit set to 1 and replace the amplitudes according to what we have above 



### Computing probabilities from state vector amplitudes

- The following kernel is only used to compute the probability for each pure state vector

- We compute that on the FPGA card as its is time-consuming

```cpp title="kernel code" linenums="1"
queue.parallel_for<class Proba>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
	  int global_id = item.get_id(0);
            std::complex<float> amp = stateVector_d[global_id];
            probaVector_d[global_id] = std::abs(amp * amp);
        }).wait();
```

## <u>**Measuring qubits**</u>: 

- **Quantum State Collapse**: In classical simulation, measurements typically do not affect the system being measured. However, in quantum simulations, the act of measurement causes the qubit to collapse from its superposition of states to one of the basis states (e.g., 0 or 1). This is a fundamental aspect of quantum mechanics known as wave function collapse.

- **Probabilistic Nature**: The result of measuring a qubit is probabilistic. Before measurement, a qubit in superposition has probabilities associated with its possible states. Measurement forces the qubit into one of these states, with the likelihood of each state given by its quantum amplitude squared.

- In simulation, measuring is a synonym of **sampling** 

!!! tig "Sampling the possible outcomes"
    === "Question"
        - Using the `#!cpp void get_proba(...)` function, fill the body of the `#!cpp void measure(...)` function
        - You can use the standard library function `#!cpp std::discrete_distribution` (see below)
        ```cpp 
        std::random_device rd;                          // Obtain a random number from hardware
        std::mt19937 gen(rd());                         // Seed the generator
        std::discrete_distribution<> dist(probaVector, probaVector + size);
        ```

    === "Solution"
        - Add the following code in the `#!cpp void measure(...)` function body
        ```cpp linenums="1"
         int size = std::pow(2,numQubits);
         float *probaVector = new float[size];
         float *probaVector_d = malloc_device<float>(size,queue);
         get_proba(queue,stateVector_d,size,probaVector_d); 
         queue.memcpy(probaVector, probaVector_d, size * sizeof(float)).wait();
         std::random_device rd;                          // Obtain a random number from hardware
         std::mt19937 gen(rd());                         // Seed the generator
         std::discrete_distribution<> dist(probaVector, probaVector + size);
         std::vector<int> arr(size);
         for(int i = 0; i < samples; i++){
             int index = dist(gen);
             arr[index]++;
         }
         std::cout << "Quantum State Probabilities:" << std::endl;
         for (size_t i = 0; i < size; ++i) {
             std::cout << "State " <<toBinary(i,numQubits) << ": " << arr[i] << std::endl;
         }
         delete[] probaVector;
        ```

## Implementing the two Pauli H and Z gates

!!! tig "Pauli H gate"
    === "Question"
        - The Hadamard gate puts qubits in **superposition**
        - It transform the basis state:
            - $|0 \rangle$ to $\frac{|0\rangle + |1 \rangle}{\sqrt{2}} $
            - $|1 \rangle$ to $\frac{|0\rangle - |1 \rangle}{\sqrt{2}} $

        <div align="center"> $\begin{aligned} H & = \frac{1}{\sqrt{2}}\begin{pmatrix}1 & 1 \\1 & -1 \end{pmatrix}\end{aligned}$ </div>

        - Fill the body of the `#!cpp void h(...)` function body

        - To test your gate, uncomment `set(SOURCE_FILES src/test_h_gate.cpp src/kernels.cpp)` in the CMakeLists.txt file
        !!! info "Building and the code"  
            ```bash
            mkdir build-test-h-gate && cd build-test-h-gate
            cmake ..
            make fpga
            LD_PRELOAD=${JEMALLOC_PRELOAD} ./quantum.fpga
            ```

    === "Solution"
        - Add the following code in the `#!cpp void h(...)` function body
        ```cpp linenums="1"
        std::complex<float> A (1.0f,0.0f);
        std::complex<float> B (1.0f,0.0f);
        std::complex<float> C (1.0f,0.0f);
        std::complex<float> D (-1.0f,0.0f);
        apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A/std::sqrt(2.0f),
                                                                  B/std::sqrt(2.0f),
                                                                  C/std::sqrt(2.0f),
                                                                  D/std::sqrt(2.0f));
        ```

!!! tig "Pauli Z gate"
    === "Description"
         The Pauli-Z gate is a single-qubit rotation through $\pi$ radians around the z-axis.
        <div align="center"> $\begin{aligned} Z & = \begin{pmatrix}1 & 0 \\0 & -1 \end{pmatrix}\end{aligned}$ </div>

        - Fill the body of the `#!cpp void z(...)` function body

        - To test your gate, uncomment `set(SOURCE_FILES src/test_z_gate.cpp src/kernels.cpp)` in the CMakeLists.txt file
        !!! info "Building and the code"  
            ```bash
            mkdir build-test-z-gate && cd build-test-z-gate
            cmake ..
            make fpga
            LD_PRELOAD=${JEMALLOC_PRELOAD} ./quantum.fpga
            ```

    === "Solution"
        - Add the following code in the `#!cpp void z(...)` function body
        ```cpp linenums="1"
        std::complex<float> A (1.0f,0.0f);
        std::complex<float> B (0.0f,0.0f);
        std::complex<float> C (0.0f,0.0f);
        std::complex<float> D (-1.0f,0.0f);
        apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A,
                                                                B,
                                                                C,
                                                                D);
        ```


## Implementing the Bernstein-Varizani algorithm


!!! tig "Let's put everything together"
    === "Question"

        - Fill the body of the `#!cpp int main(...)` function body

        - Apply the h gate to all qubits to put them all into superpositions

        - Apply the z gate to the register qubits corresponding to the classical bits matching the so called hidden number

        - Finally, sample from the probabilities using the `#!cpp void measure(...)`

        ```cpp linenums="1"
        --8<-- "./code/FQSim/src/bernstein-vazirani.cpp"
        ```
        !!! info "Building and the code"  
            ```bash
            mkdir build-fpga && cd build-fpga
            cmake ..
            make fpga
            LD_PRELOAD=${JEMALLOC_PRELOAD} ./quantum.fpga
            ```

    === "Solution"
        ```cpp 
        --8<-- "./code/FQSim/src-solution/bernstein-vazirani.cpp"
        ```










