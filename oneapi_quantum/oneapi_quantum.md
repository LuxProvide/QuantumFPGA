- We will develop the quantum circuit we have seen previously, i.e.,  Bernstein-Varizani to search an hidden element
- We will need to code the Pauli H and Z gates 
- We will also need a function to measure (sample) from the distribution obtained using the state vector amplitudes

## Setup

- We need first to obtain an interactive job on the fpga partition and load some modules

!!! example "Commands"
    ```bash
    # Please log out and reconnect to Meluxina using X11 forwarding
    # ssh -X ... (-X option to enables X11 forwarding)
    srun -A <ACCOUNT> --reservation=lxp-quantum-training-fpga -t 02:00:00 -q default -p fpga -N1  --forward-x  --pty bash -i
    module load env/staging/2023.1
    module load git-lfs
    module load CMake jemalloc freeglut intel-fpga 520nmx
    ```
!!! warning "FPGA emulation"
    As the number of FPGA nodes is limited to 20, your interactive job may not start if the number of participants is larger than this number. If so, please use a different partition (e.g., cpu, largemem) to use FPGA emulation.
    ```bash
    srun -A <ACCOUNT> --reservation=lxp-quantum-training-fpga -t 02:00:00 -q default -p largemem -N1  --forward-x  --pty bash -i
    module load env/staging/2023.1
    module load git-lfs
    module load CMake jemalloc freeglut intel-fpga 520nmx
    ```

- Clone the repository if not already done: `git lfs clone https://github.com/LuxProvide/QuantumFPGA`

- Create a symbolic link to the project `ln -s QuantumFPGA/code/FQSim` and `cd FQSim`

- The project contains the following files:

```bash
$>tree -L 2
.
├── CMakeLists.txt
├── fpga_image
│   └── quantum.fpga
├── src
│   ├── bernstein-vazirani.cpp
│   ├── blochSphere.cpp
│   ├── blochSphere.hpp
│   ├── kernels.cpp
│   ├── kernels.hpp
│   ├── test_h_gate.cpp
│   ├── test_rxryrz.cpp
│   └── test_z_gate.cpp
└── src-solution
    ├── bernstein-vazirani.cpp
    ├── blochSphere.cpp
    ├── blochSphere.hpp
    ├── kernels.cpp
    ├── kernels.hpp
    ├── test_h_gate.cpp
    ├── test_rxryrz.cpp
    └── test_z_gate.cpp
```

- **fpga_image** : contains the fpga image build prior to the workshop training to avoid waiting hardware synthesis. Indeed, the offline compiler will extract the bitstream file `aocx` and reuse it if and only if the device code did not change
- **src** : All files contain blank code that we are going to fill step by step
    * **bernstein-vazirani.cpp**: the source file with the Bernstein-Vazirani circuit.
    * **blochSphere.cpp**: source file containing all code to draw an OpenGL BlockSphere.
    * **blochSphere.hpp**: header file containing the signature of function to draw an OpenGL BlockSphere.
    * **kernel.cpp**: source file containing all code for the gates.
    * **kernel.hpp**: header file containing the signature of function.
    * **test_h_gate.cpp**: source file for the example testing the h gate.
    * **test_rxryrz.cpp**: test example for the 3 rotation gates rx, ry and rz.
    * **test_z_gate.cpp**: source file for the example testing the z gate.
- **src-solution**:  the solution to fill all blank code. Replace `set(SOURCE_FILES src/bernstein-vazirani.cpp src/kernels.cpp)` by `set(SOURCE_FILES src-solution/bernstein-vazirani.cpp src-solution/kernels.cpp)` in the CMakeLists.txt file
         test_z_gate.cpp

## Building code

- We strongly recommend to compile and execute your code using the `Intel(R) FPGA Emulation Platform` which does not require any FPGA board on the system.

!!! example "Commands"
    ```bash
    mkdir build-emu && cd build-emu
    cmake ..
    make fpga_emu
    ```

- Once your code runs on the `Intel(R) FPGA Emulation Platform` without errors:

!!! example "Commands"
    ```bash
    mkdir build-fpga && cd build-fpga
    cmake ..
    make fpga
    ```

!!! warning "Using Direct Memory Access (DMA)"
    * DMA is enabled between host and the device if buffer data have a 64-byte alignment.
    * We strongly recommend you to load our `jemalloc` module which provides such default alignment:
    ```bash
    module load jemalloc
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

- Let's consider a multiqubit register $ |\psi \rangle = \sum\limits_{k=0}^{2^N-1} \alpha_k |k \rangle $ which is a mixture of pure state

- $ |k \rangle $ is the decimal representation of the pure state k 

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
\end{aligned}
$$

$$
\begin{aligned}
                              \hspace*{1cm} & = \begin{matrix} 
                                              ( \alpha_1 { \color{red}{       u_1 \alpha_2 + u_2 \beta_2 }}  \alpha_3)  |000 \rangle + \\
                                              ( \alpha_1 { \color{blue}{     u_1 \alpha_2 + u_2 \beta_2  }}  \beta_3  )  |001 \rangle + \\ 
                                              ( \alpha_1 { \color{red}{       u_3 \alpha_2  + u_4 \beta_2}}  \alpha_3 )  |010 \rangle + \\
                                              ( \alpha_1 { \color{blue}{     u_3 \alpha_2  + u_4 \beta_2 }}  \beta_3  )  |011 \rangle + \\
                                              ( \beta_1  { \color{green}{   u_1 \alpha_2 + u_2 \beta_2   }}  \alpha_3 )  |100 \rangle + \\
                                              ( \beta_1 { \color{purple}{  u_1 \alpha_2 + u_2 \beta_2   }}  \beta_3   )  |101 \rangle + \\
                                              ( \beta_1 {  \color{green}{  u_3 \alpha_2  + u_4 \beta_2  }}  \alpha_3  )  |110 \rangle + \\
                                              ( \beta_1{  \color{purple}{ u_3 \alpha_2  + u_4 \beta_2  }}  \beta_3    )  |111 \rangle \phantom{0} 
                                \end{matrix}
\end{aligned}
$$


- Nonetheless, we are not applying the tensor product $\otimes$ every time which would be inefficient 

- Starting from specific state vector, we will apply for example a gate to the 2nd qubit like following:

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


- As you can see in the previous example to apply a gate U with its 4 complex coefficients, we apply $(u_1 u_2)$ to the coefficients corresponding to basis vector with a 0 at position 2 and $(u_3 u_4)$ to the coefficients corresponding to basis vector with a 1 at position 2

- Knowing this fact, we can divide by two the search and apply the gate coefficient by only searching the 1st, 2nd, kth number where the basis vector has a 0 at the chosen position

- To convince you, let's continue with our previous example:

$$
\begin{aligned}
\begin{matrix} 
|000 \rangle & \rightarrow \text{1st (index 0) position with 0 at position 2}  \\
|001 \rangle & \rightarrow \text{2nd (index 1) position with 0 at position 2}  \\ 
|010 \rangle & \\
|011 \rangle & \\
|100 \rangle & \rightarrow \text{3rd (index 2) position with 0 at position 2} \\
|101 \rangle & \rightarrow \text{4th (index 3) position with 0 at position 2}\\
|110 \rangle & \\
|111 \rangle & \phantom{0} 
 \end{matrix}
\end{aligned}
$$

- Starting from the indexes, we can find where we should apply $(u_1 u_2)$ coefficient

$$
\begin{aligned}
\begin{matrix} 
00  & \rightarrow \text{index 0. Adding 0 to position 2} & \rightarrow   |000 \rangle   \\
01  & \rightarrow \text{index 1. Adding 0 to position 2} & \rightarrow   |001 \rangle  \\ 
10  & \rightarrow \text{index 2.  Adding 0 to position 2}& \rightarrow   |100 \rangle    \\
11  & \rightarrow \text{index 3. Adding 0 to position 2} & \rightarrow   |101 \rangle    \\
\end{matrix}
\end{aligned}
$$

- To find coeffcients where $(u_3 u_4)$ should be applied, we only need to add 1 instead of 0 to get the corresponding basis vector

- Finally, we will have two functions to apply any kind of one qubit gate (except the controlled gates):

    - **nth_cleared**: finds the Nth number where a given binary digit is set to 0. 
    - **apply_gate**:  apply a general one qubit gate by finding in parallel all pure vector with digit 2 set to 0. For each of these vector, we can easily find the one with digit set to 1 and replace the amplitudes according to what we have above 

### Computing probabilities from state vector amplitudes

- The following kernel is only used to compute the probability for each pure state vector

- The probability to measure $∣k\rangle$ is $|\alpha_k|^2$ with  $\sum\limits_{k=0}^{2^N-1} |\alpha_k|^2 = 1 $

```cpp title="kernel code" linenums="1"
queue.parallel_for<class Proba>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
	  int global_id = item.get_id(0);
            std::complex<float> amp = stateVector_d[global_id];
            // Equal but numerically more stable than abs(amp)*abs(amp)
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

        - To test your gate, set the variable `SOURCE_FILES` as follows `set(SOURCE_FILES src/test_h_gate.cpp src/kernels.cpp)` in the CMakeLists.txt file
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

        - To test your gate, set the variable `SOURCE_FILES` as follows `set(SOURCE_FILES src/test_z_gate.cpp src/kernels.cpp)` in the CMakeLists.txt file
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

        !!! info "Building and the code"  
            ```bash
            mkdir build-BV && cd build-BV
            cmake ..
            make fpga
            LD_PRELOAD=${JEMALLOC_PRELOAD} ./quantum.fpga
            ```

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










