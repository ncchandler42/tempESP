#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "simpletcp.hpp"

int main(int argc, char* argv[]) {
	int port = 50001;
	if (argc == 2) {
		port = std::atoi(argv[1]);
	}

	tcp::TcpServer serv(port); 
	serv.accept_client();

	std::string bigmsg = "";
	for (int i = 0; i < 1024*768-4; i++) {
		bigmsg += "*";
	}
	bigmsg += "yay!";
	std::cout << "sizeof(bigmsg): " << bigmsg.size() << "\n";
	
	serv.send_bytes({std::begin(bigmsg), std::end(bigmsg)});

	std::cout << "waiting for reply... ";
	auto reply = serv.recv_bytes();
	std::cout << "reply: " <<  std::string(std::begin(reply), std::end(reply)) << "\n";

	std::cout << "Done, closing in 5 seconds\n";
	std::this_thread::sleep_for(std::chrono::seconds(5));
}