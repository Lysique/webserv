/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/24 17:46:47 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserRequest.hpp"

ParserRequest::ParserRequest(void)
	: ctx(HEADER), all_received(false), content_received(0)
{
}

ParserRequest::~ParserRequest(void)
{
}

void	ParserRequest::manage_request(int fd)
{
	std::string	request;

	request = read_client(fd);
	parse(request);
}

bool	ParserRequest::is_all_received(void)
{
	return (all_received);
}

void	ParserRequest::clear(void)
{
	RequestMembers	new_members;

	all_received = false;
	content_received = 0;
	boundary = "";
	ctx = HEADER;
	m_rm = new_members;
}

const RequestMembers&	ParserRequest::getRequest(void)
{
	return (m_rm);
}

std::string	ParserRequest::read_client(int fd)
{
	int			ret;
	char		buffer[DATA_BUFFER + 1];

	memset(buffer, 0, DATA_BUFFER);
	ret = read(fd, buffer, DATA_BUFFER);

	if (ret == -1)
		throw std::runtime_error("Read failed.");

	if (ret == 0)
		return "";

	std::string	str_buff(buffer, ret);
	return (str_buff);
}

void	ParserRequest::parse(std::string buffer)
{
	std::string			line;
	std::stringstream	ss(buffer);

	while (std::getline(ss, line))
	{
		if (ctx == HEADER)
			parseHeader(line);
		else if (ctx == BODY)
			parseBody(line);
		else if (ctx == BOUNDARY)
			parseBoundary(line);
		else if (ctx == CONTENT)
			parseContent(line);
	}
	all_received = true;
}

void	ParserRequest::parseHeader(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	ss >> word;
	if (Utils::isValidMethod(word))
		parseMethod(ss, word);
	else if (word == "Host:")
		parseHost(ss);
	else if (word == "Content-Length:")
		parseContentLength(ss);
	else if (word == "Content-Type:")
		parseContentType(ss);
	else if (word == "Connection:")
		parseConnection(ss);
	else if (word == "Cookie:")
		parseCookie(ss);
	else if (word == "")
		ctx = BODY;
}

void	ParserRequest::parseBody(std::string& line)
{
	content_received += line.size();
	if (boundary != "" && line.find(boundary) != std::string::npos)
		ctx = BOUNDARY;
	else
		parsePost(line);
}

void	ParserRequest::parseBoundary(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	content_received += line.size();
	ss >> word;
	if (word == "Content-Disposition:")
		parseContentDisposition(ss);
	else if (word == "")
	{
		content_received += 4;
		ctx = CONTENT;
	}
}

void	ParserRequest::parseContent(std::string& line)
{
	content_received += line.size();
	if (line.find(boundary) != std::string::npos)
		ctx = BOUNDARY;
	else
		m_rm.postdata.back().value += line;
}

void	ParserRequest::parseMethod(std::stringstream& ss, std::string& word)
{
	m_rm.method = word;
	ss >> m_rm.location;
	ss >> m_rm.protocol;
}

void	ParserRequest::parseHost(std::stringstream& ss)
{
	std::string	word;
	size_t		double_dot;

	ss >> word;

	double_dot = word.find(":");
	if (double_dot != std::string::npos)
	{
		m_rm.host = word.substr(0, double_dot);
		m_rm.port = atoi(word.substr(double_dot + 1, word.size()).c_str());
	}
	else
	{
		m_rm.host = word;
		m_rm.port = 80;
	}
}

void	ParserRequest::parseContentLength(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	m_rm.content_length = atoi(word.c_str());
}

void	ParserRequest::parseContentType(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	if (word == "multipart/form-data;")
	{
		ss >> word;
		size_t idx = word.find("WebKitFormBoundary");
		boundary = word.substr(idx);
	}
}

void	ParserRequest::parseConnection(std::stringstream& ss)
{
	ss >> m_rm.connection;
}

void	ParserRequest::parseCookie(std::stringstream& ss)
{
	std::string	word;
	size_t		equal;
	std::string	key;
	std::string	value;

	while (ss >> word)
	{
		if (word.back() == ';')
			word.pop_back();
		equal = word.find("=");
		key = word.substr(0, equal);
		value = word.substr(equal + 1);
		m_rm.cookies[key] = value;
	}
}

void	ParserRequest::parsePost(std::string& line)
{
	size_t		equal;
	size_t		next = 0;

	while (next != std::string::npos)
	{
		RequestMembers::s_postdata	data;

		equal = line.find("=");
		next = line.find("&");

		data.env = true;
		data.file = false;
		data.key = line.substr(0, equal);
		data.value = line.substr(equal + 1, next - (equal + 1));

		m_rm.postdata.push_back(data);
		line = line.substr(next + 1, line.size());
	}
}

void	ParserRequest::parseContentDisposition(std::stringstream& ss)
{
	RequestMembers::s_postdata	data;
	std::string					value;
	std::string					key;
	std::string					word;
	size_t						equal;

	ss >> word;
	data.env = false;
	data.file = false;
	while (ss >> word)
	{
		if (word.back() == ';')
			word.pop_back();

		equal = word.find("=");
		key = word.substr(0, equal);
		value = word.substr(equal + 1);

		if (key == "filename")
			data.file = true;

		data.key = value;
	}
	m_rm.postdata.push_back(data);
}

std::ostream&	operator<<(std::ostream &ostr, RequestMembers& rm)
{
	ostr << "PARSING : \n";
	ostr << "Method : " << rm.method << std::endl;
	ostr << "Location : " << rm.location << std::endl;
	ostr << "Protocol : " << rm.protocol << std::endl;

	ostr << "Host : " << rm.host << ", port : " << rm.port << std::endl;
	ostr << "Content_length : " << rm.content_length << std::endl;
	ostr << "Connection : " << rm.connection << std::endl;
	ostr << "\n";
	return (ostr);;
}
