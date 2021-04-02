#include <iostream>

#include "tempesp_srv.hpp"

const std::size_t NIMGS = 5;
const std::size_t NSETS_PER_IMG = 1;
const std::size_t NITERATIONS = 1;

int main() {
	int port = 50001;

	double flo = 500e3, fhi = 1.75e6;
	std::size_t nsteps_fsweep = 128;
	
	TempespSrv tsrv(port);

	for (std::size_t i = 0; i < NITERATIONS; i++) {
		for (std::size_t img_n = 0; img_n < NIMGS; img_n++) {
			tsrv.load_img(img_n);
			tsrv.send_img();
			
			for (std::size_t j = 0; j < NSETS_PER_IMG; j++) {
				tsrv.collect_em_data(flo, fhi, nsteps_fsweep);
				tsrv.write_to_tdfile(img_n);
				
				std::cout << "img=" << img_n << ",\tpredict=" << tsrv.predict_img() << std::endl;
			}
		}
		
		std::cout << "Training and saving model..." << std::endl;
		tsrv.train_MLP_model();		
		tsrv.save_MLP_model();
		std::cout << ((NITERATIONS-1) - i) << " iterations remain." << std::endl;
	}
	
	tsrv.send_cmd(CMD_STOP);
}
