/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:17 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/12 09:17:59 by tamighi          ###   ########.fr       */
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
	std::vector<std::string>			env;
	std::map<std::string, std::string>	postvals;

	std::string							method;
	std::string							location;
	std::string							protocol;
	std::string							host;
	int									port;
};

class ParserRequest
{

public:
	//	Public member functions
	ParserRequest(const char *buffer);

	~ParserRequest(void);

	const RequestMembers&	getRequest(void) const;
	const std::string&		getRequestStr(void) const;
private:
	//	Private member functions

	//	Main parsing functions
	void	parse(void);
	void	parseLine(std::string& line);
	void	parsePostvals(std::string& line);

	//	Secondary parsing functions
	void	addMethod(std::stringstream& ss, std::string& word);
	void	addHost(std::stringstream& ss);

	//	Private members
	RequestMembers	m_rm;
	std::string		m_request;
};

std::ostream&	operator<<(std::ostream &ostr, ParserRequest& pr);

#endif
