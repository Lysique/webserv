/* ************************************************************************** */ 
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:25:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 09:58:14 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<ServerMembers> &servers)
	: res_handler(servers), req_handler()
{
	//	Create servers
	for (size_t i = 0; i < servers.size(); ++i)
		create_server(servers[i].port, servers[i].host); 

	//	Init current_sockets with server sockets.
	FD_ZERO(&current_sockets);
	for (size_t i = 0; i < server_sockets.size(); ++i)
		FD_SET(server_sockets[i], &current_sockets);

}

Server::~Server(void)
{
	for (size_t i = 0; i < server_sockets.size(); i++)
		close(server_sockets[i]);
}

void	Server::create_server(int iport, std::string host)
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
	
	server_sockets.push_back(fd);
}

void Server::run()
{
	fd_set						ready_r_sock;
	fd_set						ready_w_sock;

	while (1)
	{
		//	Copy because select is destructive.
		ready_r_sock = current_sockets;
		ready_w_sock = current_sockets;

		if (select(FD_SETSIZE, &ready_r_sock, &ready_w_sock, NULL, NULL) < 0)
			throw std::runtime_error("Select failed.");

		//	Select returned ; a socket is ready for reading or writing.
		for (int fd = 0; fd <= FD_SETSIZE; ++fd)
		{
			//	Check if is in the list of fd returned by select
			if (FD_ISSET(fd, &ready_r_sock))
			{
				//	It is a server, a client wants to connect and send data
				if (is_server_socket(fd))
					accept_connection(fd);

				//	It is a client sending data
				else
				{
					req_handler.manage_request(fd);
					//if (req_handler.is_disconnected(fd));
						// close connection
				}
			}

			//	Client is ready for receiving data
			if (FD_ISSET(fd, &ready_w_sock))
				//close_connection(fd);
				write_connection(fd);
		}
	}
}

void	Server::write_connection(int socket)
{
	int				ret;
	std::string		buffer;

	//	Get the parsed request
	RequestMembers	rm = req_handler.getRequest(socket);
	if (rm.parsed == false)
		return ;


	std::cout << rm << std::endl;
	//	manage request, write data, and close or not the connection
	buffer = res_handler.manage_request(rm);
	//res_handler.write_connection();

	//	Write the response
	ret = write(socket, buffer.c_str(), buffer.size());
	if (ret == -1)
		throw std::runtime_error("Write failed.");

	// Client disconnected
	if (ret == 0)
	{
		//close_connection(socket);
		//return ;
	}

	std::cout << buffer << " " << ret << std::endl;
	//	Buffer will be cleared when everything is written
	close_connection(socket);
}

void	Server::accept_connection(int socket)
{
	int					client_socket;

	//	Empty variables needed for accept.
	struct sockaddr_in	client_addr;
	int					addr_size = sizeof(socklen_t);

	client_socket = accept(socket, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
	if (client_socket == -1)
		throw std::runtime_error("Accept failed.");

	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed.");

	FD_SET(client_socket, &current_sockets);
	FD_SET(client_socket, &current_sockets);
}

void	Server::close_connection(int socket)
{
	FD_CLR(socket, &current_sockets);
	close(socket);
}

bool	Server::is_server_socket(int socket)
{
	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		if (server_sockets[i] == socket)
			return (true);
	}
	return (false);
}

