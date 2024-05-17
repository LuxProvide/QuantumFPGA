
#include "kernels.hpp"
#include <bitset>
#include <random>
#include <algorithm>
#include <numeric>
#include <string>


std::string toBinary(int num,int numQubits) {
    std::string result;
    for (int i = numQubits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        result += std::to_string(bit);
    }
    return result;
}

int nth_cleared(int n, int target)
{
    int mask = (1 << target) - 1;
    int not_mask = ~mask;

    return (n & mask) | ((n & not_mask) << 1);
}

void apply_gate(sycl::queue &queue, std::complex<float> *stateVector_d,const unsigned int numStates, 
                  const int target, 
                  const std::complex<float> A,
                  const std::complex<float> B,
                  const std::complex<float> C,
                  const std::complex<float> D)
{
  queue.parallel_for<class Gate>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
			  int global_id = item.get_id(0);
		      const int zero_state = nth_cleared(global_id,target);
    		  const int one_state = zero_state | (1 << target);
              std::complex<float> zero_amp = stateVector_d[zero_state];
              std::complex<float> one_amp = stateVector_d[one_state];
			  stateVector_d[zero_state] = A * zero_amp + B * one_amp;
			  stateVector_d[one_state] =  C * zero_amp + D * one_amp;

          }).wait();
}

void h(sycl::queue &queue, std::complex<float> *stateVector_d,const unsigned int numQubits,const int target){
    /*
    Code for the Pauli X gate here
    */
}
void z(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target){
    /*
    Put the code for the Pauli Z gate here
    */
}

void get_proba(sycl::queue &queue, std::complex<float> *stateVector_d,const unsigned int numStates,float *probaVector_d){

  queue.parallel_for<class Proba>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
			  int global_id = item.get_id(0);
              std::complex<float> amp = stateVector_d[global_id];
              probaVector_d[global_id] = std::abs(amp * amp);
          }).wait();

}

void measure(sycl::queue &queue,std::complex<float> *stateVector_d, int numQubits,int samples){
    /*
    Put the code to measure (sample) here
    */

}







