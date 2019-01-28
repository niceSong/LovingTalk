#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "Connect.h"


NSLink::T_Link::T_Link(int domain, int type, int protocol)
	:flag(1)
{
	socket_fd = socket(domain, type, protocol);
	//std::cout << " link socket_fd ls" << socket_fd << std::endl;
	
	if(-1 == socket_fd)
	{
		std::cout << "socket  error!!!" << std::endl;
		exit(1);
	}
}


NSLink::T_Link::~T_Link()
{
	close(accept_fd);
	close(socket_fd);
}



void NSLink::T_Link::LTbind(unsigned short port)   
{
	int temp_bind;
	
	struct sockaddr_in Tlocaladdr;
	Tlocaladdr.sin_family = AF_INET;
	Tlocaladdr.sin_port = htons(port);
	Tlocaladdr.sin_addr.s_addr = htonl(INADDR_ANY);
	//Tlocaladdr.sin_addr.s_addr = inet_addr("192.168.1.111");
	
	temp_bind = bind(socket_fd, (struct sockaddr *)&Tlocaladdr, sizeof(struct sockaddr));
	if(-1 == temp_bind)
	{
		std::cout << "bind error!!!" << std::endl;
		exit(1);
	}
}

void NSLink::T_Link::LTlisten()
{
	int temp_listen;
	temp_listen = listen(socket_fd, 100);
	if(-1 == temp_listen)
	{
		std::cout << "listen error!!!" << std::endl;
		exit(1);
	}
}


void NSLink::T_Link::LTConnect(const char* ip,int port)
{
	int connect_temp;

	struct sockaddr_in Tserveraddr;
	Tserveraddr.sin_family = AF_INET;
	Tserveraddr.sin_port = htons(port);
	Tserveraddr.sin_addr.s_addr = inet_addr(ip);
	
	
	connect_temp = connect(socket_fd, (struct sockaddr *)&Tserveraddr, sizeof(struct sockaddr));
	std::cout << "HB socket_fd:" << socket_fd <<std::endl;
	if(-1 == connect_temp)
	{
		std::cout << "connect error!!!" << std::endl;
		std::cout << strerror(errno) << std::endl;
		exit(1);
	}
}


/* 为满足需求添加形参，需将accept的fd传入进去 */
void NSLink::T_Link::LTaccept()
{
	int len = sizeof(struct sockaddr);
	
	struct sockaddr_in Tclientaddr;
	
	accept_fd = accept(socket_fd, (struct sockaddr *)&Tclientaddr, (socklen_t *)&len);
	if(-1 == accept_fd)
	{
		std::cout << "accept error!!!" << std::endl;
		exit(1);
	}

	//std::cout << "client ip:prot ls:" << inet_ntoa(Tclientaddr.sin_addr)<< ":" << ntohs(Tclientaddr.sin_port) << std::endl;
}

void NSLink::T_Link::LTsend(const char *buf, int len, int flags)
{
	int sendbyte;
	std::cout << "there send message" << std::endl;
	sendbyte = send(socket_fd, buf, len, flags );
	if(-1 == sendbyte)
	{
		std::cout << "send code error!!!" << std::endl;
		std::cout << strerror(errno) << std::endl;
		exit(1);
	}
}


/* 为满足需求添加形参，将recv需要的fd传入进去 */
void NSLink::T_Link::LTrecv(int fd, char *buf, int len, int flags )
{
	int recvbyte;
	recvbyte = recv(fd, buf, len, flags );
	if(-1 == recvbyte)
	{
		std::cout << "recv code error!!!" << std::endl;
		std::cout << strerror(errno) << std::endl;
		exit(1);
	}
}


void NSLink::T_Link::LTsetsockopt()
{
	int temp;
	temp = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
	if(-1 == temp)
	{
		std::cout << "setsockopt error!!!" << std::endl;
		exit(1);
	}
}



void NSLink::T_Link::LTSetnonblock(int fd)
{
	if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK) == -1)
		std::cout << "fcntl set fd nonblock error!!!" << std::endl;
}





/* expect free()... */




