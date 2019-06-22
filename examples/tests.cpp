#include <iostream>
#include <cstdio>
#include "include/easysock/tcp.hpp"
#include "include/easysock/http.hpp"

void* OnConnect(void* pointer) {
	auto conn = (easysock::tcp::Client*)pointer;
	std::string data = conn->read();
	conn->write(data + "? Hello too!");
	conn->destroy();

	pthread_exit(NULL);
	return NULL;
}

void* AsyncServer(void*) {
	auto srv = easysock::tcp::createServer(6868);
	if(srv == nullptr) {
		std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
	} else {
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

	pthread_exit(NULL);
	return NULL;
}

int main() {
	easysock::init();

	std::cout << "Creating TCP Server at port 6868..." << std::endl;
	pthread_t server;
	pthread_create(&server, NULL, AsyncServer, nullptr);

	std::cout << "Connecting to TCP Server at port 6868..." << std::endl;
	auto conn = easysock::tcp::connect("localhost", 6868);
	if(conn == nullptr) {
		std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
	} else {
		if(conn->write("Hello world!") < 0) {
			std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
		} else {
			std::cout << "Response data: " << conn->read() << std::endl;
		}
	}

	delete conn;

	std::cout << std::endl << "============================================================" << std::endl;

	easysock::http::defaultSettings->version = "HTTP/1.0";

	auto settings = new easysock::http::Settings();
	settings->headers = { { "Host", "themaking.my.to" } };

	auto conn2 = easysock::http::connect("localhost", 80, "/", settings);
	if(conn2 == nullptr) {
		// std::cout << "ERROR " << easysock::lastError << " in " << easysock::lastErrorPlace << std::endl;
		std::cout << WSAGetLastError() << std::endl;
	} else {
		std::cout << conn2->readAll() << std::endl;
	}

	delete conn2;

	easysock::exit();
	return 0;
}
