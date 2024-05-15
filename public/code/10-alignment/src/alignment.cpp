#include <iostream>
#include <typeinfo>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>
#include <chrono>
using namespace std::chrono;

#define ALIGNMENT 64
#define IT 1024


constexpr int kVectSize = 2048;


template<typename T>
void test_structure( T* device,sycl::queue &q, int nb_iters){

      sycl::event e;
      const sycl::property_list props = {sycl::property::buffer::use_host_ptr()};

      auto start = high_resolution_clock::now();
      sycl::buffer buffer_device{device, sycl::range(kVectSize),props};
      e = q.submit([&](sycl::handler &h) {
       sycl::accessor accessor_device{buffer_device, h, sycl::read_write};
       h.single_task([=]() {
       for(int it=0;it < nb_iters ;it++){
        for (int idx = 0; idx < kVectSize; idx++) {
          accessor_device[idx].C = (int)accessor_device[idx].A + accessor_device[idx].B;
         }
        }
        });
       });

    sycl::host_accessor buffer_host(buffer_device);
    auto stop = high_resolution_clock::now();
    // convert from nanoseconds to ms
    duration<double> kernel_time = stop - start;

    std::cout  << " Time (" <<typeid(T).name()<<  ") : " << kernel_time.count() << " ms\n";
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

    //mystruct host_vec_a[kVectSize];
    //mystruct_packed host_vec_b[kVectSize];
    //mystruct_packed_aligned host_vec_c[kVectSize];

    mystruct* vec_a = new(std::align_val_t{ 64 }) mystruct[kVectSize];
    mystruct_packed* vec_b = new(std::align_val_t{ 64 }) mystruct_packed[kVectSize];
    mystruct_packed_aligned* vec_c = new(std::align_val_t{ 64 }) mystruct_packed_aligned[kVectSize];


    //mystruct * vec_a = static_cast<mystruct*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct),q));
    //mystruct_packed*vec_b = static_cast<mystruct_packed*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct_packed),q));
    //mystruct_packed_aligned*vec_c = static_cast<mystruct_packed_aligned*>(aligned_alloc_device(ALIGNMENT,kVectSize*sizeof(mystruct_packed_aligned),q));

    for (int i = 0; i < kVectSize; i++) {
        vec_a[i].A = vec_b[i].A = vec_c[i].A = char(std::rand() % 256);
        vec_a[i].B = vec_b[i].B = vec_c[i].B = std::rand();
        vec_a[i].C = vec_b[i].C = vec_c[i].C = std::rand();
    }

    std::cout << "Packed with default alignment" << kVectSize << std::endl;

    test_structure<mystruct>(vec_a,q,IT);
    test_structure<mystruct_packed>(vec_b,q,IT);
    test_structure<mystruct_packed_aligned>(vec_c,q,IT);


    delete[] vec_a;
    delete[] vec_b;
    delete[] vec_c;

    //sycl::free(vec_a,q);
    //sycl::free(vec_b,q);
    //sycl::free(vec_c,q);

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
