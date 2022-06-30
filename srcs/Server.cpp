/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:25:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/06/29 11:38:33 by fejjed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "request/ParserRequest.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "config/ParserConfig.hpp"

#define MAX_CONNECTIONS 65535 

int Server::read_connection(int socket)
{
	char	buffer[DATA_BUFFER + 1];

	memset(buffer, 0, DATA_BUFFER);
	int ret = recv(socket, buffer, DATA_BUFFER, 0);
	if (ret == -1)
		return -1;
	if (ret == 0)
		return 5;
	buffu += std::string(buffer, ret);
	size_t res = buffu.find("\r\n\r\n");
	if (res != std::string::npos)
	{
		if (buffu.find("Content-Length: ") == std::string::npos)
			return (0);
	
		size_t len = std::atoi(buffu.substr(buffu.find("Content-Length: ") + strlen("Content-Length: "), 10).c_str());
		
		if (buffu.size() >= len + res + strlen("\r\n\r\n"))
			return (0);
		else
			return (1);
	}
	return (1);
}


int create_server(int iport, std::string host);

Server::Server(std::vector<ServerMembers> &a)
{

	servers = a;
	setNbPort(servers.size());
	for (unsigned long i = 0; i < servers.size(); ++i)
	{
		fd[i] = create_server(servers[i].port, servers[i].host); 
	}
}

int Server::getNbPort(void)
{
	return	(this->NbPort);
}

void Server::setNbPort(int n)
{
	this->NbPort = n;
}

Server::~Server(void)
{
	for (int i = 0; i < getNbPort(); i++)
	{
		close(fd[i]);
	}
	close(efd);

}


int Server::create_server(int iport, std::string host)
{
	struct sockaddr_in saddr;
	int  ret_val;

	efd = socket(AF_INET, SOCK_STREAM, 0); 
	if (efd == -1)
	{
		std::cout << stderr << "socket failed \n" << strerror(errno) << std::endl;
		return -1;
	}
//

		if (fcntl(efd, F_SETFL, O_NONBLOCK) < 0)
		{
			std::cout << stderr << "fcntl failed \n" << strerror(errno) << std::endl;
			return -1;
		}

		int optval = 1;
		if (setsockopt(efd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		{
			std::cout << stderr << "setsockopt failed \n" << strerror(errno) << std::endl;
			return -1;
		}

//
	saddr.sin_family = AF_INET;        
	saddr.sin_port = htons(iport);

	char* str_host = const_cast<char*>(host.c_str());
	saddr.sin_addr.s_addr = inet_addr(str_host);
	ret_val = bind(efd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	if (ret_val != 0)
	{

		close(efd);
		return -1;
	}
	ret_val = listen(efd, 4092);
	if (ret_val != 0)
	{
		close(efd);
		return -1;
	}
	return efd;
}

