#pragma once

#include <sycl/sycl.hpp>
#include <complex>

std::string toBinary(int num); 
SYCL_EXTERNAL int nth_cleared(int n, int target);
void apply_gate(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numStates,
                  const int target,
                  const std::complex<float> A,
                  const std::complex<float> B,
                  const std::complex<float> C,
                  const std::complex<float> D);
void h(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target);
void z(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target);

void get_proba(sycl::queue &queue, std::complex<float> *stateVector_d, const unsigned int numStates,
                                       float *probaVector_d);

void measure(sycl::queue &queue,std::complex<float> *stateVector_d,int numQubits,int samples);


void rx(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle);

void ry(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle);

void rz(sycl::queue &queue, std::complex<float> *stateVector_d,
				  const unsigned int numQubits,
                  const int target,
                  const double angle);
