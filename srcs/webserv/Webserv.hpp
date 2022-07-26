/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:22:23 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/24 15:19:13 by tamighi          ###   ########.fr       */
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
class Webserv
{
	//	Client class
	class	Client {

	public:
		Client(ServerMembers &sm);

		void	get_request(void);

		int				sock;
		ResponseHandler	response;
		ParserRequest	request;
	};

	//	Server class
	class	Server {

	public:
		Server(ServerMembers &sm);

		void	send_response(Client &client);
		void	accept_client(fd_set &current_sockets);

		int					sock;
		std::vector<Client>	clients;
		ServerMembers		members;
	};

public:

			//	Public functions

	Webserv(std::vector<ServerMembers> &sm);
	~Webserv(void);

	void	run();

private:

			//	Private variables

	std::vector<Server>		servers;
	fd_set					current_sockets;
};

#endif
