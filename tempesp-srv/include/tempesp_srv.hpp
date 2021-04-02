#ifndef TEMPESPSRV_HPP
#define TEMPESPSRV_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cmath>

#include <opencv2/opencv.hpp>

#include "constants.hpp"
#include "simpletcp.hpp"
#include "rtlsdrpp.hpp"
#include "csv.hpp"

using cv::ml::TrainData;
using cv::ml::ANN_MLP;

///////////////////////////////////////////////////////////

enum CmdCode {
	CMD_STOP = 0,
	CMD_RECV_IMG,
	CMD_DISPLAY_IMG	
};

enum RespCode {
	RESP_FAILED = 0,
	RESP_RECV_SUCCESS,
	RESP_DISPLAY_SUCCESS
};

///////////////////////////////////////////////////////////

class TempespSrv {
public:
	TempespSrv(int port);
	
	///////////////////////////////////////////////////////////
	// TCP FUNCS
	///////////////////////////////////////////////////////////
	
	void accept_cli();
	void send_cmd(CmdCode code);

	void load_img(const std::string& path);
	void load_img(std::size_t n_img);
	void send_img();
	
	///////////////////////////////////////////////////////////
	// SDR FUNCS
	///////////////////////////////////////////////////////////
	
	void conf_sdr();
	void collect_em_data(float flo, float fhi, std::size_t nsteps);
	void write_to_tdfile(std::size_t img_n);
	
	///////////////////////////////////////////////////////////
	// MLP FUNCS
	///////////////////////////////////////////////////////////
	
	void load_MLP_model();
	void save_MLP_model();
	void train_MLP_model();
	cv::Mat predict_img();

	///////////////////////////////////////////////////////////
	
private:
	tcp::TcpServer tcpsrv;	
	std::vector<unsigned char> tcpdata;
	cv::Mat loaded_img;

	rtlsdr::RtlSdr sdr;
	std::vector<float> psd;

	cv::Ptr<ANN_MLP> mlp;
};

//////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////

TempespSrv::TempespSrv(int port): tcpsrv(port) { 
	conf_sdr();
	load_MLP_model();
	accept_cli(); 

	psd.resize(NSAMPS/2);
}

///////////////////////////////////////////////////////////
// TCP FUNCS
///////////////////////////////////////////////////////////

void TempespSrv::accept_cli() { tcpsrv.accept_client(); }

void TempespSrv::send_cmd(CmdCode code) {
	tcpsrv.send_bytes({code});
}

void TempespSrv::load_img(const std::string& path) {
	loaded_img = cv::imread(path, cv::IMREAD_GRAYSCALE);
}

void TempespSrv::load_img(std::size_t n_img) {
	std::string path = "../esp-imgs/" + std::to_string(n_img) + ".png";
	load_img(path); 
}

void TempespSrv::send_img() {
	send_cmd(CMD_RECV_IMG);
	tcpsrv.send_bytes(loaded_img.reshape(1, loaded_img.total()));

	tcpdata = tcpsrv.recv_bytes();
	if (tcpdata[0] != RESP_RECV_SUCCESS) {
		throw std::runtime_error("Failed to send image");
	}

	send_cmd(CMD_DISPLAY_IMG);
	tcpdata = tcpsrv.recv_bytes();
	if (tcpdata[0] != RESP_DISPLAY_SUCCESS) {
		throw std::runtime_error("Failed to display image");
	}
}

///////////////////////////////////////////////////////////
// SDR FUNCS
///////////////////////////////////////////////////////////

void TempespSrv::conf_sdr() {
	sdr.set_sample_rate(2.4e6);
	sdr.set_direct_sampling(2);
	sdr.set_gain(0);
}

void TempespSrv::collect_em_data(float flo, float fhi, std::size_t nsteps) {
	float fcent = flo;
	float logstep = std::pow(fhi/flo,  1.0/nsteps);

	std::vector<std::complex<double>> fft_n(NSAMPS);

	// Resize (if needed) and zero values to prepare for sum
	std::for_each(std::begin(psd), std::end(psd), [](auto& f) { f = 0; });
	
	while (fcent <= fhi) {
		sdr.set_center_freq(fcent);
		auto samples = sdr.read_samples_direct(NSAMPS);
		
		// Welch windowing function
		double N;
		for (std::size_t n = 0; n < NSAMPS; n++) {
			N = (n - NSAMPS/2.0) / (NSAMPS/2.0);
			samples[n] *= 1.0 - N*N;
		}

		cv::dft(samples, fft_n, cv::DFT_COMPLEX_OUTPUT);

		// don't care about 0 Hz component
		for (std::size_t i = 0; i < NSAMPS/2; i++)
			psd[i] += std::norm(fft_n[i]);
		
		fcent *= logstep;
	}

	// Normalize power spectrum, shift and scale so that 
	// the mean is 0 and stddev is 1
	
	float maxp = *std::max_element(std::cbegin(psd), std::cend(psd));
	std::for_each(std::begin(psd), std::end(psd),
		[maxp](float& d) {  d = (2 * d / maxp) - 1; }
	);
	/*
	float mean = std::accumulate(std::cbegin(psd)+1, std::cend(psd), 0.0) / (NSAMPS/2.0);
	std::for_each(std::begin(psd)+1, std::end(psd),
		[mean](float& d) {  d -= mean; }
	);
	*/	
}


void TempespSrv::write_to_tdfile(std::size_t img_n) {
	// traindata is always appended to
	// Nothing is done to control the size of the file
	// Just uhhh, be reasonable about it
	std::ofstream fout("../traindata.csv", std::ios::app);
	csv::Writer td_csv(fout);
	
	std::vector<float> outp(NOUTPUTS, -0.999999);
	outp[img_n] = 0.999999;
	
	td_csv.write_fields(psd);
	td_csv.write_row(outp);
}

///////////////////////////////////////////////////////////
// MLP FUNCS
///////////////////////////////////////////////////////////

void TempespSrv::load_MLP_model() {
	std::cout << "Loading MLP model \"../MLP_model.yml\"..." << std::flush;
	try {
		mlp = ANN_MLP::load("../MLP_model.yml");
		std::cout << " done!" << std::endl;
	}
	catch (cv::Exception& err) {
		std::cout << "\nModel does not exist. Reinitializing...\n";

		mlp = cv::ml::ANN_MLP::create();
		
		cv::Mat_<int> layer_s(NLAYERS, 1);
		
		layer_s(0) = NINPUTS;
		layer_s(NLAYERS-1) = NOUTPUTS;
		
		for (std::size_t i = 1; i < NLAYERS-1; i++) {
			layer_s(i) = (1.0*NOUTPUTS-NINPUTS)/(NLAYERS-1)*i + NINPUTS;
		}
	
		mlp->setLayerSizes(layer_s);
	
		auto crit = cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 10000, 1e-2);
		mlp->setTermCriteria(crit);
		
		mlp->setActivationFunction(ANN_MLP::SIGMOID_SYM, 1, 1);
		mlp->setTrainMethod(ANN_MLP::RPROP);
	
		mlp->save("../MLP_model.yml");
		std::cout << "Reinitialized model at \"../MLP_model.yml\"\n";
	}

}

void TempespSrv::save_MLP_model() {
	mlp->save("../MLP_model.yml");
}

void TempespSrv::train_MLP_model() {
	// (filename, n_header_lines, response_start_ind, response_end_ind)
	auto tdata = TrainData::loadFromCSV("../traindata.csv", 0, NINPUTS, NINPUTS+NOUTPUTS);
	tdata->setTrainTestSplitRatio(0.8);
	
	if (mlp->isTrained()) {
		mlp->train(tdata, ANN_MLP::UPDATE_WEIGHTS | ANN_MLP::NO_OUTPUT_SCALE);
	}
	else {
		mlp->train(tdata, ANN_MLP::NO_OUTPUT_SCALE);
	}
}

cv::Mat TempespSrv::predict_img() {
	// Hasn't been trained. Return empty mat
	if (!mlp->isTrained())
		return cv::Mat();
		
	cv::Mat data, pred;

	cv::Mat(psd).reshape(1, 1).convertTo(data, CV_32F);
	mlp->predict(data, pred);

	return pred;
}


#endif //TEMPESPSRV_HPP
