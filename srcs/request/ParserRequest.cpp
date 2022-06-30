/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/06/10 15:33:38 by fejjed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "ParserRequest.hpp"
#include <sys/socket.h>

ParserRequest::ParserRequest(const char *buffer)
{
	// char	buffer[1024];

	// if (read(fdConnection, buffer, 1024) == -1)
	// 	throw std::runtime_error("Unable to read connection :" + std::to_string(fdConnection));
	m_request = buffer;
	parse();
}

ParserRequest::~ParserRequest(void)
{
}

const RequestMembers&	ParserRequest::getRequest(void) const
{
	return (m_rm);
}

const std::string&	ParserRequest::getRequestStr(void) const
{
	return (m_request);
}

void	ParserRequest::parse(void)
{
	std::stringstream	ss(m_request);
	std::string			line;

	while (std::getline(ss, line))
		parseLine(line);
}

void	ParserRequest::parseLine(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	ss >> word;
	if (Utils::isValidMethod(word))
		addMethod(ss, word);
	else if (word == "Host:")
		addHost(ss);
	else if (word == "Content-Length:")
		addContentLength(ss);
}

void	ParserRequest::addMethod(std::stringstream& ss, std::string& word)
{
	m_rm.method = word;
	ss >> m_rm.location;
	ss >> m_rm.protocol;
}

void	ParserRequest::addHost(std::stringstream& ss)
{
	ss >> m_rm.host;
}

void	ParserRequest::addContentLength(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	m_rm.content_length = std::stoi(word);
}

std::ostream&	operator<<(std::ostream &ostr, ParserRequest& pr)
{
	RequestMembers	rm = pr.getRequest();
	std::string		str = pr.getRequestStr();

	std::cout << "REQUEST : \n" << str << std::endl;
	// std::cout << "PARSING : \n";
	// ostr << "Method : " << rm.method << std::endl;
	// ostr << "Location : " << rm.location << std::endl;
	// ostr << "Protocol : " << rm.protocol << std::endl;
	// ostr << "Host : " << rm.host << std::endl;
	// ostr << "Content length : " << rm.content_length << std::endl;
	return (ostr);
}
