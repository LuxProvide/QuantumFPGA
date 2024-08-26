# Introduction to FPGA programming with Intel® oneAPI

![](./images/Intel-oneAPI-logo-686x600.jpg){ align=right width=200 }

[Intel® oneAPI](https://www.intel.com/content/www/us/en/developer/tools/oneapi/toolkits.html#gs.3c0top) is a software development toolkit from Intel designed to simplify the process of developing high-performance applications for various types of computing architecture. It aims to provide a unified and simplified programming model for CPUs, GPUs, FPGAs, and other types of hardware, such as AI accelerators, allowing developers to use a single codebase for multiple platforms.

One of the main components of oneAPI is the [Data Parallel C++ (DPC++)](https://www.intel.com/content/www/us/en/developer/videos/dpc-part-1-introduction-to-new-programming-model.html#gs.3c0wb4), an open, standards-based language built upon the ISO C++ and [SYCL standards](https://www.khronos.org/sycl/). DPC++ extends C++ with features like parallel programming constructs and heterogeneous computing support, providing developers with the flexibility to write code for different types of hardware with relative ease.

In addition to DPC++, oneAPI includes a range of libraries designed to optimize specific types of tasks, such as machine learning, linear algebra, and deep learning. These include oneDNN for deep neural networks, oneMKL for math kernel library, and oneDAL for data analytics, among others.

It's important to note that Intel oneAPI is part of Intel's broader strategy towards open, standards-based, cross-architecture programming, which is intended to reduce the complexity of application development and help developers leverage the capabilities of different types of hardware more efficiently and effectively.

In this documentation, you will explore how to:

* Use the DPC++ compiler to create executable for Intel FPGA hardware
* Discover the SYCL C++ abstraction layer
* How to move data from and to FPGA hardware
* Optimize FPGA workflows

In order to get an overview of FPGA computing for the HPC ecosystem, please refer to the following [slides](./pdfs/01-Introduction to FPGA computing for the HPC ecosystem.pdf).


## What is the Intel® oneAPI DPC++ compiler

In heterogenous computing, accelerator devices support the host processor by executing specific portion of code more efficiently. In this context, the Intel® oneAPI toolkit supports two different approaches for heterogeous computing:


!!! abstract "1. Data Parallel C++ with SYCL" 
    SYCL (Specification for Unified Cross-platform C++) provides a higher-level model for writing standard ISO C++ code that is both performance-oriented and portable across various hardware, including CPUs, GPUs and FPGAs
    It enables the use of standard C++ with extensions to leverage parallel hardware. Host and kernel code share the same source file. The DPC++ compiler is adding SYCL support on top of the LLVM C++ compiler. DPC++ is distributed with the Intel® oneAPI toolkit.



!!! abstract "2. OpenMP for C, C++, and Fortran " 
    For more than two decades, OpenMP has stood as a standard programming language, with Intel implementing its 5th version. The Intel oneAPI C++ Compiler, which includes support for OpenMP offloading, can be found in the Intel oneAPI Base Toolkit, Intel oneAPI HPC Toolkit, and Intel oneAPI IoT Toolkit. Both the Intel® Fortran Compiler Classic and the Intel® Fortran Compiler equipped with OpenMP offload support are accessible through the Intel oneAPI HPC Toolkit. 

    **Note**: <u>OpenMP is not supported for FPGA devices</u>.

## DPC++ is one of the existing SYCL implementations

* Data Parallel C++ is the oneAPI's Implementation of SYCL.

![](https://www.khronos.org/assets/uploads/blogs/2020-05-sycl-landing-page-02.jpg)

!!! warning "ComputeCpp (codeplay)"
    Support for ComputeCpp will no longer be provided from September 1st 2023 (see [announce](https://codeplay.com/portal/news/2023/07/07/the-future-of-computecpp))

## Key Features and Components

* **Heterogeneous Support**: Enables coding for various types of processors within the same program.
* **Performance Optimization**: It offers various optimization techniques to ensure code runs as efficiently as possible.
* **Standard Compliance**: Aligns with the latest C++ standards, along with the SYCL standard.
* **Debugging and Analysis Tools**: Integrates with tools that assist in debugging and analyzing code.
* **Integration with IDEs**: Compatible with popular Integrated Development Environments to facilitate a seamless coding experience.
* **Open Source and Community Driven**: This promotes collaboration and ensures that the technology stays up to date with industry needs.

## SYCL 


<figure markdown>
[![](./images/sycl_prog.png)](https://www.khronos.org/assets/uploads/developers/presentations/SYCL-2020-Launch-Feb21.pdf)
  <figcaption><small>(source: https://www.khronos.org)</small></figcaption>
</figure>

* SYCL  abstractions to enable heterogeneous device programming

* SYCL's stratregy  is to allow different heterogenous devices, e.g., CPUs, GPUs, FPGAs, to be used simultaneously

* Generic programming with templates and lambda functions


## Intel® oneAPI FPGA

SYCL offers APIs and abstractions, but FPGA cards are unique to each vendor, and even within the same vendor, FPGA cards may have diverse capabilities. DPC++ targets Intel® FPGA cards specifically and extends SYCL's functions. This allows it to leverage the strength of FPGA, all the while maintaining as much generalizability and portability as possible.


## References

* [Data Parallel C++: Mastering DPC++ for Programming of Heterogeneous Systems using C++ and SYCL](https://link.springer.com/book/10.1007/978-1-4842-5574-2)
* [Intel® oneAPI DPC++/C++ Compiler](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html#gs.3luom6) 
* [SYCL official documentation](https://www.khronos.org/sycl/)

