/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:22:23 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 10:00:40 by tamighi          ###   ########.fr       */
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

#define DATA_BUFFER 800000

class ResHandler;

class Server
{

public:

	//	Public functions
	Server(std::vector<ServerMembers> &a);
	~Server(void);

	int	run();

private:

	//	Private functions
	int		create_server(int iport, std::string host);
	void	handle_connection(int socket, ResHandler response);

	bool	is_server_socket(int socket);

	//	Private members
	std::vector<ServerMembers> servers;
	std::vector<int>			NewFds;
};

#endif
