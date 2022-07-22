/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:22:23 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/22 10:07:45 by tamighi          ###   ########.fr       */
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
#include "../config/ParserConfig.hpp"
#include "../response/ResponseHandler.hpp"
#include "../request/ParserRequest.hpp"

class ResponseHandler; 
class Server
{

public:

	//	Public functions
	Server(std::vector<ServerMembers> &a);
	~Server(void);

	void	run();

private:

	//	Private functions
	void	create_server(int iport, std::string host);

	//	Connection management
	void	accept_connection(int socket);
	void	close_connection(int socket);

	void	write_connection(int socket);

	//	Utils
	bool	is_server_socket(int socket);

	//	Private members
	std::vector<int>				server_sockets;

	ResponseHandler					res_handler;
	ParserRequest					req_handler;

	fd_set							current_sockets;
};

#endif
