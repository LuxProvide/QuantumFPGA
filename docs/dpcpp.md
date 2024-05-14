# What is the Intel® oneAPI DPC++ compiler

In heterogenous computing, accelerator devices support the host processor by executing specific portion of code more efficiently. In this context, the Intel® oneAPI toolkit supports two different approaches for heterogeous computing:


!!! abstract "1. Data Parallel C++ with SYCL" 
    SYCL (Specification for Unified Cross-platform C++) provides a higher-level model for writing standard ISO C++ code that is both performance-oriented and portable across various hardware, including CPUs, GPUs and FPGAs
    It enables the use of standard C++ with extensions to leverage parallel hardware. Host and kernel code share the same source file. The DPC++ compiler is adding SYCL support on top of the LLVM C++ compiler. DPC++ is distributed with the Intel® oneAPI toolkit.



!!! abstract "2. OpenMP for C, C++, and Fortran " 
    For more than two decades, OpenMP has stood as a standard programming language, with Intel implementing its 5th version. The Intel oneAPI C++ Compiler, which includes support for OpenMP offloading, can be found in the Intel oneAPI Base Toolkit, Intel oneAPI HPC Toolkit, and Intel oneAPI IoT Toolkit. Both the Intel® Fortran Compiler Classic and the Intel® Fortran Compiler equipped with OpenMP offload support are accessible through the Intel oneAPI HPC Toolkit. 

    **Note**: <u>OpenMP is not supported for FPGA devices</u>.

## DPC++ is one of the existing SYCL implementations

![](https://www.khronos.org/assets/uploads/blogs/2020-05-sycl-landing-page-02.jpg)

!!! warning "ComputeCpp (codeplay)"
    support will no longer be provided from September 1st 2023 (see [announce](https://codeplay.com/portal/news/2023/07/07/the-future-of-computecpp))

## Key Features and Components

* **Heterogeneous Support**: Enables coding for various types of processors within the same program.
* **Performance Optimization**: It offers various optimization techniques to ensure code runs as efficiently as possible.
* **Standard Compliance**: Aligns with the latest C++ standards, along with the SYCL standard.
* **Debugging and Analysis Tools**: Integrates with tools that assist in debugging and analyzing code.
* **Integration with IDEs**: Compatible with popular Integrated Development Environments to facilitate a seamless coding experience.
* **Open Source and Community Driven**: This promotes collaboration and ensures that the technology stays up to date with industry needs.

## SYCL and FPGA

SYCL offers APIs and abstractions, but FPGA cards are unique to each vendor, and even within the same vendor, FPGA cards may have diverse capabilities. DPC++ targets Intel® FPGA cards specifically and extends SYCL's functions. This allows it to leverage the strength of FPGA, all the while maintaining as much generalizability and portability as possible.

## References

* [Data Parallel C++: Mastering DPC++ for Programming of Heterogeneous Systems using C++ and SYCL](https://link.springer.com/book/10.1007/978-1-4842-5574-2)
* [Intel® oneAPI DPC++/C++ Compiler](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html#gs.3luom6) 
* [SYCL official documentation](https://www.khronos.org/sycl/)

