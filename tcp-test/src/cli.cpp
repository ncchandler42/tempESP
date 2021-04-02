#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "simpletcp.hpp"

int main(int argc, char* argv[]) {
	std::string ip = "127.0.0.1";
	int port = 50001;

	if (argc == 2) {
		ip = std::string(argv[1]);
	}
	else if (argc == 3) {
		ip = std::string(argv[1]);
		port = std::atoi(argv[2]);
	}
	
	tcp::TcpClient cli;
	cli.connect_to_server(ip, port); 

	auto msg = cli.recv_bytes();
	std::string msg_s(std::prev(std::end(msg), 4), std::end(msg));
	std::cout << msg_s << "\n";

	std::cout << "sending reply... \n";
	std::string reply = "yah, k";
	cli.send_bytes({std::begin(reply), std::end(reply)});

	std::cout << "Done, closing in 5 seconds\n";
	std::this_thread::sleep_for(std::chrono::seconds(5));
}