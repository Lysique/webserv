/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:25:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/05 16:09:43 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<ServerMembers> &a)
{
	int	fdd;

	servers = a;
	NbPort = servers.size();
	for (int i = 0; i < NbPort; ++i)
	{
		fdd = create_server(servers[i].port, servers[i].host); 
		fd[i] = fdd;
		NewFds.push_back(fdd);
	}
}

Server::~Server(void)
{
	for (int i = 0; i < NbPort; i++)
		close(fd[i]);
	close(efd);
}

int Server::create_server(int iport, std::string host)
{
	struct sockaddr_in saddr;

	//	Create socket
	efd = socket(AF_INET, SOCK_STREAM, 0); 
	if (efd < 0)
		throw std::runtime_error("Socket failed.");
	if (fcntl(efd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed.");


	//	Allow the port to be reusable when restarting
	int reuse = 1;
	if (setsockopt(efd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
		throw std::runtime_error("Setsockopt failed.");

	saddr.sin_family = AF_INET;        
	saddr.sin_port = htons(iport);
	saddr.sin_addr.s_addr = inet_addr(host.c_str());

	//	Bind socket to address and listen to it
	if (bind(efd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0)
		throw std::runtime_error("Bind failed.");
	if (listen(efd, 4092) < 0)
		throw std::runtime_error("Listen failed.");
	return (efd);
}


int Server::read_connection(int socket)
{
	char	buffer[DATA_BUFFER + 1];
	int		ret;

	memset(buffer, 0, DATA_BUFFER);
	ret = read(socket, buffer, DATA_BUFFER);
	if (ret == -1)
		throw std::runtime_error("Read failed.");

	//	Client closed the connection
	if (ret == 0)
	{
		close(socket);
		return (-1);
	}
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
			return (-1);
	}
	return (-1);
}

bool	Server::isServerSocket(int socket)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (NewFds[i] == socket)
			return (true);
	}
	return (false);
}

void	Server::handle_connection(int socket, std::string FileConf)
{
	ResHandler test;
	char buf[DATA_BUFFER + 1];
	memset(buf, 0, DATA_BUFFER);
	strcpy(buf, buffu.c_str());
	test.MainServer.bando = buffu;
	std::string conttype;
	test.parse_buf(buf, test.MainServer.filename, conttype);
	ParserRequest pr(buf);
	test.MainServer.host = pr.getRequest().host;
	test.MainServer.protocol = pr.getRequest().protocol;
	test.MainServer.type = pr.getRequest().method;
	test.MainServer.path = pr.getRequest().location;
	test.MainServer.content_len = pr.getRequest().content_length;
	test.MainServer.buffit = std::string(buf);
	test.CheckModiDate();
	test.setDate();
	test.Erostatus();
	test.Methodes(FileConf);
	//	TO CHECK
	write(socket, test.TheReposn.c_str(), (test.TheReposn.size() + 1));
	close(socket);
	fcntl(socket, F_SETFD, FD_CLOEXEC);
	buffu = "";
	test.TheReposn = "";
}

int Server::run(std::string FileConf)
{
	fd_set	current_sockets, ready_sockets;
	int		client_socket, read;

	socklen_t addrlen;

	//	Init current_sockets
	FD_ZERO(&current_sockets);
	for (size_t i = 0; i < NewFds.size(); ++i)
		FD_SET(NewFds[i], &current_sockets);

	while (1)
	{
		//	Copy because select is destructive
		ready_sockets = current_sockets;

		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
			throw std::runtime_error("Select failed.");

		//	Accept connection
		for (int i = 0; i < FD_SETSIZE; ++i)
		{
			if (FD_ISSET(i, &ready_sockets))
			{
				//	If it is a server, we try to accept the connection.
				if (isServerSocket(i))
				{
					client_socket = accept(i, (struct sockaddr *)&new_addr, &addrlen);
					{
						if (client_socket == -1)
							throw std::runtime_error("Accept failed.");
						FD_SET(client_socket, &current_sockets);
					}
				}
				//	Else, we handle the connection.
				else
				{
					read = read_connection(i);
					if (read == 0)
					{
						handle_connection(i, FileConf);
						FD_CLR(i, &current_sockets);
					}
				}
			}
		}
	}
	return (0);
}
