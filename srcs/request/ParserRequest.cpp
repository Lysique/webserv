/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 12:52:11 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/22 17:23:48 by tamighi          ###   ########.fr       */
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

void	ParserRequest::manage_request(int fd)
{
	//	Read data from fd
	int			ret;
	char		buffer[DATA_BUFFER + 1];

	memset(buffer, 0, DATA_BUFFER);
	ret = read(fd, buffer, DATA_BUFFER);

	if (ret == -1)
		throw std::runtime_error("Read failed.");

	if (ret == 0)
	{
		std::cout << "ERASE\n\n" << std::endl;
		m_rms.erase(fd);
		return ;
	}

	//	Retrieve data structure from current fd
	curr_rm = &m_rms[fd];

	//	Put the char to a cpp string
	std::string	str_buff(buffer, ret);
	parse(str_buff);
}

void	ParserRequest::clear(int fd)
{
	m_rms.erase(fd);
}

void	ParserRequest::parse(std::string buffer)
{
	std::string			line;
	std::stringstream	ss(buffer);

	while (std::getline(ss, line))
	{
		if (curr_rm->ctx == RequestMembers::HEADER)
			parseHeader(line);
		else if (curr_rm->ctx == RequestMembers::BODY)
			parseBody(line);
		else if (curr_rm->ctx == RequestMembers::BOUNDARY)
			parseBoundary(line);
		else if (curr_rm->ctx == RequestMembers::CONTENT)
			parseContent(line);
	}
	curr_rm->parsed = true;
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
	else if (word == "")
		curr_rm->ctx = RequestMembers::BODY;
}

void	ParserRequest::parseBody(std::string& line)
{
	if (line.find(curr_rm->boundary) != std::string::npos)
		curr_rm->ctx = RequestMembers::BOUNDARY;
	else
		parseEnv(line);
}

void	ParserRequest::parseBoundary(std::string& line)
{
	std::stringstream	ss(line);
	std::string			word;

	ss >> word;
	if (word == "")
		curr_rm->ctx = RequestMembers::CONTENT;
	else if (word == "Content-Disposition:")
		parseContentDisposition(ss);
}

void	ParserRequest::parseContent(std::string& line)
{
	//	Means end of upload file, there can be multiple ?
	if (line.find(curr_rm->boundary) != std::string::npos)
		curr_rm->ctx = RequestMembers::END;
	else
		curr_rm->upload += line;
}

void	ParserRequest::parseMethod(std::stringstream& ss, std::string& word)
{
	curr_rm->method = word;
	ss >> curr_rm->location;
	ss >> curr_rm->protocol;
}

void	ParserRequest::parseHost(std::stringstream& ss)
{
	std::string	word;
	size_t		double_dot;

	ss >> word;

	double_dot = word.find(":");
	if (double_dot != std::string::npos)
	{
		curr_rm->host = word.substr(0, double_dot);
		curr_rm->port = atoi(word.substr(double_dot + 1, word.size()).c_str());
	}
	else
	{
		curr_rm->host = word;
		curr_rm->port = 80;
	}
}

void	ParserRequest::parseContentLength(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	curr_rm->content_length = atoi(word.c_str());
}

void	ParserRequest::parseContentType(std::stringstream& ss)
{
	std::string	word;

	ss >> word;
	if (word == "multipart/form-data;")
	{
		ss >> word;
		size_t idx = word.find("WebKitFormBoundary");
		curr_rm->boundary = word.substr(idx);
	}
}

void	ParserRequest::parseConnection(std::stringstream& ss)
{
	ss >> curr_rm->connection;
}

void	ParserRequest::parseContentDisposition(std::stringstream& ss)
{
	std::string	word;
	std::string	key;
	std::string	value;
	size_t		equal;

	ss >> word;
	word.pop_back();
	curr_rm->content_disposition = word;
	while (ss >> word)
	{
		equal = word.find("=");
		key = word.substr(0, equal);
		value = word.substr(equal + 1);
		if (value.back() == ';')
			value.pop_back();
		curr_rm->postvals[key] = value;
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
		curr_rm->postvals[key] = value;
		line = line.substr(next + 1, line.size());
	}
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
	ostr << "Boundary : " << rm.boundary << std::endl;

	ostr << "parsed : " << rm.parsed << std::endl;

	ostr << "Postvals: \n";
	for (std::map<std::string, std::string>::iterator it = rm.postvals.begin(); it != rm.postvals.end(); ++it)
		ostr << "'" << it->first << "' : '" << it->second << "'. ";
	ostr << "\n";
	ostr << "Upload :\n" << rm.upload << std::endl;
	return (ostr);;
}
