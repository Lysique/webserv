/* ************************************************************************** */ /*                                                                            */ /*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:25:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 09:58:14 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<ServerMembers> &a)
{
	int	fd;

	servers = a;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		fd = create_server(servers[i].port, servers[i].host); 
		NewFds.push_back(fd);
	}
}

Server::~Server(void)
{
	for (size_t i = 0; i < NewFds.size(); i++)
		close(NewFds[i]);
}

int Server::create_server(int iport, std::string host)
{
	struct sockaddr_in	saddr;
	int					fd;

	//	Create socket
	fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (fd < 0)
		throw std::runtime_error("Socket failed.");
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed.");


	//	Allow the port to be reusable when restarting
	int reuse = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
		throw std::runtime_error("Setsockopt failed.");

	saddr.sin_family = AF_INET;        
	saddr.sin_port = htons(iport);
	saddr.sin_addr.s_addr = inet_addr(host.c_str());

	//	Bind socket to address and listen to it
	if (bind(fd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0)
		throw std::runtime_error("Bind failed.");
	if (listen(fd, 4092) < 0)
		throw std::runtime_error("Listen failed.");
	return (fd);
}

bool	Server::is_server_socket(int socket)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (NewFds[i] == socket)
			return (true);
	}
	return (false);
}

void	Server::handle_connection(int socket, ResHandler response)
{
	char	buffer[DATA_BUFFER + 1];
	int		ret;

	//	Read connection from socket
	memset(buffer, 0, DATA_BUFFER);
	ret = read(socket, buffer, DATA_BUFFER);
	if (ret == -1)
		throw std::runtime_error("Read failed.");

	//	Client closed the connection -> no response needed
	if (ret == 0)
	{
		close(socket);
		return ;
	}

	//	Else handle the request.
	
	//	Store request XX
	response.MainServer.bando = buffer;

	//	Get a response
	ParserRequest pr(buffer);
	response.manage_request(pr.getRequest());

	//	TO CHECK
	write(socket, response.TheReposn.c_str(), (response.TheReposn.size() + 1));
	close(socket);
	//	FCNTL flag interdit
	//fcntl(socket, F_SETFD, FD_CLOEXEC);
	response.TheReposn = "";
}

int Server::run()
{
	fd_set		current_sockets, ready_sockets;
	int			client_socket;
	ResHandler	response(servers);

	struct sockaddr_in	client_addr;
	int	addr_size = sizeof(socklen_t);

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
				if (is_server_socket(i))
				{
					client_socket = accept(i, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
					{
						if (client_socket == -1)
							throw std::runtime_error("Accept failed.");
						FD_SET(client_socket, &current_sockets);
					}
				}
				//	Else, we handle the connection and close it.
				else
				{
					handle_connection(i, response);
					FD_CLR(i, &current_sockets);
				}
			}
		}
	}
	return (0);
}
