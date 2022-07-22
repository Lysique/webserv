/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:17 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/21 15:54:52 by tamighi          ###   ########.fr       */
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

struct RequestMembers
{
	RequestMembers(void)
	{
	}

	std::string							method;
	std::string							location;
	std::string							protocol;

	std::string							host;
	int									port;

	std::string							content_disposition;
	size_t								content_length;
	size_t								header_length;

	std::map<std::string, std::string>	postvals;
};

class ParserRequest
{

public:
	//	Public member functions
	ParserRequest(void);

	~ParserRequest(void);

	void	parse(std::string buffer, int fd);

	const RequestMembers&	getRequest(int fd);

private:
	//	Private member functions
	
	//	Main parsing
	

	//	Main parsing functions
	void	parseLine(std::string& line);

	//	Secondary parsing functions
	void	addMethod(std::stringstream& ss, std::string& word);
	void	addHost(std::stringstream& ss);
	void	addContentLength(std::stringstream& ss);

	//	Post method parsing
	void	parsePostvals(std::string& line);

	void	parseEnv(std::string& line);
	void	addContentDisposition(std::stringstream& ss);


	//	Private members
	std::map<int, RequestMembers>	m_rms;

	RequestMembers					curr_rm;
	int								curr_fd;
};

std::ostream&	operator<<(std::ostream &ostr, ParserRequest& pr);

#endif
