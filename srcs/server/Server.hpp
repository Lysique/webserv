/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:22:23 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/05 14:23:55 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include "../config/ParserConfig.hpp"
#include "../Respon/ResHandler.hpp"
#include "../request/ParserRequest.hpp"

#define MAX_CONNECTIONS 65535 
#define DATA_BUFFER 800000

class Server
{

public:
	Server(std::vector<ServerMembers> &a);
	~Server(void);

	int	run(std::string FileConf);

private:
	int create_server(int iport, std::string host);
	int read_connection(int socket);


	std::vector<ServerMembers> servers;
	std::vector<int>			NewFds;
	int					fd[MAX_CONNECTIONS];
	int					efd;
	int					NbPort;
	struct sockaddr_in	new_addr;

	std::string buffu;

	class ServerCreateError : public std::exception
	{
		const char * what () const throw ()
		{ 
			return ("An error occured while creating the server.");
		}
	};
};

#endif
