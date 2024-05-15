#include <iostream>
#include <typeinfo>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>

#define ALIGNMENT 64
#define IT 1024


constexpr int kVectSize = 2048;


template<typename T>
void test_structure(T* host, T* device, sycl::queue &q, int nb_iters){

	  sycl::event e1 = q.submit([&](sycl::handler &h) {
	  // copy host to device
      h.memcpy(device, host, kVectSize * sizeof(T));
      });
	  e1.wait();


      q.submit([&](sycl::handler &h) {
       h.single_task([=]() {
       for(int it=0;it < nb_iters ;it++){
        for (int idx = 0; idx < kVectSize; idx++) {
          device[idx].C = (int)device[idx].A + device[idx].B;
         }
        }
        });
       }).wait();


	  sycl::event e2 = q.submit([&](sycl::handler &h) {
	  // copy device to host
      h.memcpy(host,device, kVectSize * sizeof(T));
      });
	  e2.wait();



    double start = e1.get_profiling_info<sycl::info::event_profiling::command_start>();
    double end = e2.get_profiling_info<sycl::info::event_profiling::command_end>();
    // convert from nanoseconds to ms
    double kernel_time = (double)(end - start) * 1e-6;

    std::cout  << " Time (" <<typeid(T).name()<<  ") : " << kernel_time << " ms\n";
}

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
    sycl::queue q(selector,sycl::property::queue::enable_profiling{});

    // make sure the device supports USM host allocations
    auto device = q.get_device();

    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;

    // declare arrays and fill them
    typedef struct {
        char A;
        int  B;
        int  C;
    } mystruct;

    typedef struct __attribute__ ((packed)) {
        char A;
        int  B;
        int  C;
    } mystruct_packed;


    typedef struct __attribute__ ((packed)) __attribute__ ((aligned(16))) {
        char A;
        int  B;
        int  C;
    } mystruct_packed_aligned;


    mystruct* host_vec_a = new(std::align_val_t{ 64 }) mystruct[kVectSize];
    mystruct_packed* host_vec_b = new(std::align_val_t{ 64 }) mystruct_packed[kVectSize];
    mystruct_packed_aligned* host_vec_c = new(std::align_val_t{ 64 }) mystruct_packed_aligned[kVectSize];


    mystruct* vec_a = static_cast<mystruct*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct),q));
    mystruct_packed* vec_b = static_cast<mystruct_packed*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct_packed),q));
    mystruct_packed_aligned* vec_c = static_cast<mystruct_packed_aligned*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct_packed_aligned),q));

    for (int i = 0; i < kVectSize; i++) {
        host_vec_a[i].A = host_vec_b[i].A = host_vec_c[i].A = char(std::rand() % 256);
        host_vec_a[i].B = host_vec_b[i].B = host_vec_c[i].B = std::rand();
        host_vec_a[i].C = host_vec_b[i].C = host_vec_c[i].C = std::rand();
    }

    std::cout << "Packed with default alignment" << kVectSize << std::endl;

    test_structure<mystruct>(host_vec_a,vec_a,q,IT);
    test_structure<mystruct_packed>(host_vec_b,vec_b,q,IT);
    test_structure<mystruct_packed_aligned>(host_vec_c,vec_c,q,IT);


    delete[] host_vec_a;
    delete[] host_vec_b;
    delete[] host_vec_c;

    sycl::free(vec_a,q);
    sycl::free(vec_b,q);
    sycl::free(vec_c,q);

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
