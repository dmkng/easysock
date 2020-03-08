#ifndef __EASYSOCK_HTTP
#define __EASYSOCK_HTTP

#include "tcp.hpp"

namespace easysock {
	namespace http {
		std::map<int, std::string> statusCodes = {
			{ 100, "Continue" },
			{ 101, "Switching Protocols" },
			{ 102, "Processing" },
			{ 200, "OK" },
			{ 201, "Created" },
			{ 202, "Accepted" },
			{ 203, "Non-Authoritative Information" },
			{ 204, "No Content" },
			{ 205, "Reset Content" },
			{ 206, "Partial Content" },
			{ 207, "Multi-Status" },
			{ 300, "Multiple Choices" },
			{ 301, "Moved Permanently" },
			{ 302, "Found" },
			{ 303, "See Other" },
			{ 304, "Not Modified" },
			{ 305, "Use Proxy" },
			{ 306, "Switch Proxy" },
			{ 307, "Temporary Redirect" },
			{ 400, "Bad Request" },
			{ 401, "Unauthorized" },
			{ 402, "Payment Required" },
			{ 403, "Forbidden" },
			{ 404, "Not Found" },
			{ 405, "Method Not Allowed" },
			{ 406, "Not Acceptable" },
			{ 407, "Proxy Authentication Required" },
			{ 408, "Request Timeout" },
			{ 409, "Conflict" },
			{ 410, "Gone" },
			{ 411, "Length Required" },
			{ 412, "Precondition Failed" },
			{ 413, "Request Entity Too Large" },
			{ 414, "Request-URI Too Long" },
			{ 415, "Unsupported Media Type" },
			{ 416, "Requested Range Not Satisfiable" },
			{ 417, "Expectation Failed" },
			{ 418, "I'm a teapot" },
			{ 422, "Unprocessable Entity" },
			{ 423, "Locked" },
			{ 424, "Failed Dependency" },
			{ 425, "Unordered Collection" },
			{ 426, "Upgrade Required" },
			{ 449, "Retry With" },
			{ 450, "Blocked by Windows Parental Controls" },
			{ 500, "Internal Server Error" },
			{ 501, "Not Implemented" },
			{ 502, "Bad Gateway" },
			{ 503, "Service Unavailable" },
			{ 504, "Gateway Timeout" },
			{ 505, "HTTP Version Not Supported" },
			{ 506, "Variant Also Negotiates" },
			{ 507, "Insufficient Storage" },
			{ 509, "Bandwidth Limit Exceeded" },
			{ 510, "Not Extended" }
		};

		class Client {
		private:
			easysock::tcp::Client* client;
		public:
			Client(easysock::tcp::Client* client) {
				this->client = client;
			}

			~Client() {
				client->destroy();
			}

			std::string read(int &status) {
				return client->read(status);
			}

			std::string read() {
				return client->read();
			}

			std::string readAll() {
				return client->readAll();
			}
		};

		struct Settings {
			std::string method = "GET";
			std::string version = "HTTP/1.1";
			easysock::strmap headers;
			std::string body;
			int timeout = 5000;
		} *defaultSettings;

		Client* connect(std::string addr, int port, std::string path = "/", Settings* conn = defaultSettings) {
			if(path.empty() || conn->method.empty() || conn->version.empty()) return nullptr;

			auto tcpClient = easysock::tcp::connect(addr, port);
			if(tcpClient == nullptr) return nullptr;

			if(tcpClient->write(conn->method + " " + path + " " + conn->version + "\r\n" + easysock::toString(conn->headers, ": ", "\r\n") + "\r\n" + conn->body) < 1) {
				return nullptr;
			}

			int status;
			std::string data;
			clock_t time = clock();
			do {
				data += tcpClient->read(status);
			} while(status != -1 && data.find("\r\n\r\n") == std::string::npos && clock() - time < conn.timeout);

			if(data == "") return nullptr;

			easysock::strmap headers;
			std::string status;
			std::string body;

			// TODO: Parsing

			return new Client(tcpClient);
		}

		class Response {
		private:
			easysock::tcp::Client* client;
			std::string head;
			easysock::strmap headers;
			std::string body;
		public:
			Response(easysock::tcp::Client* client) {
				this->client = client;
			}

			~Response() {
				client->destroy();
			}

			bool writeHead(int statusCode, std::string status, easysock::strmap headers = {}) {
				if(status.empty()) return false;
				this->head = "HTTP/1.1 " + std::to_string(statusCode) + " " + status;

				for(auto const &header : headers) {
					this->headers.emplace(header.first, header.second);
				}

				return true;
			}

			bool writeHead(int status, easysock::strmap headers = {}) {
				if(statusCodes.find(status) == statusCodes.end()) return false;
				return writeHead(status, statusCodes[status], headers);
			}

			void write(std::string body) {
				if(!body.empty()) this->body += body;
			}

			bool end(std::string body = "") {
				if(this->head.empty() || this->headers.empty()) return false;

				write(body);
				if(client->end(this->head + "\r\n" + easysock::toString(this->headers, ": ", "\r\n") + "\r\n" + this->body) == -1) {
					return false;
				}

				return true;
			}
		};

		class Server {
		private:
			easysock::tcp::Server* server;
		public:
			Server(easysock::tcp::Server* server) {
				this->server = server;
			}

			virtual ~Server() {
				destroy();
			}

			Response* listen() {
				auto client = server->listen();
				if(client == nullptr) return nullptr;

				std::string data = client->readAll();
				if(data == "") return nullptr;

				auto resp = new Response(client);

				return resp;
			}

			void destroy() {
				server->destroy();
			}
		};

		Server* createServer(int port, std::string addr = "0.0.0.0") {
			auto tcpServer = easysock::tcp::createServer(port, addr);
			if(tcpServer == nullptr) return nullptr;

			return new Server(tcpServer);
		}
	}
}

#endif
