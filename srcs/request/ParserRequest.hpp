/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:17 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/24 17:10:43 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSERREQUEST_HPP
# define PARSERREQUEST_HPP

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include "../utils/Utils.hpp"

#define DATA_BUFFER 800000

struct RequestMembers
{

	//	Struct for postdata
	struct s_postdata {
		bool		file;
		bool		env;
		std::string	key;
		std::string	value;
	};

	RequestMembers(void)
		: port(0), content_length(0), header_length(0)
	{
	}

	//	Request header
	std::string							method;
	std::string							location;
	std::string							protocol;

	std::string							connection;
	std::string							host;
	int									port;

	size_t								content_length;
	size_t								header_length;
	std::map<std::string, std::string>	cookies;

	//	Request body
	std::vector<s_postdata>				postdata;
};

class ParserRequest
{

public:
	//	Public member functions

	ParserRequest(void);

	~ParserRequest(void);

	void					manage_request(int fd);
	bool					is_all_received(void);
	void					clear(void);
	const RequestMembers&	getRequest(void);

private:
			//	Private member functions
	
	std::string	read_client(int fd);
	void		parse(std::string buffer);

	//	Main parsing
	void		parseHeader(std::string& line);
	void		parseBody(std::string& line);
	void		parseBoundary(std::string& line);
	void		parseContent(std::string& line);

	//	Header parsing
	void		parseMethod(std::stringstream& ss, std::string& word);
	void		parseHost(std::stringstream& ss);
	void		parseContentLength(std::stringstream& ss);
	void		parseContentType(std::stringstream& ss);
	void		parseConnection(std::stringstream& ss);
	void		parseCookie(std::stringstream& ss);

	//	Body parsing
	void		parsePost(std::string& line);

	//	Boundary parsing
	void		parseContentDisposition(std::stringstream& ss);

	//	Context for parsing
	enum Context
	{
		HEADER,
		BODY,
		BOUNDARY,
		CONTENT
	};

			//	Private variables
	RequestMembers					m_rm;

	Context							ctx;
	std::string						boundary;
	bool							all_received;
	size_t							content_received;
};

std::ostream&	operator<<(std::ostream &ostr, RequestMembers& rm);

#endif
