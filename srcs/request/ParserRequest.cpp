/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/25 12:14:49 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserRequest.hpp"

ParserRequest::ParserRequest(void)
	: ctx(HEADER), has_read(false), content_received(0)
{
}

ParserRequest::~ParserRequest(void)
{
}

void	ParserRequest::manage_request(int fd)
{
	std::string	request;

	request = read_client(fd);
	has_read = true;
	//std::cout << request << std::endl;
	parse(request);
	std::cout << m_rm;
}

bool	ParserRequest::is_all_received(void)
{
	return (has_read && content_received == m_rm.content_length);
}

void	ParserRequest::clear(void)
{
	RequestMembers	new_members;

	has_read = false;
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

	content_received += ret;
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
}

void	ParserRequest::parseHeader(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	content_received -= line.size() + 1;
	ss >> word;
	if (Utils::isValidMethod(word))
		parseMethod(ss, word);
	else if (word == "Host:")
		parseHost(ss);
	else if (word == "Content-Length:")
		parseContentLength(ss);
	else if (word == "Content-Type:")
		parseContentType(ss);
	else if (word == "Cookie:")
		parseCookie(ss);
	else if (word == "")
		ctx = BODY;
}

void	ParserRequest::parseBody(std::string& line)
{
	if (boundary != "" && line.find(boundary) != std::string::npos)
		ctx = BOUNDARY;
	else
		parsePost(line);
}

void	ParserRequest::parseBoundary(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	ss >> word;
	if (word == "Content-Disposition:")
		parseContentDisposition(ss);
	else if (word == "")
		ctx = CONTENT;
}

void	ParserRequest::parseContent(std::string& line)
{
	if (line.find(boundary) != std::string::npos)
		ctx = BOUNDARY;
	else
		m_rm.big_datas.back().data += line;
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

void	ParserRequest::parseCookie(std::stringstream& ss)
{
	std::string	word;

	while (ss >> word)
	{
		if (word.back() == ';')
			word.pop_back();
		m_rm.cookies.push_back(word);
	}
}

void	ParserRequest::parsePost(std::string& line)
{
	size_t		next = 0;

	while (next != std::string::npos)
	{
		next = line.find("&");
		m_rm.small_datas.push_back(line.substr(0, next));
		line = line.substr(next + 1, line.size());
	}
}

void	ParserRequest::parseContentDisposition(std::stringstream& ss)
{
	RequestMembers::big_data	data;
	std::string					word;
	size_t						equal;

	ss >> word;
	ss >> word;
	if (word.back() == ';')
		word.pop_back();
	equal = word.find("=");
	data.envname = word.substr(equal + 1);
	if (ss >> word)
	{
		equal = word.find("=");
		data.filename = word.substr(equal + 2);
		data.filename.pop_back();
	}
	m_rm.big_datas.push_back(data);
}

std::ostream&	operator<<(std::ostream &ostr, RequestMembers& rm)
{
	ostr << "PARSING : \n";
	ostr << "Method : " << rm.method << std::endl;
	ostr << "Location : " << rm.location << std::endl;
	ostr << "Protocol : " << rm.protocol << std::endl;

	ostr << "Host : " << rm.host << ", port : " << rm.port << std::endl;

	ostr << "Content_length : " << rm.content_length << std::endl;
	ostr << "Cookies :\n";
	for (size_t i = 0; i < rm.cookies.size(); ++i)
		std::cout << rm.cookies[i] + " ";
	ostr << "\n";
	ostr << "Files :\n";
	for (size_t i = 0; i < rm.big_datas.size(); ++i)
		std::cout << rm.big_datas[i].filename + " ";
	ostr << "\n";
	ostr << "Envir :\n";
	for (size_t i = 0; i < rm.small_datas.size(); ++i)
		ostr << rm.small_datas[i] + " ";
	ostr << "\n";
	ostr << "\n";
	return (ostr);;
}
