#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdlib>

const std::size_t NSAMPS = 512; // Samples per PSD periodogram

const std::size_t NINPUTS = NSAMPS/2;
const std::size_t NOUTPUTS = 5;
const std::size_t NLAYERS = 16;

#endif
