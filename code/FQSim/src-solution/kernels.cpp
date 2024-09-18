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

    std::complex<float> A (1.0f,0.0f);
    std::complex<float> B (1.0f,0.0f);
    std::complex<float> C (1.0f,0.0f);
    std::complex<float> D (-1.0f,0.0f);
    apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A/std::sqrt(2.0f),
                                                                  B/std::sqrt(2.0f),
                                                                  C/std::sqrt(2.0f),
                                                                  D/std::sqrt(2.0f));
    
}
void z(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target){

    std::complex<float> A (1.0f,0.0f);
    std::complex<float> B (0.0f,0.0f);
    std::complex<float> C (0.0f,0.0f);
    std::complex<float> D (-1.0f,0.0f);
    apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A,
                                                                B,
                                                                C,
                                                                D);
}

void get_proba(sycl::queue &queue, std::complex<float> *stateVector_d,const unsigned int numStates,float *probaVector_d){

  queue.parallel_for<class Proba>(sycl::range<1>(numStates),[=]( sycl::item<1> item) {
			  int global_id = item.get_id(0);
              std::complex<float> amp = stateVector_d[global_id];
              probaVector_d[global_id] = std::abs(amp * amp);
          }).wait();

}

void measure(sycl::queue &queue,std::complex<float> *stateVector_d, int numQubits,int samples){
    int size = std::pow(2,numQubits);
    float *probaVector = new float[size];
    float *probaVector_d = malloc_device<float>(size,queue);
    get_proba(queue,stateVector_d,size,probaVector_d); 
    queue.memcpy(probaVector, probaVector_d, size * sizeof(float)).wait();
    std::random_device rd;                          // Obtain a random number from hardware
    std::mt19937 gen(rd());                         // Seed the generator
    std::discrete_distribution<> dist(probaVector, probaVector + size);
    std::vector<int> arr(size);
    for(int i = 0; i < samples; i++){
        int index = dist(gen);
        arr[index]++;
    }
    std::cout << "Quantum State Probabilities:" << std::endl;
    for (size_t i = 0; i < size; ++i) {
        std::cout << "State " <<toBinary(i,numQubits) << ": " << arr[i] << std::endl;
    }
    delete[] probaVector;

}


void rx(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle){

    double angle_2 = 0.5*angle;
    double cos = std::cos(angle_2);
    double sin = std::sin(angle_2);
    std::complex<float> A (cos,0.0f);
    std::complex<float> B (0.0f,-1.0*sin);
    std::complex<float> C (0.0f,-1.0*sin);
    std::complex<float> D (cos,0.0f);
    apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A,
                                                                  B,
                                                                  C,
                                                                  D);
}

void ry(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle){

    double angle_2 = 0.5*angle;
    double cos = std::cos(angle_2);
    double sin = std::sin(angle_2);
    std::complex<float> A (cos,0.0f);
    std::complex<float> B (-1.0*sin,0.0f);
    std::complex<float> C (sin,0.0f);
    std::complex<float> D (cos,0.0f);
    apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A,
                                                                  B,
                                                                  C,
                                                                  D);
}

void rz(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle){

    double angle_2 = 0.5*angle;
    double cos = std::cos(angle_2);
    double sin = std::sin(angle_2);

    std::complex<float> A (cos,-sin);
    std::complex<float> B (0.0f,0.0f);
    std::complex<float> C (0.0f,0.0f);
    std::complex<float> D (cos,sin);
    apply_gate(queue,stateVector_d,std::pow(2,numQubits)/2,target,A,
                                                                  B,
                                                                  C,
                                                                  D);
}
