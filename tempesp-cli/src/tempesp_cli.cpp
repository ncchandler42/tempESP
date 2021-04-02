#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include <opencv2/opencv.hpp>
#include "simpletcp.hpp"

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

class TempespCli {
public:
	~TempespCli() {
		cv::destroyWindow("TempESP");	
	}
	
	void connect(const std::string& ip, int port) {
		tcpcli.connect_to_server(ip, port);
	}

	CmdCode get_cmd() {
		buf = tcpcli.recv_bytes();
		if (buf.size() != 1) {
			return CMD_STOP;
		}

		return static_cast<CmdCode>(buf[0]);
	}
	
	void recv_image() {
		imgdata = tcpcli.recv_bytes();
		img = cv::Mat(imgdata).reshape(1, 768); // One channel, 768 rows

		tcpcli.send_bytes({RESP_RECV_SUCCESS});
	}

	void display_image() {
		cv::namedWindow("TempESP", cv::WINDOW_NORMAL);
		cv::setWindowProperty("TempESP", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
		cv::imshow("TempESP", img);
		cv::waitKey(250);

		tcpcli.send_bytes({RESP_DISPLAY_SUCCESS});
	}

	
private:
	tcp::TcpClient tcpcli;
	cv::Mat img;
	std::vector<unsigned char> imgdata, buf;
};

int main(int argc, char* argv[]) {

	if (argc < 3) {
		std::cout << "usage: " << argv[0] << " [server-ip] [server-port]\n";
		return 0;
	}

	std::string ip = std::string(argv[1]);
	int port = std::atoi(argv[2]);

	TempespCli tcli;
	tcli.connect(ip, port);

	bool stop = false;
	CmdCode code;
	while (!stop) {
		code = tcli.get_cmd();

		switch (code) {
			case CMD_RECV_IMG:
				tcli.recv_image();
				break;
				
			case CMD_DISPLAY_IMG:
				tcli.display_image();
				break;
				
			default:
			case CMD_STOP:
				stop = true;
				break;
		}
	}
}