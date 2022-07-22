/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/21 15:56:50 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserRequest.hpp"

ParserRequest::ParserRequest(void)
{
}

ParserRequest::~ParserRequest(void)
{
}

const RequestMembers&	ParserRequest::getRequest(int fd)
{
	return (m_rms[fd]);
}

void	ParserRequest::parse(std::string buffer, int fd)
{
	std::stringstream	ss(buffer);
	std::string			line;

	curr_rm = m_rms[fd];
	curr_fd = fd;

	//	parse here

	while (std::getline(ss, line))
		parseLine(line);

	//	Should be it

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
	//	First line of a request ; we clear the previous one
	m_rms.erase(fd);

	curr_rm.method = word;
	ss >> curr_rm.location;
	ss >> curr_rm.protocol;
}

void	ParserRequest::addHost(std::stringstream& ss)
{
	std::string	word;
	size_t		double_dot;

	ss >> word;
	double_dot = word.find(":");
	curr_rm.host = word.substr(0, double_dot);
	if (double_dot != std::string::npos)
		curr_rm.port = atoi(word.substr(double_dot + 1, word.size()).c_str());
}

void	ParserRequest::addContentLength(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	curr_rm.content_length = atoi(word.c_str());
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
	curr_rm.content_disposition = word;
	while (ss >> word)
	{
		equal = word.find("=");
		key = word.substr(0, equal);
		value = word.substr(equal + 1);
		if (value.back() == ';')
			value.pop_back();
		curr_rm.postvals[key] = value;
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
		curr_rm.postvals[key] = value;
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
