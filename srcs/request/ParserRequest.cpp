/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/14 13:28:17 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserRequest.hpp"

ParserRequest::ParserRequest(std::string buffer)
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

	while (std::getline(ss, line) && line != "\r")
		parseLine(line);

	if (line == "\r")
	{
		std::getline(ss, line);
		if (line.find("WebKitFormBoundary") != std::string::npos)
		{
			while (std::getline(ss, line))
				parsePostvals(line);
		}
		else
			parseEnv(line);
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
	m_rm.content_length = atoi(word.c_str());
}

void	ParserRequest::parsePostvals(std::string& line)
{
	std::string			word;
	std::stringstream	ss(line);

	ss >> word;
	if (word == "Content-Disposition:")
		addContentDisposition(ss);
}

void	ParserRequest::addContentDisposition(std::stringstream& ss)
{
	std::string	word;
	std::string	key;
	std::string	value;
	size_t		equal;

	ss >> word;
	word.pop_back();
	m_rm.content_disposition = word;
	while (ss >> word)
	{
		equal = word.find("=");
		key = word.substr(0, equal);
		value = word.substr(equal + 1);
		if (value.back() == ';')
			value.pop_back();
		m_rm.postvals[key] = value;
	}
}

void	ParserRequest::parseEnv(std::string& line)
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
		m_rm.env.push_back(key + "=" + value);
		line = line.substr(next + 1, line.size());
	}
}

std::ostream&	operator<<(std::ostream &ostr, ParserRequest& pr)
{
	RequestMembers	rm = pr.getRequest();
	std::string		str = pr.getRequestStr();

	//ostr << "REQUEST : \n" << str << std::endl;
	ostr << "PARSING : \n";
	ostr << "Method : " << rm.method << std::endl;
	ostr << "Location : " << rm.location << std::endl;
	ostr << "Protocol : " << rm.protocol << std::endl;
	ostr << "Host : " << rm.host << std::endl;
	ostr << "Postvals: \n";
	for (std::map<std::string, std::string>::iterator it = rm.postvals.begin(); it != rm.postvals.end(); ++it)
		ostr << "'" << it->first << "' : '" << it->second << "'. ";
	return (ostr);;
}
