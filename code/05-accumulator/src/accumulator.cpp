#include <iostream>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>

// Forward declare the kernel name in the global scope. This is an FPGA best
// practice that reduces name mangling in the optimization reports.
class Accumulator;

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
    double * vec = new(std::align_val_t{ 64 }) double[kVectSize];
    double res = 0;
    for (int i = 0; i < kVectSize; i++) {
      vec[i] = 1.0;
    }

    std::cout << "Accumulate values " << kVectSize << std::endl;
    {
      // copy the input arrays to buffers to share with kernel
      sycl::buffer buffer_in{vec, sycl::range(kVectSize)};
      sycl::buffer buffer_out{&res, sycl::range(1)};

      q.submit([&](sycl::handler &h) {
        // use accessors to interact with buffers from device code
        sycl::accessor arr{buffer_in, h, sycl::read_only};
        sycl::accessor result{buffer_out, h, sycl::write_only,sycl::no_init};

        h.single_task<Accumulator>([=]() {
	  double temp_sum = 0;
          for (int i = 0; i < kVectSize; ++i)
            temp_sum += arr[i];
          result[0] = temp_sum;
        });
      });
    }
    // result is copied back to host automatically when accessors go out of
    // scope.

    // verify that Accumulation is correct
    double expected = 0.0; 
    for (int i = 0; i < kVectSize; i++) 
      expected += vec[i];

    if (res != expected) {
        std::cout << "res = " << res <<  ", expected = "
                  << expected << std::endl;
        passed = false;
      }
    

    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;

    delete[] vec;
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
