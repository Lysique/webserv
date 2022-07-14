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
		server_sockets.push_back(fd);
	}
}

Server::~Server(void)
{
	for (size_t i = 0; i < server_sockets.size(); i++)
		close(server_sockets[i]);
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
		if (server_sockets[i] == socket)
			return (true);
	}
	return (false);
}

bool	Server::handle_connection(int socket, std::string& buffer, int& ret, ResponseHandler& response)
{
	std::string	http_response;
	int			curr_ret;
	char		curr_buffer[DATA_BUFFER + 1];

	//	Read connection from socket
	memset(curr_buffer, 0, DATA_BUFFER);
	curr_ret = read(socket, curr_buffer, DATA_BUFFER);
	if (curr_ret == -1)
		throw std::runtime_error("Read failed.");

	//	Client closed the connection -> no response needed
	if (curr_ret == 0)
		return true;

	//	Add the ret and buffer to previous
	ret += curr_ret;
	buffer += curr_buffer;
	//	Parse request and manage response
	// std::cout << buffer << std::endl;

	ParserRequest pr(buffer);
	RequestMembers rm = pr.getRequest();

	//	Not everything has been readed
	if (ret < static_cast<int>(rm.content_length))
		return false;

	//	Else manage request and send response
	http_response = response.manage_request(rm);
	//std::cout << http_response << std::endl;

	curr_ret = write(socket, http_response.c_str(), http_response.size());
	if (ret == -1)
		throw std::runtime_error("Write failed.");

	//	If ret is 0, it means that the client disconnected.  
	//	We close the connection in any case so nothing to do here. 
	return true;
}

void Server::run()
{
	fd_set						current_sockets, ready_sockets;
	int							client_socket;
	ResponseHandler				response(servers);
	std::map<int, std::string>	buffers;
	std::map<int, int>			ret;

	//	Empty variables needed for accept
	struct sockaddr_in	client_addr;
	int	addr_size = sizeof(socklen_t);

	//	Init current_sockets with server sockets
	FD_ZERO(&current_sockets);
	for (size_t i = 0; i < server_sockets.size(); ++i)
		FD_SET(server_sockets[i], &current_sockets);

	while (1)
	{
		//	Copy because select is destructive
		ready_sockets = current_sockets;

		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
			throw std::runtime_error("Select failed.");

		//	Select returned ; a socket is ready to be read
		for (int fd = 0; fd < FD_SETSIZE; ++fd)
		{
			//	Check if is in the list of fd returned by select
			if (FD_ISSET(fd, &ready_sockets))
			{
				//	If it is a server, we accept the connection.
				if (is_server_socket(fd))
				{
					client_socket = accept(fd, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
					std::cout << "Client accepted : " << client_socket << "\n";
					if (client_socket == -1)
						throw std::runtime_error("Accept failed.");

					buffers[client_socket].clear();
					ret[client_socket] = 0;
					FD_SET(client_socket, &current_sockets);
				}
				//	Else, we handle the connection and close it.
				else
				{
					std::cout << "Handle Request : " << fd << std::endl;
				
					//	All the request has been read if true
					if (handle_connection(fd, buffers[fd], ret[fd], response) == true)
					{
						FD_CLR(fd, &current_sockets);
						close(fd);
					}
				}
			}
		}
	}
}
