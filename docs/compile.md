# Compiling SYCL programs for Intel® FPGA cards

## Setups

Please clone first the [oneAPI-sample](https://github.com/oneapi-src/oneAPI-samples.git) repository with the `git clone https://github.com/oneapi-src/oneAPI-samples.git` in your home folder.

Once the repository cloned, you should see the following hierarchy:

```bash
tree -d -L 2 oneAPI-samples
oneAPI-samples
├── AI-and-Analytics
│   ├── End-to-end-Workloads
│   ├── Features-and-Functionality
│   ├── Getting-Started-Samples
│   ├── images
│   └── Jupyter
├── common
│   └── stb
├── DirectProgramming
│   ├── C++
│   ├── C++SYCL
│   ├── C++SYCL_FPGA
│   └── Fortran
├── Libraries
│   ├── oneCCL
│   ├── oneDAL
│   ├── oneDNN
│   ├── oneDPL
│   ├── oneMKL
│   └── oneTBB
├── Publications
│   ├── DPC++
│   └── GPU-Opt-Guide
├── RenderingToolkit
│   ├── GettingStarted
│   └── Tutorial
├── Templates
│   └── cmake
└── Tools
    ├── Advisor
    ├── ApplicationDebugger
    ├── Benchmarks
    ├── GPU-Occupancy-Calculator
    ├── Migration
    └── VTuneProfiler
```

* As you can see Intel provides numerous code samples and examples to help your grasping the power of the oneAPI toolkit.
* We are going to focus on `DirectProgramming/C++SYCL_FPGA`.
* Create a symbolic at the root of your home directory pointing to this folder:
```bash
cd
ln -s oneAPI-samples/DirectProgramming/C++SYCL_FPGA/Tutorials/GettingStarted
tree -d -L 2 GettingStarted
GettingStarted
├── fast_recompile
│   ├── assets
│   └── src
├── fpga_compile
│   ├── part1_cpp
│   ├── part2_dpcpp_functor_usm
│   ├── part3_dpcpp_lambda_usm
│   └── part4_dpcpp_lambda_buffers
└── fpga_template
    └── src
```

* The **fpga_compile** folder provides basic examples to start compiling SYCL C++ code with the DPC++ compiler

* The **fpga_recompile** folder show you how to recompile quickly your code without having to rebuild the FPGA image

* The **fpga_template** is a starting template project that you can use to bootstrap a project


## Discovering devices

Before targeting a specific hardware accelerator, you need to ensure that the sycl runtime is able to detect it.
!!! example "Commands"
    ```bash linenums="1"
    # Create permanent tmux session
    tmux new -s fpga_session
    # We need a job allocation on a FPGA node
    salloc -A <account> -t 48:00:00 -q default -p fpga -N 1
    # Load the staging environment
    module load env/staging/2023.1
    module load intel-fpga
    module load 520nmx/20.4
    # Check the available devices
    sycl-ls
    ```

!!! success "Output"
    ```bash
    [opencl:cpu:0] Intel(R) OpenCL, AMD EPYC 7452 32-Core Processor                 3.0 [2022.13.3.0.16_160000]
    [opencl:acc:1] Intel(R) FPGA Emulation Platform for OpenCL(TM), Intel(R) FPGA Emulation Device 1.2 [2022.13.3.0.16_160000]
    [opencl:acc:2] Intel(R) FPGA SDK for OpenCL(TM), p520_hpc_m210h_g3x16 : BittWare Stratix 10 MX OpenCL platform (aclbitt_s10mx_pcie0) 1.0 [2022.1]
    [opencl:acc:3] Intel(R) FPGA SDK for OpenCL(TM), p520_hpc_m210h_g3x16 : BittWare Stratix 10 MX OpenCL platform (aclbitt_s10mx_pcie1) 1.0 [2022.1]
    ```

## First code

!!! example "GettingStarted/fpga_compile/part4_dpcpp_lambda_buffers/src/vector_add.cpp"
    ```cpp linenums="1"
    #include <iostream>

    // oneAPI headers
    #include <sycl/ext/intel/fpga_extensions.hpp>
    #include <sycl/sycl.hpp>

    // Forward declare the kernel name in the global scope. This is an FPGA best
    // practice that reduces name mangling in the optimization reports.
    class VectorAddID;

    void VectorAdd(const int *vec_a_in, const int *vec_b_in, int *vec_c_out,
                   int len) {
      for (int idx = 0; idx < len; idx++) {
        int a_val = vec_a_in[idx];
        int b_val = vec_b_in[idx];
        int sum = a_val + b_val;
        vec_c_out[idx] = sum;
      }
    }

    constexpr int kVectSize = 256;

    int main() {
      bool passed = true;
      try {
        // Use compile-time macros to select either:
        //  - the FPGA emulator device (CPU emulation of the FPGA)
        //  - the FPGA device (a real FPGA)
        //  - the simulator device
    #if FPGA_SIMULATOR
        auto selector = sycl::ext::intel::fpga_simulator_selector_v;
    #elif FPGA_HARDWARE
        auto selector = sycl::ext::intel::fpga_selector_v;
    #else  // #if FPGA_EMULATOR
        auto selector = sycl::ext::intel::fpga_emulator_selector_v;
    #endif

        // create the device queue
        sycl::queue q(selector);

        // make sure the device supports USM host allocations
        auto device = q.get_device();

        std::cout << "Running on device: "
                  << device.get_info<sycl::info::device::name>().c_str()
                  << std::endl;

        // declare arrays and fill them
        int * vec_a = new int[kVectSize];
        int * vec_b = new int[kVectSize];
        int * vec_c = new int[kVectSize];
        for (int i = 0; i < kVectSize; i++) {
          vec_a[i] = i;
          vec_b[i] = (kVectSize - i);
        }

        std::cout << "add two vectors of size " << kVectSize << std::endl;
        {
          // copy the input arrays to buffers to share with kernel
          sycl::buffer buffer_a{vec_a, sycl::range(kVectSize)};
          sycl::buffer buffer_b{vec_b, sycl::range(kVectSize)};
          sycl::buffer buffer_c{vec_c, sycl::range(kVectSize)};

          q.submit([&](sycl::handler &h) {
            // use accessors to interact with buffers from device code
            sycl::accessor accessor_a{buffer_a, h, sycl::read_only};
            sycl::accessor accessor_b{buffer_b, h, sycl::read_only};
            sycl::accessor accessor_c{buffer_c, h, sycl::read_write, sycl::no_init};

            h.single_task<VectorAddID>([=]() {
              VectorAdd(&accessor_a[0], &accessor_b[0], &accessor_c[0], kVectSize);
            });
          });
        }
        // result is copied back to host automatically when accessors go out of
        // scope.

        // verify that VC is correct
        for (int i = 0; i < kVectSize; i++) {
          int expected = vec_a[i] + vec_b[i];
          if (vec_c[i] != expected) {
            std::cout << "idx=" << i << ": result " << vec_c[i] << ", expected ("
                      << expected << ") A=" << vec_a[i] << " + B=" << vec_b[i]
                      << std::endl;
            passed = false;
          }
        }

        std::cout << (passed ? "PASSED" : "FAILED") << std::endl;

        delete[] vec_a;
        delete[] vec_b;
        delete[] vec_c;
      } catch (sycl::exception const &e) {
        // Catches exceptions in the host code.
        std::cerr << "Caught a SYCL host exception:\n" << e.what() << "\n";

        // Most likely the runtime couldn't find FPGA hardware!
        if (e.code().value() == CL_DEVICE_NOT_FOUND) {
          std::cerr << "If you are targeting an FPGA, please ensure that your "
                       "system has a correctly configured FPGA board.\n";
          std::cerr << "Run sys_check in the oneAPI root directory to verify.\n";
          std::cerr << "If you are targeting the FPGA emulator, compile with "
                       "-DFPGA_EMULATOR.\n";
        }
        std::terminate();
      }
      return passed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    ```
* The `vector_add.cpp` source file contains all the necessary to understand how to create a SYCL program

* **lines 4 and 5** are the minimal headers to include in your SYCL program

* **line 9** is a forward declaration of the kernel name

* **lines 11-19** is a function representing our kernel. Note the absence of `__kernel`, `__global` as it exists in OpenCL

* **lines 30-36** are pragmas defining whether you want a full compilation, a CPU emulation or the simulator

* **line 39** is the queue creation. The queue is bounded to a device. We will discuss it later in details.

* **lines 41-46** provides debugging information at runtime.

* **lines 48-54** instantiates 3 vectors. `vec_a` and `vec_b` are input C++ arrays and are initialized inside the next loop. `vec_c` is an output C++ array collecting computation results between `vec_a` and `vec_b`.

* **lines 60-62** create buffers for each vector and specify their size. The runtime copies the data to the FPGA global memory when the kernel starts

* **line 64** submits a command group to the device queue

* **lines 66-68** relies on accessor to infer data dependencies. "read_only" accessor have to wait for data to be fetched. "no_init" option indicates ito the runtime know that the previous contents of the buffer can be discarded

* **lines 70-73** starts a single tasks (single work-item) and call the kernel function

* **lines 99-105** catch SYCL exceptions and terminate the execution

### Emulation

* FPGA emulation refers to the process of using a software or hardware system to mimic the behavior of an FPGA device. This is usually done to test, validate, and debug FPGA designs before deploying them on actual hardware. The Intel® FPGA emulator runs the code on the host cpu.

* Emulation is crucial to validate the functionality of your kernel design. 

* During emulation, your are not seeking for performance.

!!! example "Compile for emulation (in one step)"
    ```bash
    icpx -fsycl -fintelfpga -qactypes vector_add.cpp -o vector_add.fpga_emu
    ```

Intel uses the SYCL Ahead-of-time (AoT) compilation which as two steps:

1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).

2. The "link" stage invokes the compiler's FPGA backend before linking.

!!! example "Two-steps compilation"
    ```bash
    # Compile 
    icpx -fsycl -fintelfpga -qactypes -o vector_add.cpp.o -c vector_add.cpp
    # Link
    icpx -fsycl -fintelfpga -qactypes vector_add.cpp.o -o vector_add.fpga_emu
    ```

* The compiler option `-qactypes` informs the compiler to sreahc and include the Algorithmic C (AC) data type folder for header and libs to the AC data types libraries for Field Programmable Gate Array (FPGA) and CPU compilations.
* The [Algorithmic C (AC) datatypes](https://hlslibs.org/) libraries include a numerical set of datatypes and an interface datatype for modeling channels in communicating processes in C++.

### Static reports

* During the process of compiling an FPGA hardware image with the Intel® oneAPI DPC++/C++ Compiler, various checkpoints are provided at different compilation steps. These steps include object files generation, an FPGA early image object generation, an FPGA image object generation, and finally executables generation. These checkpoints offer the ability to review errors and make modifications to the source code without needing to do a full compilation every time. 

* When you reach the FPGA early image object checkpoint, you can examine the optimization report generated by the compiler. 

* Upon arriving at the FPGA image object checkpoint, the compiler produces a finished FPGA image.

In order to generate the FPGA early image, you will need to add the following option:

* `-Xshardware`

* `-Xstarget=<target>` or `-Xsboard=<board>`

* `-fsycl-link=early`


!!! example "Compile for FPGA early image"
    ```bash
    icpx -fsycl -fintelfpga -qactypes -Xshardware -fsycl-link=early -Xsboard=p520_hpc_m210h_g3x16 vector_add.cpp -o vector_add_report.a
    ```

* The `vector_add_report.a` is not what we target in priority. We target the reports directory `vector_add_report.prj` which has been created.

    
* You can evaluate whether the estimated kernel performance data is satisfactory by going to the <project_dir>/reports/ directory and examining one of the following files related to your application:

1. report.html: This file can be viewed using Internet browsers of your choice
2. <design_name>.zip: Utilize the Intel® oneAPI FPGA Reports tool,i.e., `fpga_report`

### Full compilation

This phase produces the actual FPGA bitstream, i.e., a file containing the programming data associated with your FPGA chip. This file requires the target FPGA platform to be generated and executed. For FPGA programming, the Intel® oneAPI toolkit requires the [Intel® Quartus® Prime](https://www.intel.com/content/www/us/en/products/details/fpga/development-tools/quartus-prime.html) software to generate this bitstream.

!!! example "Full hardware compilation"
    ```bash
    icpx -fsycl -fintelfpga -qactypes -Xshardware -Xsboard=p520_hpc_m210h_g3x16 -DFPGA_HARDWARE vector_add.cpp -o vector_add_report.fpga
    ```

* The compilation will take several hours. Therefore, we strongly advise you to verify your code through emulation first.

* You can also use the `-Xsfast-compile` option which offers a faster compile time but reduce the performance of the final FPGA image.

### Fast recompilation

* At first glance having a single source file is not necessarily a good idea when host and device compilation differs so much

* However, there is two different strategies to deal with it:

  1. Use a single source file and add the `-reuse-exe`

  2. Separate host and device code compilation in your FPGA project


* This is up to you to choose the method that suits you the most

!!! example "Using the `-reuse-exe` option"
    ```bash
    icpx -fsycl -fintelfpga -qactypes -Xshardware -Xsboard=p520_hpc_m210h_g3x16 -DFPGA_HARDWARE -reuse-exe=vector_add.fpga vector_add.cpp -o vector_add.fpga
    ```
    If only the host code changed since the previous compilation, providing the `-reuse-exe=image` flag to `icpx` instructs the compiler to extract the compiled FPGA binary from the existing executable and package it into the new executable, saving the device compilation time.
  
!!! tig "Question"
    * What happens if the vector_add.fpga is missing ?

!!! example "Separating host and device code"
    Go to the `GettingStarted/fpga_recompile` folder. It provides an example of separate host and device code
    The process is similar as the compilation process for OpenCL except that a single tool is used, i.e., `icpx`

    1. Compile the host code:
    ```bash 
    icpx -fsycl -fintelfpga -DFPGA_HARDWARE host.cpp -c -o host.o
    ```
    2. Compile the FPGA image:
    ```bash
    icpx -fsycl -fintelfpga -Xshardware -Xsboard=p520_hpc_m210h_g3x16 -fsycl-link=image kernel.cpp -o dev_image.a
    ```
    3. Link both:
    ```bash
    icpx -fsycl -fintelfpga host.o dev_image.a -o fast_recompile.fpga
    ```






