#ifndef SIMPLETCP_HPP
#define SIMPLETCP_HPP

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <iterator>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

namespace tcp {

const std::size_t BUFFER_SIZE = 1024+256;
double timeout_s = 5.0;

//============================================================================================

namespace {

std::array<unsigned char, BUFFER_SIZE> buf;
fd_set rfds, wfds;
timeval tv;

void set_tv(double timeout) {
	tv.tv_sec = static_cast<std::time_t>(timeout);
	tv.tv_usec = static_cast<std::time_t>(timeout*1e6) % 1000000;
}

void send_(int dest, const std::vector<unsigned char>& msg) { 
	std::size_t sent = 0;
	int res;

	set_tv(0.0);

	// tell receiving end size of msg
	std::string msgsize_s = std::to_string(msg.size());
	send(dest, msgsize_s.c_str(), msgsize_s.size(), 0);

	// wait for confirmation
	recv(dest, buf.data(), 1, 0);
	
	while (sent < msg.size()) {
		FD_ZERO(&wfds);
		FD_SET(dest, &wfds);
		select(dest+1, nullptr, &wfds, nullptr, &tv);

		if (!FD_ISSET(dest, &wfds)) { // if not immediately available
			set_tv(timeout_s);
			
			FD_ZERO(&wfds);
			FD_SET(dest, &wfds);
			select(dest+1, nullptr, &wfds, nullptr, &tv); // wait for a bit...
			
			if (!FD_ISSET(dest, &rfds)) {
				throw std::runtime_error("send_ timed out");
				break;
			}
			
			set_tv(0.0);
		}

		res = send(dest, std::next(msg.data(), sent), msg.size()-sent, 0);

		if (res < 0) {
			throw std::runtime_error("Failed to send");
		}

		sent += res;
	}

	// wait for confirmation
	recv(dest, buf.data(), 1, 0);
}

std::vector<unsigned char> recv_(int from) {
	int res;
	set_tv(0.0);
	FD_ZERO(&rfds);
	FD_SET(from, &rfds);
	select(from+1, &rfds, nullptr, nullptr, nullptr); // Block

	// receive size of incoming msg
	res = recv(from, buf.data(), BUFFER_SIZE, 0);
	std::size_t msgsize = std::stoll(std::string(std::begin(buf), std::next(std::begin(buf), res)));

	send(from, "?", 1, 0);
	
	std::vector<unsigned char> msg;
	msg.reserve(msgsize);
	
	while (msg.size() < msgsize) {
		res = recv(from, buf.data(), BUFFER_SIZE, MSG_DONTWAIT);
		if (res > 0) {
			msg.insert(std::end(msg), std::begin(buf), std::next(std::begin(buf), res));
		}
		else if (res == 0) {
			std::cerr << "Warning: recv returned 0, socket closed?\n";
			break;
		}
		else {			
			set_tv(timeout_s);

			FD_ZERO(&rfds);
			FD_SET(from, &rfds);
			select(from+1, &rfds, nullptr, nullptr, &tv); // wait for another event...
			if (!FD_ISSET(from, &rfds)) {
				throw std::runtime_error("recv_ timed out");
			}

			set_tv(0.0);
		}
	}

	// std::cout << "Done receiving, sending confirmation... ";	
	send(from, "!", 1, 0);
	
	return msg;
}

} // namespace 

//============================================================================================

class TcpServer {
public:
	TcpServer(): clisock(-1), port(-1) {
		srvsock = socket(AF_INET, SOCK_STREAM, 0);
		if (srvsock == -1) {
			throw std::runtime_error("Failed to create socket");
		}

		int optval = 1;
		int res = setsockopt(srvsock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		if (res < 0)  {
			throw std::runtime_error("Failed to set socket options");
		}
	} 
	
	TcpServer(int port_): TcpServer() { init(port_); }

	void init(int port_) {
		port = port_;
		sockaddr_in myaddr;
		memset(&myaddr, 0, sizeof(myaddr));

		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(port);
		myaddr.sin_addr.s_addr = 0; //inet_addr("192.168.1.114");

		if (bind(srvsock, reinterpret_cast<sockaddr*>(&myaddr), sizeof(myaddr)) < 0) {
			kill();
			throw std::runtime_error("Failed to bind server");
		}

		if(listen(srvsock, 1) < 0) {
			kill();
			throw std::runtime_error("Failed to open server to listen for connections");
		}
	}	
	
	//----------------------------------------------------------------------

	~TcpServer() { kill(); }

	void kill() {
		close(clisock);
		close(srvsock);
	}

	//----------------------------------------------------------------------

	void accept_client() {
		std::cout << "Waiting for connection on port " << port << "... " << std::flush;

		sockaddr_in cliaddr;
		socklen_t clilen = sizeof(cliaddr);

		clisock = accept(srvsock, reinterpret_cast<sockaddr*>(&cliaddr), &clilen);
		if (clisock < 0) {
			kill();
			throw std::runtime_error("Failed to accept client connection");
		}

		std::cout << "success!\n";
	}

	//----------------------------------------------------------------------
	
	void send_bytes(const std::vector<unsigned char>& msg) { 
		try { send_(clisock, msg); }
		catch (std::runtime_error& e) {
			kill();
			throw e;
		}
	}
	
	std::vector<unsigned char> recv_bytes() {
		try { return recv_(clisock); }
		catch (std::runtime_error& e) {
			kill();
			throw e;
		}
	}	
	
private:
	int srvsock, clisock;
	int port;
};

//============================================================================================

class TcpClient {
public:
	TcpClient() {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
			throw std::runtime_error("Failed to create socket");
		}
	}

	TcpClient(const std::string& ipaddr, int port, unsigned int maxattempts=0): TcpClient() {
		connect_to_server(ipaddr, port, maxattempts);
	}
	
	~TcpClient() { kill(); }

	void kill() { close(sock); }

	//----------------------------------------------------------------------
	void connect_to_server(const std::string& ipaddr, int port, unsigned int maxattempts=0) {
		std::cout << "Connecting to " << ipaddr << ":" << port << "... " << std::flush;
		
		sockaddr_in servaddr;
		memset(&servaddr, 0, sizeof(servaddr));

		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		servaddr.sin_addr.s_addr = inet_addr(ipaddr.c_str());

		for (unsigned int attempts = 0; attempts < maxattempts || maxattempts == 0; ++attempts) {

			if (connect(sock, reinterpret_cast<sockaddr*>(&servaddr), sizeof(servaddr)) < 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			else {
				std::cout << " success!\n";
				return;
			}
		}
		
		kill();
		throw std::runtime_error("Failed to connect to server after " + std::to_string(maxattempts) + " attempts");
	}

	//----------------------------------------------------------------------

	void send_bytes(const std::vector<unsigned char>& msg) {
		try { send_(sock, msg); }
		catch (std::runtime_error& e) {
			kill();
			throw e;
		} 
	}
	
	//----------------------------------------------------------------------

	std::vector<unsigned char> recv_bytes() {
		try { return recv_(sock); } 
		catch (std::runtime_error& e) {
			kill();
			throw e;
		}
	}	

private:
	int sock;
};

	
} //namespace tcp

#endif // SIMPLETCP_HPP
