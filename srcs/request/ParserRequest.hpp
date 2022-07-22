/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:17 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/22 11:20:35 by tamighi          ###   ########.fr       */
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
	//	Context enum for request members
	enum Context
	{
		HEADER,
		BODY,
		BOUNDARY
	};

	RequestMembers(void)
		: port(80), content_length(0), header_length(0), parsed(false), ctx(HEADER)
	{
	}

	//	Request header
	std::string							method;
	std::string							location;
	std::string							protocol;

	std::string							host;
	int									port;

	size_t								content_length;
	std::string							connection;
	std::string							boundary;

	//	Request body
	std::string							content_disposition;
	std::map<std::string, std::string>	postvals;
	std::string							upload;

	//	Additional data
	size_t								header_length;
	bool								parsed;
	Context								ctx;
};

class ParserRequest
{

public:
	//	Public member functions
	ParserRequest(void);

	~ParserRequest(void);

	void	manage_request(int fd);

	const RequestMembers&	getRequest(int fd);

private:
	//	Private member functions
	
	//	Main parsing
	void	parse(std::string buffer);

	//	Header parsing
	void	parseHeader(std::string& line);
	void	parseBody(std::string& line);

	void	parseMethod(std::stringstream& ss, std::string& word);
	void	parseHost(std::stringstream& ss);
	void	parseContentLength(std::stringstream& ss);
	void	parseContentType(std::stringstream& ss);
	void	parseConnection(std::stringstream& ss);

	//	Post method parsing
	void	parsePostvals(std::string& line);

	void	parseEnv(std::string& line);
	void	parseContentDisposition(std::stringstream& ss);


	//	Private members
	std::map<int, RequestMembers>	m_rms;
	RequestMembers					*curr_rm;
};

std::ostream&	operator<<(std::ostream &ostr, RequestMembers& rm);

#endif
