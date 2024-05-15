#include <iostream>
#include <algorithm>
#include <random>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>

#include <boost/align/aligned_allocator.hpp>


// Forward declare the kernel name in the global scope. This is an FPGA best
// practice that reduces name mangling in the optimization reports.
class MatMultKernel;


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


    // initialize input and output memory on the host
    constexpr size_t N = 512;
    constexpr size_t B =  16;
    std::vector<float,boost::alignment::aligned_allocator<float,64>> mat_a(N * N);
    std::vector<float,boost::alignment::aligned_allocator<float,64>> mat_b(N * N);
    std::vector<float,boost::alignment::aligned_allocator<float,64>> mat_c(N * N); 

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0, 1.0);

    // Generate random values
    std::generate(mat_a.begin(), mat_a.end(), [&dist, &mt]() {
      return dist(mt);
    });

    // Generate random values
    std::generate(mat_b.begin(), mat_b.end(), [&dist, &mt]() {
      return dist(mt);
    });

    // fill with zero
    std::fill(mat_c.begin(), mat_c.end(), 0.0); 


    std::cout << "Matrix multiplication A X B = C " <<std::endl;
    {
      // copy the input arrays to buffers to share with kernel
      // We can access the buffer using mat[i][j]
      sycl::buffer<float,2> buffer_a{mat_a.data(), sycl::range<2>(N,N)};
      sycl::buffer<float,2> buffer_b{mat_b.data(), sycl::range<2>(N,N)};
      sycl::buffer<float,2> buffer_c{mat_c.data(), sycl::range<2>(N,N)};

      
      /* DEFINE HERE the global size and local size ranges*/


      q.submit([&](sycl::handler &h) {
        // use accessors to interact with buffers from device code
        sycl::accessor accessor_a{buffer_a, h, sycl::read_only};
        sycl::accessor accessor_b{buffer_b, h, sycl::read_only};
        sycl::accessor accessor_c{buffer_c, h, sycl::read_write, sycl::no_init};

        sycl::local_accessor<float,2> tileA{{B,B}, h};
        sycl::local_accessor<float,2> tileB{{B,B}, h};

        h.parallel_for<MatMultKernel>(sycl::nd_range{global, local}, [=](sycl::nd_item<2> item)

            [[intel::max_work_group_size(1, B, B)]] 	{
            // Indices in the global index space:
            int m = item.get_global_id()[0];
            int n = item.get_global_id()[1];

            // Index in the local index space:
            // Provide local indexes i and j -- fill here

            float sum = 0;
            for (int p = 0; p < N/B; p++) {
              // Load the matrix tile from matrix A, and synchronize
              // to ensure all work-items have a consistent view
              // of the matrix tile in local memory.
              tileA[i][j] = accessor_a[m][p*B+j];
              // Do the same for tileB
              // fill here 
              item.barrier();

              // Perform computation using the local memory tile, and
              // matrix B in global memory.
              for (int kk = 0; kk < B; kk++) {
		sum += tileA[i][kk] * tileB[kk][j];
              }

              // After computation, synchronize again, to ensure all
	      // Fill here 
            }

            // Write the final result to global memory.
            accessor_c[m][n] = sum;

        });
      });
    }

  
  // result is copied back to host automatically when accessors go out of
  // scope.

    // verify that Matrix multiplication is correct
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++){
         float true_val=0.0;
         for (int k = 0 ; k < N; k++){
           true_val += mat_a[i*N +k] * mat_b[k*N+j];
         }
         if (std::abs(true_val - mat_c[i*N+j])/true_val > 1.0e-4 ) {
            std::cout << "C[" << i << ";" << j << "] = " << mat_c[i*N+j] << " expected = " << true_val << std::endl;
            passed = false;
         }
    }
    }
    
    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;

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
