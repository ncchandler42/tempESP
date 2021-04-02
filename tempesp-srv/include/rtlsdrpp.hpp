#ifndef RTLSDRPP_HPP
#define RTLSDRPP_HPP

// This file is part of rtlsdrpp.
// Copyright (C) 2020 by Nolan Chandler <https://github.com/ncchandler42>
//
// rtlsdrpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// rtlsdrpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with rtlsdrpp.  If not, see <http://www.gnu.org/licenses/>.

#include <vector>
#include <map>
#include <string>
#include <complex>
#include <exception>
#include <algorithm>
#include <cstdint>
#include <cmath>

#include "rtl-sdr.h"

namespace rtlsdr {

const std::uint32_t DEFAULT_GAIN = 0; // automatic
const std::uint32_t DEFAULT_FC = 80e6;
const std::uint32_t DEFAULT_RS = 1.024e6;
const int DEFAULT_READ_SIZE = 1024;

const long CRYSTAL_FREQ = 28800000;

const std::map<int, std::string> errno_map = {
	{-1, "LIBUSB_ERROR_IO"},
	{-2, "LIBUSB_ERROR_INVALID_PARAM"},
	{-3, "LIBUSB_ERROR_ACCESS"},
	{-4, "LIBUSB_ERROR_NO_DEVICE"},
	{-5, "LIBUSB_ERROR_NOT_FOUND"},
	{-6, "LIBUSB_ERROR_BUSY"},
	{-7, "LIBUSB_ERROR_TIMEOUT"},
	{-8, "LIBUSB_ERROR_OVERFLOW"},
	{-9, "LIBUSB_ERROR_PIPE"},
	{-10, "LIBUSB_ERROR_INTERRUPTED"},
	{-11, "LIBUSB_ERROR_NO_MEM"},
	{-12, "LIBUSB_ERROR_NOT_SUPPORTED"},
	{-99, "LIBUSB_ERROR_OTHER"}
};

class LibUSBException: public std::exception {
public:
	LibUSBException(int errno_, const std::string& msg_="") noexcept {
		msg = "<" + errno_map.at(errno_) + ": " + msg_ + ">";
	}

	const char* what() const noexcept {
		return msg.c_str();
	}
	
private:
	std::string msg;
};

int get_device_index_by_serial(const std::string& serial) {
	// Retrieves the device index for a device matching the given serial number
	// 
	// Arguments:
	//	serial (str): The serial number to search for
	// 
	// Returns:
	//	int: The device_index as reported by ``librtlsdr``
	// 
	// Notes:
	//	Most devices by default have the same serial number: `'00000001'`.
	//	This can be set to a custom value by using the `rtl\_eeprom`_ utility
	//	packaged with ``librtlsdr``.
	//

	int result = rtlsdr_get_index_by_serial(serial.c_str());
	if (result < 0) {
		throw LibUSBException(result);
	}

	return result;
}

std::vector<std::string> get_device_serial_addresses() {
	// Get serial numbers for all attached devices
	// 
	// Returns:
	// 	list(str): A ``list`` of all detected serial numbers (``str``)
	//

	auto get_serial = [](std::uint32_t index) {
		char bfr[256];
		int r = rtlsdr_get_device_usb_strings(index, nullptr, nullptr, bfr);
		if (r != 0){
			throw LibUSBException(r, "while reading USB strings (device " + std::to_string(index) + ")");
		}
		return std::string(bfr);			
	};

	std::vector<std::string> addresses;

	for (std::uint32_t i = 0; i < rtlsdr_get_device_count(); i++) {
		addresses.emplace_back(get_serial(i));
	}

	return addresses;
}

class BaseRtlSdr {
public:
	BaseRtlSdr(std::uint32_t index=0, bool test_mode_enabled=false, const std::string& serial_number="") {
		open(index, test_mode_enabled, serial_number);
	}

	void open(std::uint32_t index=0, bool test_mode_enabled=false, const std::string& serial_number="") {
		if (serial_number != "") {
			index = get_device_index_by_serial(serial_number);
		}

		// initialize device
		int result = rtlsdr_open(&dev_p, index);
		if (result < 0) {
			throw LibUSBException(result, "Could not open SDR (device index = " + std::to_string(index) + ")");
		}

		// set test mode if necessary
		result = rtlsdr_set_testmode(dev_p, test_mode_enabled);
		if (result < 0) {
			throw LibUSBException(result, "Could not set test mode");
		}

		// reset buffers
		result = rtlsdr_reset_buffer(dev_p);
		if (result < 0) {
			throw LibUSBException(result, "Could not reset buffer");
		}

		device_opened = true;
		init_device_values();
	}

	void init_device_values() {
		gain_values = get_gains();
		valid_gains_db.resize(gain_values.size());
		std::transform(
			std::begin(gain_values), std::end(gain_values), std::begin(valid_gains_db),
			[](int gain_val) { return gain_val/10.0; }
		);
	
		set_sample_rate(DEFAULT_RS);
		set_center_freq(DEFAULT_FC);
		set_gain(DEFAULT_GAIN);
	}

	void close() {
		if (!device_opened) {
			return;
		}

		rtlsdr_close(dev_p);
		device_opened = false;
	}

	~BaseRtlSdr() { close(); }
	
	void set_center_freq(std::uint32_t freq) {
		int result = rtlsdr_set_center_freq(dev_p, freq);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set center freq to " + std::to_string(freq) + " Hz");
		}
	}

	std::uint32_t get_center_freq() {
		std::uint32_t result = rtlsdr_get_center_freq(dev_p);
		if (result == 0) {
			close();
			throw LibUSBException(result, "Could not get center freq");
		}

		return result;
	}

	void set_freq_correction(int err_ppm) {
		int result = rtlsdr_set_freq_correction(dev_p, err_ppm);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set freq offset to " + std::to_string(err_ppm) + " ppm");
		}
	}

	int get_freq_correction() {
		int result = rtlsdr_get_freq_correction(dev_p);
		if (result == 0) {
			close();
			throw LibUSBException(result, "Could not get freq offset");
		}

		return result;
	}

	void set_sample_rate(std::uint32_t rate) {
		int result = rtlsdr_set_sample_rate(dev_p, rate);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set sample rate to " + std::to_string(rate) + " Hz");
		}
	}

	std::uint32_t get_sample_rate() {
		std::uint32_t result = rtlsdr_get_sample_rate(dev_p);
		if (result == 0) {
			close();
			throw LibUSBException(result, "Could not get sample rate");
		}

		std::uint32_t reported_sample_rate = result;
		std::uint32_t rsamp_ratio = (CRYSTAL_FREQ * std::pow(2, 22)) / reported_sample_rate;
		rsamp_ratio &= ~3;
		std::uint32_t real_rate = (CRYSTAL_FREQ * std::pow(2, 22)) / rsamp_ratio;
		
		return real_rate;
	}

	void set_bandwidth(std::uint32_t bw) {
		int result = rtlsdr_set_tuner_bandwidth(dev_p, bw);
		if (result == 0) {
			close();
			throw LibUSBException(result, "Could not set tuner bandwidth to " + std::to_string(bw) + " Hz");
		}
	}

	void set_gain(int gain) {
		if (gain == 0) {
			set_manual_gain_enabled(false);
			return;
		}

		set_manual_gain_enabled(true);

		std::vector<int> errors(gain_values.size());
		std::transform(
			std::begin(gain_values), std::end(gain_values), std::begin(errors),
			[&](int g) { return std::abs(10*gain - g); }
		);

		auto nearest_gain_ind = std::distance(
			std::begin(errors), std::min_element(std::begin(errors), std::end(errors))
		);

		int result = rtlsdr_set_tuner_gain(dev_p, gain_values[nearest_gain_ind]);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set gain to " + std::to_string(gain));
		}
	}

	int get_gain() {
		int result = rtlsdr_get_tuner_gain(dev_p);
		if (result == 0) {
			close();
			throw LibUSBException(result, "Could not get current gain");
		}
		
		return result/10;
	}

	std::vector<int> get_gains() {
		int buffer[50];
		int result = rtlsdr_get_tuner_gains(dev_p, buffer);
		if (result <= 0) {
			close();
			throw LibUSBException(result, "Could not get list of gains");
		}

		return std::vector<int>(buffer, buffer+result);
	}

	void set_manual_gain_enabled(bool enabled) {
		int result = rtlsdr_set_tuner_gain_mode(dev_p, enabled);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set gain mode");
		}
	}

	void set_agc_mode(bool enabled) {
		int result = rtlsdr_set_agc_mode(dev_p, enabled);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set AGC mode");
		}
	}

	void set_direct_sampling(int direct) {
		// 0 = disabled, 1 = I ADC, 2 = Q ADC
		int result = rtlsdr_set_direct_sampling(dev_p, direct);
		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not set direct sampling");
		}	
	}

	int get_tuner_type() { return (int)rtlsdr_get_tuner_type(dev_p); }

	std::vector<unsigned char> read_bytes(std::size_t num_bytes=DEFAULT_READ_SIZE) {
		if (buffer.size() != num_bytes) {
			buffer.resize(num_bytes);
		}
		
		int n_read;
		int result = rtlsdr_read_sync(dev_p, buffer.data(), num_bytes, &n_read);

		if (result < 0) {
			close();
			throw LibUSBException(result, "Could not read " + std::to_string(num_bytes) + " bytes");
		}
		
		if ((std::size_t)n_read != num_bytes) {
			close();
			throw LibUSBException(result, "Short read, requested " + std::to_string(num_bytes) + 
				", received " + std::to_string(n_read) + " bytes");
		}

		return buffer;
	}
	
	std::vector<double> read_samples_direct(std::size_t num_samples) {
		std::vector<double> samples(num_samples);
		auto raw_data = read_bytes(num_samples);
		
		for (std::size_t i = 0; i < num_samples; i++) {
			samples[i] = raw_data[i]/(255.0/2.0) -1;
		}
		
		return samples;
	}

	std::vector<std::complex<double>> read_samples(std::size_t num_samples) {
		std::size_t num_bytes = 2*num_samples;
		auto raw_data = read_bytes(num_bytes);

		return packed_bytes_to_iq(raw_data);
	}

	std::vector<std::complex<double>> packed_bytes_to_iq(const std::vector<unsigned char>& bytes) {
		std::vector<std::complex<double>> iq;

		for (std::size_t i = 0; i < bytes.size()-1; i+=2) {
			iq.emplace_back(bytes[i]/(255.0/2.0) -1, bytes[i+1]/(255.0/2.0) -1);
		}

		return iq;
	}

private:
	rtlsdr_dev* dev_p;
	bool device_opened;

	std::vector<int> gain_values;
	std::vector<double> valid_gains_db;
	std::vector<unsigned char> buffer;
};

// TODO Add async
class RtlSdr: public BaseRtlSdr {
	
};
	
} // namespace rtlsdr

#endif
