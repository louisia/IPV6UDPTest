/*
 * Main.cpp
 *
 *  Created on: 2016年11月30日
 *      Author: louisia
 */

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include<sstream>
using namespace std;

//服务器端开放的消息接收端口
#define PORT_RECV 8000
//服务器端开放的消息发送端口
#define PORT_SEND 8001

#define PORT_MIN 9000
//网络节点的数目
#define NODE_NUM 3

void runServer(char *serverIP) {
	struct sockaddr_in6 server;
	std::vector<sockaddr_in6> client_addrs;
	char buf[2048];

	int sock_recv;
	int sock_send;
	if ((sock_recv = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		cout << "create sock_recv failed" << endl;
		exit(errno);
	}
	if ((sock_send = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		cout << "create sock_send failed" << endl;
		exit(errno);
	}

	memset(&server, 0, sizeof(struct sockaddr_in6));
	server.sin6_family = AF_INET6;
	server.sin6_port = htons(PORT_RECV);
	if (inet_pton(AF_INET6, serverIP, &server.sin6_addr))
		cout << "ip is correct" << endl;
	else {
		cout << "ip is wrong" << endl;
		;
		exit(errno);
	}

	if ((bind(sock_recv, (struct sockaddr *) &server, sizeof(server))) == -1) {
		cout << "bind sock_recv failed:" << ntohs(server.sin6_port) << endl;
		exit(errno);
	} else {
		cout << "bind sock_recv succeed: " << ntohs(server.sin6_port) << endl;

	}

	server.sin6_port = htons(PORT_SEND);

	if ((bind(sock_send, (struct sockaddr *) &server, sizeof(server))) == -1) {
		cout << "bind sock_send failed:" << ntohs(server.sin6_port) << endl;
		exit(errno);
	} else {
		cout << "bind sock_send succeed: " << ntohs(server.sin6_port) << endl;

	}

	while (1) {

		struct sockaddr_in6 client_addr;
		socklen_t client_addr_len = sizeof(client_addr);

		memset(buf, 0, sizeof(buf));
		int recv_num = recvfrom(sock_recv, buf, sizeof(buf) - 1, 0,
				(struct sockaddr *) &client_addr, &client_addr_len);
		if (recv_num < 0) {
			cout << "recvfrom failed" << endl;
			exit(errno);
		} else {
			client_addr.sin6_port = htons(atoi(buf));
			client_addrs.push_back(client_addr);

			if (client_addrs.size() == NODE_NUM) {
				int pos = 0;
				memset(buf, 0, sizeof(buf));

				for (vector<int>::size_type i = 0; i < NODE_NUM; i++) {
					memcpy(buf + pos, &client_addrs[i].sin6_addr,
							sizeof(client_addrs[i].sin6_addr));
					pos += sizeof(client_addrs[i].sin6_addr);

					memcpy(buf + pos, &client_addrs[i].sin6_port,
							sizeof(client_addrs[i].sin6_port));
					pos += sizeof(client_addrs[i].sin6_port);
				}

				for (vector<int>::size_type i = 0; i < NODE_NUM; i++) {
					client_addr = client_addrs[i];
					int send_num = sendto(sock_send, buf, pos, 0,
							(struct sockaddr *) &client_addr, client_addr_len);
					if (send_num < 0) {
						cout << "send failed" << endl;
						exit(1);
					}
				}
				break;
			}
		}

	}
}

void runClient(char *serverIp, char *clientname) {
	struct sockaddr_in6 controller_addr;
	struct sockaddr_in6 self_addr;

	vector<sockaddr_in6> neighbours;

	int sock_send;
	int sock_recv;

	char buf[2048];

	//create socket
	if ((sock_send = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		cout << "sock_send created failed"<<endl;
		exit(errno);
	}
	cout << "------------------------------------------" << endl;
	if ((sock_recv = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		cout << "sock_recv created failed"<<endl;
		exit(errno);
	}

	memset(&self_addr, 0, sizeof(struct sockaddr_in6));
	self_addr.sin6_family = AF_INET6;
	self_addr.sin6_port = htons(PORT_MIN);
	self_addr.sin6_addr = in6addr_any;

	while ((bind(sock_send, (struct sockaddr *) &self_addr, sizeof(self_addr)))
			== -1) {
		cout << "send bind failed" << ntohs(self_addr.sin6_port) << endl;
		int port = ntohs(self_addr.sin6_port);
		self_addr.sin6_port = htons(++port);
	}
	cout << "bind sock_send succeed: " << ntohs(self_addr.sin6_port) << endl;

	while ((bind(sock_recv, (struct sockaddr *) &self_addr, sizeof(self_addr)))
			== -1) {
		cout << "recv bind failed " << ntohs(self_addr.sin6_port) << endl;
		int port = ntohs(self_addr.sin6_port);
		self_addr.sin6_port = htons(++port);
	}
	cout << "bind sock_recv succeed " << ntohs(self_addr.sin6_port) << endl;

	controller_addr.sin6_family = AF_INET6;
	if (inet_pton(AF_INET6, serverIp, &controller_addr.sin6_addr))
		cout << "ip is correct" << endl;
	else {
		cout << "ip is wrong" << endl;
		exit(1);
	}
	controller_addr.sin6_port = htons(PORT_RECV);

	memset(buf, 0, sizeof(buf));
	stringstream ss;
	ss << ntohs(self_addr.sin6_port);
	strcpy(buf, ss.str().c_str());
	if (sendto(sock_send, buf, strlen(buf), 0,
			(struct sockaddr *) &controller_addr, sizeof(controller_addr))
			< 0) {
		cout << "send msg failed" << endl;
		exit(1);
	} else {
		cout << "send msg success" << endl;
		socklen_t server_len = sizeof(struct sockaddr_in);
		memset(buf, 0, sizeof(buf));
		int recv_num = recvfrom(sock_recv, buf, sizeof(buf), 0,
				(struct sockaddr *) &controller_addr, &server_len);
		if (recv_num < 0) {
			cout << "recv msg failed" << endl;
			exit(errno);
		} else {
			int pos = 0;
			for (int i = 0; i < NODE_NUM; i++) {
				in6_addr addr;
				memcpy(&addr, buf + pos, sizeof(in6_addr));

				char tmp[128];
				inet_ntop(AF_INET6, &addr, tmp, sizeof(tmp));
				pos += sizeof(in6_addr);

				in_port_t port;
				memcpy(&port, buf + pos, sizeof(in_port_t));
				pos += sizeof(in_port_t);

				if (port != self_addr.sin6_port) {
					std::cout << "addr: " << tmp << std::endl;
					std::cout << "port: " << ntohs(port) << std::endl;

					sockaddr_in6 neighbor;
					neighbor.sin6_addr = addr;
					neighbor.sin6_port = port;
					neighbor.sin6_family = AF_INET6;
					neighbours.push_back(neighbor);
				}
			}

			memset(buf, 0, sizeof(buf));
			strcpy(buf, "hello from ");
			memcpy(buf + 11, clientname, sizeof(clientname));
			for (vector<sockaddr_in6>::size_type i = 0; i < neighbours.size();
					i++) {
				if (sendto(sock_send, buf, strlen(buf), 0,
						(struct sockaddr *) &neighbours[i],
						sizeof(neighbours[i])) == -1) {
					cout << "send to failed" << endl;
				}
			}

			sockaddr_in6 neighbor;
			for (vector<sockaddr_in6>::size_type i = 0; i < neighbours.size();
					i++) {
				memset(buf, 0, sizeof(buf));
				if (recvfrom(sock_recv, buf, sizeof(buf), 0,
						(struct sockaddr *) &neighbor, &server_len) != -1) {
					cout << buf << endl;
				}
			}

		}

	}

}
int main(int argc, char **args) {

	char *serverIp = "::1";
	if (strcmp(args[1], "server") == 0) {
		runServer(serverIp);
	} else {
		runClient(serverIp, args[1]);
	}

	return 0;
}
