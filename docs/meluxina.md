# Using the Luxembourgish National Supercomputer Meluxina

If you follow this course through the workshop organized by [Supercomputing Luxembourg](https://supercomputing.lu/), an allocation on [Meluxina](https://docs.lxp.lu/system/overview/) has been provided to you by [LuxProvide](https://www.luxprovide.lu/).

During this workshop, we will stringly rely on the National Supercomputer. Meluxina is a supercomputer located in Luxembourg, which began operation in 2021. It is part of the European High-Performance Computing (EuroHPC) Joint Undertaking, an initiative by the European Union to develop a world-class supercomputing ecosystem in Europe.

The Meluxina supercomputer is hosted by LuxProvide, the national HPC organization in Luxembourg. Its computing power is intended to be used for a wide range of tasks, such as data visualization, artificial intelligence, and simulating complex systems, serving both academic researchers and industry. The machine's architecture includes CPUs, GPUs and FPGAs. It is designed to be energy-efficient, utilizing technologies to reduce power consumption and lower its environmental footprint.

Meluxina's FPGA nodes have two 2 Intel [Stratix 10MX](https://www.intel.com/content/www/us/en/products/details/fpga/stratix/10/mx.html) 16 GB FPGA cards. The FPGA partition contains 20 nodes.

[![](https://www.bittware.com/files/520N-MX-800px.svg)](https://www.bittware.com/products/520n-mx/)

## Connecting to Meluxina

You should have received your credentials some days before the workshop. If not, please contact the organizer.
Please follow the LuxProvide [documentation](https://docs.lxp.lu/first-steps/quick_start/) to setup your access.

## Accessing the fpga partition

```bash
$ tmux new -s fpga_session
[fpga_session]$ salloc -A p200117 -t 01:00:00 -q default -p fpga -N 1
```
