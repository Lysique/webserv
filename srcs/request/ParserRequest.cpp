/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/26 09:49:24 by tamighi          ###   ########.fr       */
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
	whole_buff += request;
	has_read = true;
	parse(request);
}

bool	ParserRequest::is_all_received(void)
{
	return (has_read && content_received == m_rm.content_length);
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

	return (std::string(buffer, ret));
}

void	ParserRequest::parse(std::string &buffer)
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
		{
			parseFile();
			break ;
		}
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

void	ParserRequest::parseFile(void)
{
	if (m_rm.post_file.filename == "" || is_all_received() == false)
		return ;

	std::string	file;
	size_t		idx;

	idx = whole_buff.find("\r\n\r\n");
	idx = whole_buff.find("\r\n\r\n", idx + 1);
	file = whole_buff.substr(idx + 4);
	idx = file.find("------" + boundary);
	file = file.substr(0, idx - 2);
	m_rm.post_file.data = file;
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
	struct RequestMembers::post_file	data;
	std::string							word;
	size_t								equal;

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
	m_rm.post_file = data;
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
		ostr << rm.cookies[i] + " ";
	ostr << "\n";
	ostr << "File : " << rm.post_file.filename << "\n";
	ostr << "\n";
	ostr << "Envir :\n";
	for (size_t i = 0; i < rm.small_datas.size(); ++i)
		ostr << rm.small_datas[i] + " ";
	ostr << "\n";
	ostr << "\n";
	return (ostr);;
}
