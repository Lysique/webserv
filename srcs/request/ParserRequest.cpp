/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/11 16:45:35 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserRequest.hpp"

ParserRequest::ParserRequest(const char *buffer)
{
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
	{
		parseLine(line);
		if (line == "\r")
		{
			std::getline(ss, line);
			parsePostvals(line);
		}
	}
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

void	ParserRequest::parsePostvals(std::string& line)
{
	std::string	key;
	std::string	value;
	size_t		equal;
	size_t		next = 0;

	while (next != std::string::npos)
	{
		equal = line.find("=");
		next = line.find("&");

		key = line.substr(0, equal);
		value = line.substr(equal + 1, next - (equal + 1));
		m_rm.postvals[key] = value;
		line = line.substr(next + 1, line.size());
	}
}

void	ParserRequest::addMethod(std::stringstream& ss, std::string& word)
{
	m_rm.method = word;
	ss >> m_rm.location;
	ss >> m_rm.protocol;
}

void	ParserRequest::addHost(std::stringstream& ss)
{
	std::string	word;
	size_t		double_dot;

	ss >> word;
	double_dot = word.find(":");
	m_rm.host = word.substr(0, double_dot);
	m_rm.port = atoi(word.substr(double_dot + 1, word.size()).c_str());
	m_rm.host = word;
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

	ostr << "REQUEST : \n" << str << std::endl;
	ostr << "PARSING : \n";
	ostr << "Method : " << rm.method << std::endl;
	ostr << "Location : " << rm.location << std::endl;
	ostr << "Protocol : " << rm.protocol << std::endl;
	ostr << "Host : " << rm.host << std::endl;
	ostr << "Content length : " << rm.content_length << std::endl;
	return (ostr);
}
