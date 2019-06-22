#include <iostream>
#include <cstdio>
#include "include/easysock/tcp.hpp"

void* OnConnect(void* pointer) {
	auto conn = (easysock::tcp::Client*)pointer;
	std::cout << conn->read() << std::endl;
	conn->destroy();

	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char* argv[]) {
	easysock::init();

	int port = 0;
	if(argc > 1) port = std::atoi(argv[1]);
	if(port == 0) port = 6868;

	auto srv = easysock::tcp::createServer(port);
	if(srv == nullptr) {
		std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
	} else {
		std::cout << "TCP Server listening on port " << port << std::endl;
		easysock::tcp::Client* conn;
		pthread_t thread;
	    while((conn = srv->listen()) != nullptr) {
	    	pthread_create(&thread, NULL, OnConnect, (void*)conn);
		}
		if(conn == nullptr) {
			std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
		}
	}

	delete srv;

	easysock::exit();
	return 0;
}
