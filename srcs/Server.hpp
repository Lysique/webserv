/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:22:23 by tamighi           #+#    #+#             */
/*   Updated: 2022/06/27 14:49:59 by fejjed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include "config/ParserConfig.hpp"

#define MAX_CONNECTIONS 65535 
#define DATA_BUFFER 800000
class ConnectionErr : public std::exception
{
	const char * what () const throw () { return ("Read error occurred while receiving on the socket, closing connection"); }
};
class Server
{
	public:
		Server(std::vector<ServerMembers> &a);
		// Server(int eport);
		int autoindx[65535];		
		~Server(void);
		// void	setPort(int p);
		// void	creat(void);
		int	run(std::string FileConf);
		int getNbPort(void);
		void setNbPort(int n);
		int create_server(int iport, std::string host);
		int read_connection(int socket);
	private:
		std::vector<ServerMembers> servers;
		
		int					fd[MAX_CONNECTIONS];
		int					efd;
		// int					sockfd;
		int					NbPort;
		struct sockaddr_in	new_addr;

		std::string buffu;
};
