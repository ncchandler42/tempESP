#include <iostream>
#include <fstream>
#include <vector>

#include "rtlsdrpp.hpp"

int main() {
	rtlsdr::RtlSdr sdr;

	sdr.set_sample_rate(2.4e6);
	sdr.set_center_freq(91.5e6);
	sdr.set_gain(0);

	auto samples = sdr.read_samples(256*1024);

	sdr.close();

	std::ofstream fout("../data.txt");
	for (auto& sample: samples) {
		fout << sample.real() << "+" << sample.imag() << "j\n";
	}
	fout.close();
}
