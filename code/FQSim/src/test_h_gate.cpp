// Sample code Bernstein-Vazirani

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include "kernels.hpp"

using namespace sycl;


int main() {

	bool passed = true;
	try{

	   #if FPGA_SIMULATOR
	       auto selector = sycl::ext::intel::fpga_simulator_selector_v;
	   #elif FPGA_HARDWARE
	       auto selector = sycl::ext::intel::fpga_selector_v;
	   #else  // #if FPGA_EMULATOR
	       auto selector = sycl::ext::intel::fpga_emulator_selector_v;
	   #endif

       // Create a SYCL queue
       queue queue(selector);
       // make sure the device supports USM host allocations
       auto device = queue.get_device();

       std::cout << "Running on device: "
                 << device.get_info<sycl::info::device::name>().c_str()
                 << std::endl;



       constexpr size_t numQubits = 7;
       constexpr size_t numStates = 1 << numQubits; // 2^n
                                                    
               
       std::complex<float> *stateVector   = new std::complex<float>[numStates];
       std::complex<float> *stateVector_d = malloc_device<std::complex<float>>(numStates,queue);

       // Initial state |00...00>
       stateVector[0] = std::complex<float>(1.0f,0.0f);
       queue.memcpy(stateVector_d, stateVector, numStates * sizeof(std::complex<float>)).wait();


       for(int i = 0; i < numQubits; i++){
          h(queue, stateVector_d, numQubits,i);
       }


       measure(queue, stateVector_d, numQubits, 1000);

    sycl::free(stateVector_d,queue);
    }catch (exception const &e) {
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
