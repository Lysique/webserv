/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:25:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/05 13:37:35 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<ServerMembers> &a)
{
	servers = a;
	NbPort = servers.size();
	for (int i = 0; i < NbPort; ++i)
		fd[i] = create_server(servers[i].port, servers[i].host); 
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


	//	TO CHECK
	int optval = 1;
	if (setsockopt(efd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
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

int Server::run(std::string FileConf)
{
	fd_set read_fd_set;
	fd_set write_fd_set = {0};

	int new_fd, nb_connections, i;
	int check_probl = 0;
	socklen_t addrlen;
	int all_connections[MAX_CONNECTIONS];
	std::string kfe;

	//	Init fds with server fds (cannot use class member // vectors ?)
	for (i = 0; i < MAX_CONNECTIONS; i++)
		all_connections[i] = -1;
	for (int i = 0; i < NbPort; i++)
		all_connections[i] = fd[i];

	while (1)
	{
		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);

		//	Put the fd in the set of select
		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (all_connections[i] != -1)
				FD_SET(all_connections[i], &read_fd_set);
		}

		//	Select
		nb_connections = select(FD_SETSIZE, &read_fd_set, &write_fd_set, NULL, NULL);
		if (nb_connections == -1)
			throw std::runtime_error("Select failed.");

		//	Loop to accept connections
		for (int i = 0; i < NbPort; i++)
		{
			if (FD_ISSET(fd[i], &read_fd_set))
			{
				new_fd = accept(fd[i], (struct sockaddr *)&new_addr, &addrlen);
				if (new_fd == -1)
					throw std::runtime_error("Accept failed.");
				fcntl(fd[i], F_SETFD, FD_CLOEXEC);

				//	Put the new connection if there is one in all connections
				if (new_fd >= 0)
				{
					for (i = 0; i < MAX_CONNECTIONS; i++)
					{
						if (all_connections[i] < 0)
						{
							all_connections[i] = new_fd;
							break;
						}
					}
				}
				//	Continue till no more connections
				nb_connections--;
				if (!nb_connections)
					continue;
			}
		}

		//	Loop again to get request and respond
		for (i = 1; i < MAX_CONNECTIONS; i++)
		{

			int err = -1;
			if ((all_connections[i] > 0) && (FD_ISSET(all_connections[i], &read_fd_set)))
			{
				err = read_connection(all_connections[i]);
				if (err == 0)
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
					int checkWrit = write(all_connections[i], test.TheReposn.c_str(), (test.TheReposn.size() + 1));
					if (checkWrit == 0)
						check_probl = 0;
					else if (checkWrit == -1)
						check_probl = -1;
					close(all_connections[i]);
					fcntl(all_connections[i], F_SETFD, FD_CLOEXEC);
					FD_CLR(all_connections[i], &read_fd_set);
					buffu = "";
					test.TheReposn = "";
					all_connections[i] = -1;
					break;
				}
				if (err == 5)
				{
					close(all_connections[i]);
					all_connections[i] = -1;
				}
				if (err == -1)
				{
					break;
				}
			}
			err--;
		}
	}
	close(new_fd);
	return 0;
}

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
