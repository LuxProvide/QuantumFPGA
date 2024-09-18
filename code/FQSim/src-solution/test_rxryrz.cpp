// Sample code Bernstein-Vazirani

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include "kernels.hpp"
#include "blochSphere.hpp"

using namespace sycl;

// Variables to store rotation angles
float angleX = -45.0f;  // Start viewpoint in front of North and South poles
float angleY = -20.0f;
float angleZ = 0.0f;
int lastMouseX, lastMouseY;
bool isDragging = false;

// Default: Draw a bold black arrow (example theta=45 degrees, phi=60 degrees)
float theta = M_PI / 4;  // 45 degrees in radians
float phi = M_PI / 3;    // 60 degrees in radians


void state2angle(std::complex<float> alpha, std::complex<float> beta, float & theta_out, float & phi_out){

    float magnitude_alpha = std::abs(alpha);
    float magnitude_beta = std::abs(beta);


   // Ensure the state is normalized (alpha^2 + beta^2 = 1)
    double norm = std::sqrt(magnitude_alpha * magnitude_alpha + magnitude_beta * magnitude_beta);
    if (std::abs(norm - 1.0) > 1e-6) {
        std::cerr << "Warning: The state is not normalized!" << std::endl;
    }

    // Compute the polar angle (theta)
    theta_out = 2 * std::acos(magnitude_alpha);

    // Compute the azimuthal angle (phi)
    if (std::abs(magnitude_alpha) > 1e-6) {  // To avoid division by zero when alpha is 0
        phi_out = std::arg(beta) - std::arg(alpha);
    } else {
        phi_out = std::arg(beta);  // If alpha is real, just use the argument of beta
    }

    // Normalize phi to be in the range [0, 2*pi]
    if (phi_out < 0) {
        phi_out += 2 * M_PI;
    }

}

int main(int argc, char *argv[]) {

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



       constexpr size_t numQubits = 1;
       constexpr size_t numStates = 1 << numQubits; // 2^n
                                                    
               
       std::complex<float> *stateVector   = new std::complex<float>[numStates];
       std::complex<float> *stateVector_d = malloc_device<std::complex<float>>(numStates,queue);

       // Initial state |00...00>
       stateVector[0] = std::complex<float>(1.0f,0.0f);
       queue.memcpy(stateVector_d, stateVector, numStates * sizeof(std::complex<float>)).wait();


       for(int i = 0; i < numQubits; i++){
          rx(queue, stateVector_d, numQubits,i,M_PI/4);
       }


       for(int i = 0; i < numQubits; i++){
          ry(queue, stateVector_d, numQubits,i,M_PI/4);
       }


       for(int i = 0; i < numQubits; i++){
          rz(queue, stateVector_d, numQubits,i,M_PI/4);
       }

       measure(queue, stateVector_d, numQubits, 1000);
       queue.memcpy(stateVector, stateVector_d, numStates * sizeof(std::complex<float>)).wait();
       state2angle(stateVector[0],stateVector[1],theta,phi);
       bloch_sphere(argc, argv);


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
